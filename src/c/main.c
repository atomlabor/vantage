#include <pebble.h>

// --------------------------------------------------------------------------
// VANTAGE v1.1.0 (Emery 200x228)
// Features: 
// Atomlabor.de Watchface
// --------------------------------------------------------------------------

static Window *s_window;
static Layer *s_bg_layer, *s_hands_layer;
static GBitmap *s_bmp_moon = NULL;
static GPath *s_path_hour = NULL;
static GPath *s_path_minute = NULL;
static GPath *s_path_sub = NULL;

static bool s_show_battery = false;
static int s_battery_level = 0;
static AppTimer *s_battery_timer = NULL;

static GPoint c_main = {100, 114};
static GPoint c_date = {50, 114}; 
static GPoint c_day  = {150, 114}; 
static GPoint c_moon = {100, 182}; 

#define COL_BG          GColorMidnightGreen
#define COL_NIGHT       GColorBlack
#define COL_BORDER      GColorCadetBlue
#define COL_HOUR_HAND   GColorRed
#define COL_TEXT        GColorWhite
#define COL_METAL       GColorWhite

static void update_layout(GRect bounds) {
    if (bounds.size.w == 0 || bounds.size.h == 0) return;
    
    c_main = GPoint(bounds.origin.x + (bounds.size.w / 2), bounds.origin.y + (bounds.size.h / 2));
    
    c_date = GPoint(c_main.x - 50, c_main.y);
    c_day  = GPoint(c_main.x + 50, c_main.y);
    c_moon = GPoint(c_main.x, c_main.y + 68); 
}

static void unobstructed_area_change_proc(AnimationProgress progress, void *context) {
    if (!s_window) return;
    Layer *root = window_get_root_layer(s_window);
    if (!root) return;

    update_layout(layer_get_unobstructed_bounds(root));
    
    if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    if (s_hands_layer) layer_mark_dirty(s_hands_layer);
}


static void battery_timer_callback(void *data) {
    s_show_battery = false;
    if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    s_battery_timer = NULL;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
    s_show_battery = true;
    s_battery_level = battery_state_service_peek().charge_percent;
    if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    if (s_battery_timer) app_timer_cancel(s_battery_timer);
    s_battery_timer = app_timer_register(5000, battery_timer_callback, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    if (s_hands_layer) layer_mark_dirty(s_hands_layer);

    if (units_changed & HOUR_UNIT) {
        if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    }
}

static void draw_hand(GContext *ctx, GPath *path, GPoint center, int32_t angle, GColor fill) {
    if (!path) return;
    gpath_move_to(path, center);
    gpath_rotate_to(path, angle);
    graphics_context_set_fill_color(ctx, fill);
    gpath_draw_filled(ctx, path);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 1);
    gpath_draw_outline(ctx, path);
}

static void draw_arc_label(GContext *ctx, GPoint center, int radius, int angle_deg, char* text) {
    GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    int32_t angle = DEG_TO_TRIGANGLE(angle_deg);
    GPoint pos = {
        .x = center.x + (sin_lookup(angle) * radius / TRIG_MAX_RATIO),
        .y = center.y - (cos_lookup(angle) * radius / TRIG_MAX_RATIO)
    };
    graphics_draw_text(ctx, text, font, GRect(pos.x - 12, pos.y - 10, 24, 20), 0, GTextAlignmentCenter, NULL);
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
    time_t now = time(NULL); struct tm *t = localtime(&now);
    
    GRect full = layer_get_bounds(layer);
    GRect safe = layer_get_unobstructed_bounds(layer); 

    bool is_night = (t->tm_hour >= 22 || t->tm_hour < 6);
    GColor c_bg = is_night ? COL_NIGHT : COL_BG;
    GColor c_bord = is_night ? COL_NIGHT : COL_BORDER;
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, full, 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, c_bg);
    graphics_fill_rect(ctx, GRect(2, safe.origin.y + 2, safe.size.w - 4, safe.size.h - 4), 18, GCornersAll);
    
    if (s_bmp_moon && safe.size.h > 160) {

        const uint32_t lunation_seconds = 2551443; 

        const time_t new_moon_anchor = 1704974220; 

        time_t current_time = now;
        if (current_time < new_moon_anchor) current_time = new_moon_anchor; 
        time_t delta = current_time - new_moon_anchor;

        int32_t angle = ((delta % lunation_seconds) * (TRIG_MAX_ANGLE / 2)) / lunation_seconds;
        
        graphics_context_set_compositing_mode(ctx, GCompOpSet);
        graphics_draw_rotated_bitmap(ctx, s_bmp_moon, GPoint(35, 35), angle, c_moon);

        graphics_context_set_fill_color(ctx, c_bg);
        graphics_fill_circle(ctx, GPoint(c_moon.x - 20, c_moon.y + 12), 22);
        graphics_fill_circle(ctx, GPoint(c_moon.x + 20, c_moon.y + 12), 22);
        graphics_fill_circle(ctx, GPoint(c_moon.x, c_moon.y + 18), 8);
        graphics_fill_rect(ctx, GRect(10, c_moon.y + 24, 180, 40), 0, GCornerNone);
    }

    graphics_context_set_stroke_color(ctx, c_bord);
    graphics_context_set_stroke_width(ctx, 5);
    graphics_draw_round_rect(ctx, GRect(2, safe.origin.y + 2, safe.size.w - 4, safe.size.h - 4), 18);

    graphics_context_set_fill_color(ctx, GColorBlack); 
    graphics_fill_rect(ctx, GRect(c_main.x - 3, safe.origin.y, 6, 10), 0, GCornerNone); 
    graphics_fill_rect(ctx, GRect(c_main.x - 3, safe.origin.y + safe.size.h - 10, 6, 10), 0, GCornerNone); 
    graphics_fill_rect(ctx, GRect(0, c_main.y - 3, 10, 6), 0, GCornerNone); 
    graphics_fill_rect(ctx, GRect(safe.size.w - 10, c_main.y - 3, 10, 6), 0, GCornerNone); 

    graphics_context_set_text_color(ctx, COL_TEXT);
    int ty = safe.origin.y + 8;
    if (s_show_battery) {
        static char s_batt[8]; snprintf(s_batt, sizeof(s_batt), "%d%%", s_battery_level);
        graphics_draw_text(ctx, s_batt, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0, ty + 4, 200, 30), 0, GTextAlignmentCenter, NULL);
    } else {
        graphics_draw_text(ctx, "12", fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS), GRect(0, ty, 200, 40), 0, GTextAlignmentCenter, NULL);
    }

    graphics_context_set_fill_color(ctx, COL_METAL);
    graphics_context_set_text_color(ctx, COL_METAL);
    
    for (int i = 0; i < 31; i++) {
        int32_t a = (TRIG_MAX_ANGLE * i) / 31;
        GPoint p = { .x = c_date.x + (sin_lookup(a) * 28 / TRIG_MAX_RATIO), .y = c_date.y - (cos_lookup(a) * 28 / TRIG_MAX_RATIO) };
        graphics_fill_circle(ctx, p, 1);
        if (i == 0)  draw_arc_label(ctx, c_date, 40, 0, "31");
        if (i == 10) draw_arc_label(ctx, c_date, 40, (10 * 360) / 31, "10");
        if (i == 20) draw_arc_label(ctx, c_date, 40, (20 * 360) / 31, "20");
    }

    char* days[] = {"S", "M", "D", "M", "D", "F", "S"};
    for (int i = 0; i < 7; i++) draw_arc_label(ctx, c_day, 22, (i * 360) / 7, days[i]);
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
    time_t now = time(NULL); struct tm *t = localtime(&now);
    bool is_night = (t->tm_hour >= 22 || t->tm_hour < 6);
    GColor accent = is_night ? GColorScreaminGreen : COL_TEXT;
    
    GRect safe = layer_get_unobstructed_bounds(layer);

    static char s_t_buf[8]; strftime(s_t_buf, sizeof(s_t_buf), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
    graphics_context_set_text_color(ctx, COL_METAL);
    graphics_draw_text(ctx, s_t_buf, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0, safe.origin.y + safe.size.h - 34, 200, 20), 0, GTextAlignmentCenter, NULL);

    draw_hand(ctx, s_path_sub, c_date, (TRIG_MAX_ANGLE * (t->tm_mday % 31)) / 31, accent);
    draw_hand(ctx, s_path_sub, c_day, (TRIG_MAX_ANGLE * t->tm_wday) / 7, accent);
    draw_hand(ctx, s_path_minute, c_main, (TRIG_MAX_ANGLE * t->tm_min) / 60, accent);
    draw_hand(ctx, s_path_hour, c_main, ((TRIG_MAX_ANGLE * (t->tm_hour % 12)) / 12) + ((TRIG_MAX_ANGLE * t->tm_min) / 720), COL_HOUR_HAND);
    
    graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, c_main, 4);
    graphics_context_set_stroke_color(ctx, COL_METAL); graphics_draw_circle(ctx, c_main, 4);
}

static void main_window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    
    update_layout(layer_get_unobstructed_bounds(root));

    static const GPoint P_HOUR[] = {{-5, 12}, {-5, -60}, {-2, -63}, {2, -63}, {5, -60}, {5, 12}};
    GPathInfo i_hour = { .num_points = 6, .points = (GPoint*)P_HOUR };
    s_path_hour = gpath_create(&i_hour);

    static const GPoint P_MIN[] = {{-4, 12}, {-4, -98}, {-2, -101}, {2, -101}, {4, -98}, {4, 12}};
    GPathInfo i_min = { .num_points = 6, .points = (GPoint*)P_MIN };
    s_path_minute = gpath_create(&i_min);

    static const GPoint P_SUB[] = {{-3, 10}, {0, -28}, {3, 10}};
    GPathInfo i_sub = { .num_points = 3, .points = (GPoint*)P_SUB };
    s_path_sub = gpath_create(&i_sub);

    s_bmp_moon = gbitmap_create_with_resource(RESOURCE_ID_MOON);
    
    s_bg_layer = layer_create(layer_get_bounds(root)); 
    layer_set_update_proc(s_bg_layer, bg_update_proc); 
    layer_add_child(root, s_bg_layer);
    
    s_hands_layer = layer_create(layer_get_bounds(root)); 
    layer_set_update_proc(s_hands_layer, hands_update_proc); 
    layer_add_child(root, s_hands_layer);

    accel_tap_service_subscribe(tap_handler);
    
    UnobstructedAreaHandlers handlers = { .change = unobstructed_area_change_proc };
    unobstructed_area_service_subscribe(handlers, NULL);
    
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe(); 
    unobstructed_area_service_unsubscribe();
    
    layer_destroy(s_bg_layer); 
    layer_destroy(s_hands_layer);
    
    if (s_bmp_moon) gbitmap_destroy(s_bmp_moon);
    if (s_path_hour) gpath_destroy(s_path_hour);
    if (s_path_minute) gpath_destroy(s_path_minute);
    if (s_path_sub) gpath_destroy(s_path_sub);
}

static void init() {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){.load=main_window_load, .unload=main_window_unload});
    window_stack_push(s_window, true);
}

int main() { init(); app_event_loop(); window_destroy(s_window); }