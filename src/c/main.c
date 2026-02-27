#include <pebble.h>

// --------------------------------------------------------------------------
// VANTAGE v2.0.0 (Cross-Platform minus Round) by Jens Mahnke
// atomlabor.de Watchface
// --------------------------------------------------------------------------

#define TEST_NIGHTMODE 0 

static Window *s_window;
static Layer *s_bg_layer, *s_hands_layer;
static GPath *s_path_hour = NULL;
static GPath *s_path_minute = NULL;
static GPath *s_path_sub = NULL;

static bool s_show_battery = false;
static int s_battery_level = 0;
static AppTimer *s_battery_timer = NULL;

static GPoint c_main;
static GPoint c_date; 
static GPoint c_day; 

#define SHOW_MOON 1
static GPoint c_moon;

#define COL_BG          PBL_IF_COLOR_ELSE(GColorMidnightGreen, GColorBlack)
#define COL_NIGHT       GColorBlack
#define COL_BORDER      PBL_IF_COLOR_ELSE(GColorCadetBlue, GColorWhite)
#define COL_HOUR_HAND   PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)
#define COL_TEXT        GColorWhite
#define COL_METAL       GColorWhite
#define COL_ACCENT_N    PBL_IF_COLOR_ELSE(GColorScreaminGreen, GColorWhite)

#if defined(PBL_PLATFORM_EMERY)
  #define LAYOUT_OFFSET_X 50
  #define LAYOUT_OFFSET_Y 68
  #define TIME_Y_OFFSET 34
  #define SAFE_H_THRESHOLD 160
  #define SUB_DIAL_R 28
  #define ARC_RADIUS_DATE 40
  #define ARC_RADIUS_DAY 22
  #define BATT_Y_OFFSET 4
  #define FONT_NUMBERS FONT_KEY_LECO_32_BOLD_NUMBERS
  #define FONT_TEXT FONT_KEY_GOTHIC_24_BOLD
  #define DATE_PNT 1
  #define TIME_FONT FONT_KEY_GOTHIC_14
  #define TIME_H 20

#else
  #define LAYOUT_OFFSET_X 36
  #define LAYOUT_OFFSET_Y 56
  #define TIME_Y_OFFSET 26
  #define SAFE_H_THRESHOLD 120
  #define SUB_DIAL_R 20
  #define ARC_RADIUS_DATE 28
  #define ARC_RADIUS_DAY 16
  #define BATT_Y_OFFSET 0
  #define FONT_NUMBERS FONT_KEY_GOTHIC_28_BOLD
  #define FONT_TEXT FONT_KEY_GOTHIC_18_BOLD
  #define DATE_PNT 1
  #define TIME_FONT FONT_KEY_GOTHIC_14
  #define TIME_H 20
#endif

static bool is_night_time(struct tm *t) {
    #if TEST_NIGHTMODE
        return true;
    #else
        return (t->tm_hour >= 22 || t->tm_hour < 6);
    #endif
}

static void update_layout(GRect bounds) {
    if (bounds.size.w == 0 || bounds.size.h == 0) return;
    
    c_main = GPoint(bounds.origin.x + (bounds.size.w / 2), bounds.origin.y + (bounds.size.h / 2));
    c_date = GPoint(c_main.x - LAYOUT_OFFSET_X, c_main.y);
    c_day  = GPoint(c_main.x + LAYOUT_OFFSET_X, c_main.y);
    c_moon = GPoint(c_main.x, c_main.y + LAYOUT_OFFSET_Y); 
}

static void unobstructed_area_change_proc(AnimationProgress progress, void *context) {
    (void)progress;
    (void)context;
    
    if (!s_window) return;
    Layer *root = window_get_root_layer(s_window);
    if (!root) return;

    update_layout(layer_get_unobstructed_bounds(root));
    
    if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    if (s_hands_layer) layer_mark_dirty(s_hands_layer);
}

static void battery_timer_callback(void *data) {
    (void)data;
    s_show_battery = false;
    if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    s_battery_timer = NULL;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
    (void)axis;
    (void)direction;
    
    s_show_battery = true;
    s_battery_level = battery_state_service_peek().charge_percent;
    if (s_bg_layer) layer_mark_dirty(s_bg_layer);
    if (s_battery_timer) app_timer_cancel(s_battery_timer);
    s_battery_timer = app_timer_register(5000, battery_timer_callback, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    (void)tick_time;
    
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
    
    bool is_night = is_night_time(t);
    
    GRect full = layer_get_bounds(layer);
    GRect safe = layer_get_unobstructed_bounds(layer); 

    GColor c_bg = is_night ? COL_NIGHT : COL_BG;
    GColor c_bord = is_night ? COL_NIGHT : COL_BORDER;
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, full, 0, GCornerNone);
    
    graphics_context_set_fill_color(ctx, c_bg);
    graphics_fill_rect(ctx, GRect(2, safe.origin.y + 2, safe.size.w - 4, safe.size.h - 4), 18, GCornersAll);
    
    if (safe.size.h > SAFE_H_THRESHOLD) {
        const uint32_t lunation_seconds = 2551443; 
        const time_t new_moon_anchor = 1421759640; 

        time_t current_time = now;
        if (current_time < new_moon_anchor) current_time = new_moon_anchor; 
        time_t delta = current_time - new_moon_anchor;

        int32_t angle = (int32_t)(((uint64_t)(delta % lunation_seconds) * (TRIG_MAX_ANGLE / 2)) / lunation_seconds) - (TRIG_MAX_ANGLE / 4);

        graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorOxfordBlue, GColorBlack));
        graphics_fill_circle(ctx, c_moon, 36);

        int orbit_r = 17; 
        int moon_r = 12;  

        GPoint m1 = {
            .x = c_moon.x + (sin_lookup(angle) * orbit_r / TRIG_MAX_RATIO),
            .y = c_moon.y - (cos_lookup(angle) * orbit_r / TRIG_MAX_RATIO)
        };

        int32_t angle2 = angle + (TRIG_MAX_ANGLE / 2);
        GPoint m2 = {
            .x = c_moon.x + (sin_lookup(angle2) * orbit_r / TRIG_MAX_RATIO),
            .y = c_moon.y - (cos_lookup(angle2) * orbit_r / TRIG_MAX_RATIO)
        };

        graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite));
        graphics_fill_circle(ctx, m1, moon_r);
        graphics_fill_circle(ctx, m2, moon_r);

        graphics_context_set_fill_color(ctx, c_bg);
        #if defined(PBL_PLATFORM_EMERY)
            graphics_context_set_stroke_color(ctx, c_bg);
            graphics_context_set_stroke_width(ctx, 30);
            graphics_draw_circle(ctx, c_moon, 52); 
            graphics_fill_circle(ctx, GPoint(c_moon.x - 20, c_moon.y + 12), 22);
            graphics_fill_circle(ctx, GPoint(c_moon.x + 20, c_moon.y + 12), 22);
            graphics_fill_rect(ctx, GRect(20, c_moon.y + 18, safe.size.w - 40, 40), 0, GCornerNone);
        #else
            graphics_context_set_stroke_color(ctx, c_bg);
            graphics_context_set_stroke_width(ctx, 24);
            graphics_draw_circle(ctx, c_moon, 46); 
            graphics_fill_circle(ctx, GPoint(c_moon.x - 16, c_moon.y + 10), 18);
            graphics_fill_circle(ctx, GPoint(c_moon.x + 16, c_moon.y + 10), 18);
            graphics_fill_rect(ctx, GRect(20, c_moon.y + 14, safe.size.w - 40, 40), 0, GCornerNone);
        #endif
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
        graphics_draw_text(ctx, s_batt, fonts_get_system_font(FONT_TEXT), GRect(0, ty + BATT_Y_OFFSET, safe.size.w, 30), 0, GTextAlignmentCenter, NULL);
    } else {
        graphics_draw_text(ctx, "12", fonts_get_system_font(FONT_NUMBERS), GRect(0, ty, safe.size.w, 40), 0, GTextAlignmentCenter, NULL);
    }

    if (!is_night) {
        graphics_context_set_fill_color(ctx, COL_METAL);
        graphics_context_set_text_color(ctx, COL_METAL);
        
        for (int i = 0; i < 31; i++) {
            int32_t a = (TRIG_MAX_ANGLE * i) / 31;
            GPoint p = { .x = c_date.x + (sin_lookup(a) * SUB_DIAL_R / TRIG_MAX_RATIO), .y = c_date.y - (cos_lookup(a) * SUB_DIAL_R / TRIG_MAX_RATIO) };
            graphics_fill_circle(ctx, p, DATE_PNT);
            if (i == 0)  draw_arc_label(ctx, c_date, ARC_RADIUS_DATE, 0, "31");
            if (i == 10) draw_arc_label(ctx, c_date, ARC_RADIUS_DATE, (10 * 360) / 31, "10");
            if (i == 20) draw_arc_label(ctx, c_date, ARC_RADIUS_DATE, (20 * 360) / 31, "20");
        }

        char* days[] = {"S", "M", "T", "W", "T", "F", "S"};
        for (int i = 0; i < 7; i++) draw_arc_label(ctx, c_day, ARC_RADIUS_DAY, (i * 360) / 7, days[i]);
    }
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
    time_t now = time(NULL); struct tm *t = localtime(&now);
    
    bool is_night = is_night_time(t);
    
    GColor accent_min = is_night ? COL_ACCENT_N : COL_TEXT;
    GColor accent_hour = is_night ? COL_ACCENT_N : COL_HOUR_HAND;
    
    GRect safe = layer_get_unobstructed_bounds(layer);

    static char s_t_buf[8]; strftime(s_t_buf, sizeof(s_t_buf), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
    graphics_context_set_text_color(ctx, COL_METAL);
    graphics_draw_text(ctx, s_t_buf, fonts_get_system_font(TIME_FONT), GRect(0, safe.origin.y + safe.size.h - TIME_Y_OFFSET, safe.size.w, TIME_H), 0, GTextAlignmentCenter, NULL);

    if (!is_night) {
        draw_hand(ctx, s_path_sub, c_date, (TRIG_MAX_ANGLE * (t->tm_mday % 31)) / 31, COL_TEXT);
        draw_hand(ctx, s_path_sub, c_day, (TRIG_MAX_ANGLE * t->tm_wday) / 7, COL_TEXT);
    }
    
    draw_hand(ctx, s_path_minute, c_main, (TRIG_MAX_ANGLE * t->tm_min) / 60, accent_min);
    draw_hand(ctx, s_path_hour, c_main, ((TRIG_MAX_ANGLE * (t->tm_hour % 12)) / 12) + ((TRIG_MAX_ANGLE * t->tm_min) / 720), accent_hour);
    
    graphics_context_set_fill_color(ctx, GColorBlack); 
    #if defined(PBL_PLATFORM_EMERY)
        graphics_fill_circle(ctx, c_main, 4);
        graphics_context_set_stroke_color(ctx, COL_METAL); graphics_draw_circle(ctx, c_main, 4);
    #else
        graphics_fill_circle(ctx, c_main, 3);
        graphics_context_set_stroke_color(ctx, COL_METAL); graphics_draw_circle(ctx, c_main, 3);
    #endif
}

static void main_window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    
    update_layout(layer_get_unobstructed_bounds(root));

    #if defined(PBL_PLATFORM_EMERY)
        static const GPoint P_HOUR[] = {{-5, 12}, {-5, -60}, {-2, -63}, {2, -63}, {5, -60}, {5, 12}};
        static const GPoint P_MIN[]  = {{-4, 12}, {-4, -98}, {-2, -101}, {2, -101}, {4, -98}, {4, 12}};
        static const GPoint P_SUB[]  = {{-3, 10}, {0, -28}, {3, 10}};
    #else
        static const GPoint P_HOUR[] = {{-4, 9}, {-4, -43}, {-1, -45}, {1, -45}, {4, -43}, {4, 9}};
        static const GPoint P_MIN[]  = {{-3, 9}, {-3, -71}, {-1, -73}, {1, -73}, {3, -71}, {3, 9}};
        static const GPoint P_SUB[]  = {{-2, 7}, {0, -20}, {2, 7}};
    #endif

    GPathInfo i_hour = { .num_points = 6, .points = (GPoint*)P_HOUR };
    s_path_hour = gpath_create(&i_hour);

    GPathInfo i_min = { .num_points = 6, .points = (GPoint*)P_MIN };
    s_path_minute = gpath_create(&i_min);

    GPathInfo i_sub = { .num_points = 3, .points = (GPoint*)P_SUB };
    s_path_sub = gpath_create(&i_sub);

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
