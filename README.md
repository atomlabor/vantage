# VANTAGE (Emery (obelix) Edition)
**Version:** 1.0 
**Platform:** Pebble Time 2 (Emery)
**Author:** Atomlabor | Atomlabor.de

![vantage Header](https://raw.githubusercontent.com/atomlabor/vantage/main/VANTAGE_WATCHFACE.png)

## Rebble / Pebble Shoplink:
https://apps.rebble.io/en_US/application/6970a55f04726500096d9975 

## Overview
VANTAGE is a high-precision, native C watchface designed exclusively for the Pebble Time 2 (Emery). It features a sophisticated analog design with "Midnight Green" aesthetics and deep system integration.

Unlike standard watchfaces, VANTAGE utilizes the `UnobstructedAreaService` to mathematically adapt its entire layout in real-time when system notifications (Quick Views) appear, ensuring no information is ever obscured.

## Visual Identity
* **Background:** Midnight Green (Day) / Black (Night)
* **Border:** Cadet Blue (Day) / Black (Night)
* **Hands:** Red (Hour), White (Minute), White (Subdials)
* **Typography:** LECO 32 (Indices), Gothic 14 Bold (Labels)


![vantage Header](https://raw.githubusercontent.com/atomlabor/vantage/main/pebble_screenshot.png)


## Key Features

### 1. Adaptive Layout 
The watchface listens to the Pebble OS `UnobstructedAreaService`. When a timeline pin or notification (Quick View) slides up from the bottom:
* The entire face (center point, subdials, hands) slides up smoothly.
* Elements that would overlap or be cut off (like the Moon Phase) are automatically hidden.
* Upon dismissal, the interface glides back to its original position.

### 2. Stealth Mode
To reduce glare and maintain a low profile in dark environments:
* **22:00 - 06:00:** The background and the distinctive Cadet Blue border turn completely Black (`GColorBlack`).
* The watchface seamlessly blends with the bezel of the watch.

### 3. Precision Subdials & Moon Phase
* **Left Subdial:** Date indicator (0-31 scale).
* **Right Subdial:** Day of week indicator (S-M-D-M-D-F-S).
* **Moon Phase:** A mathematically calculated moon phase (no internet required) at the 6 o'clock position. Includes a custom mask for a seamless "noble" style integration.

### 4. Zero-Crash Architecture
* **Safe Initialization:** Uses stack-based GPath construction to prevent memory segmentation faults during startup.
* **Layer Safety:** All drawing operations perform null-checks on pointers before rendering.

## Interaction
* **Tap / Shake:** Reveals the battery percentage at the top (12 o'clock position) for 5 seconds. Afterwards, it automatically reverts to the "12" index.

## Technical Specifications
* **Language:** C (Native)
* **SDK:** Pebble SDK 4.0+
* **Resolution:** 200 x 228 px (Emery)
* **Connectivity:** Standalone (No Companion App, No Internet required).
* **Sensors:** Accelerometer (Tap Service).

## Build Instructions
1.  Ensure the Pebble SDK is installed and the target is set to `emery`.
2.  Clean previous build artifacts:
    ```bash
    pebble clean
    ```
3.  Compile the Golden Build:
    ```bash
    pebble build
    ```
4.  Install:
    ```bash
    pebble install
    ```

---
*© 2026 Atomlabor. All rights reserved.*
