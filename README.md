# VANTAGE (Cross-Platform Edition)

**Version:** 2.0.0  
**Platform:** Pebble Time 2 (Emery)  
**Platform:** Pebble Time (Basalt)  
**Platform:** Pebble 2 (Diorite)  
**Platform:** Pebble 2 Duo (Flint)  
**Platform:** Classic, Steel (Aplite)  

**Author:** Atomlabor | Atomlabor.de  

![vantage Header](https://raw.githubusercontent.com/atomlabor/vantage/main/VANTAGE_WATCHFACE.png)

## Rebble / Pebble Shoplink:
https://apps.rebble.io/en_US/application/6970a55f04726500096d9975 

## Overview
VANTAGE is a high-precision, native C watchface designed for the entire Pebble ecosystem (excluding Round). It features a sophisticated analog design with "Midnight Green" aesthetics and deep system integration. 

Built on a lightweight, 100% vector-based rendering engine, VANTAGE completely abandons static bitmaps in favor of real-time mathematical drawing. It utilizes the `UnobstructedAreaService` to mathematically adapt its entire layout when system notifications (Quick Views) appear, ensuring no information is ever obscured.

## Visual Identity
* **Background:** Midnight Green (Day) / Pitch Black (Night)
* **Border:** Cadet Blue (Day) / Pitch Black (Night)
* **Hands (Day):** Red (Hour), White (Minute), White (Subdials)
* **Hands (Night):** Glowing Screamin' Green (Color Displays) / White (Monochrome)
* **Typography:** LECO 32 (Indices), Gothic 14 Bold (Labels)

![vantage Header](https://raw.githubusercontent.com/atomlabor/vantage/main/pebble_screenshot.png)

## Key Features

### 1. Vector-Engine Moon Phase
A purely mathematical, vector-drawn moon phase at the 6 o'clock position—calculated down to the exact second of the lunar cycle (2,551,443 seconds) using a verified historical anchor. 
* **Zero Bitmaps:** No pixel-distortion, no internet connection required, and minimal memory footprint.
* **Color Adaptation:** Renders crisp Yellow moons on a deep Oxford Blue orbit (Color displays) or high-contrast White moons on Black (Monochrome displays).

### 2. Adaptive "Quick View" Layout 
The watchface listens to the Pebble OS `UnobstructedAreaService`. When a timeline pin or notification slides up from the bottom:
* The entire face (center point, subdials, hands) slides up smoothly.
* Elements that would overlap or be cut off are automatically hidden.
* Upon dismissal, the interface glides perfectly back to its original position.

### 3. Stealth Mode (Radar Look)
To reduce glare and maintain a low profile in dark environments:
* **22:00 - 06:00:** The background, Cadet Blue borders, subdials, and typography completely fade into the pitch-black screen.
* The watch transforms into a minimalist "Radar" instrument, leaving only the main hands illuminated in striking `GColorScreaminGreen`.

### 4. Precision Subdials
* **Left Subdial:** Date indicator (0-31 scale).
* **Right Subdial:** Day of week indicator (S-M-T-W-T-F-S).

### 5. Zero-Crash Architecture
* **Safe Initialization:** Uses stack-based GPath construction to prevent memory segmentation faults during startup.
* **Bitmap-Free:** Avoids `gbitmap` memory allocation bottlenecks entirely.
* **Layer Safety:** All drawing operations perform null-checks on pointers before rendering.

## Interaction
* **Tap / Shake:** Reveals the battery percentage at the top (12 o'clock position) for 5 seconds. Afterwards, it automatically reverts to the standard "12" index.

## Technical Specifications
* **Language:** C (Native)
* **SDK:** Pebble SDK 4.0+
* **Resolution:** 200 x 228 px (Emery) & 144 x 168 px (Basalt, Aplite, Diorite)
* **Connectivity:** Standalone (No Companion App, No Internet required).
* **Sensors:** Accelerometer (Tap Service).

## Build Instructions
1. Ensure the Pebble SDK is installed.
2. Clean previous build artifacts:
   ```bash
   pebble clean

---
*© 2026 Atomlabor. All rights reserved.*
