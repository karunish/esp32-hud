# ESP32 HUD Firmware

This project powers a minimalist heads-up display (HUD) using an ESP32 and an ST7735 TFT screen. It receives real-time speed data and speed limit updates via Bluetooth from a [companion Android app](https://github.com/karunish/esp32-hud-app), and renders a smooth, readable interface suitable for in-car use. Pretty snappy too if I do say so myself.

---

## Features

- **Bluetooth-connected HUD**: Displays current speed received from your Android device.
- **Speed limit indicator**: Shows a square road-sign-style icon in the bottom-right corner. Blinks red when the driver exceeds the limit.
- **Smooth & Calibrated Speed Display**: Incoming GPS speed is slightly adjusted (+3% +1 km/h) to mimic real car speedometers, then smoothed with interpolation for natural transitions.
- **Connection status indicator**: A small circle in the top-left corner shows green when connected, red when disconnected.
- **Demo mode**: Simulates acceleration and deceleration without needing a phone.
- **Boot animation**: Fades in a logo (use your own logo by converting it into Monochrome, 1‑bit) with PWM backlight control.

---

## Android Companion App

This firmware is designed to work seamlessly with my [ESP32 HUD Android App](https://github.com/karunish/esp32-hud-app).

---

## Hardware Requirements

- ESP32
- ST7735S TFT Display (SPI) (I tested the code on the Blue tab display)
- PWM-capable backlight pin (the code uses GPIO 19)
- Power source (USB or hardwired to car, I personally use USB)

### Wiring

| ESP32 Pin | TFT Pin     |
|-----------|-------------|
| GPIO 5    | CS          |
| GPIO 2    | DC          |
| GPIO 4    | RST         |
| GPIO 23   | MOSI        |
| GPIO 18   | SCK         |
| GND       | GND         |
| 3.3V      | VCC         |
| GPIO 19   | BLK         |

---

## Configuration

Inside the firmware:

``` cpp
bool DEMO_MODE = false; // Set to true to simulate speed without Bluetooth, aka Demo Mode
float smoothing = 0.15; // Controls speed interpolation responsiveness
// Check the pins for your specific ESP32 module. The code was made to work on the ESP32 Dev Board
```

---

## Custom Boot Logo

The boot logo is stored as a **1‑bit monochrome bitmap array in PROGMEM**.  
You can generate your own logo using the [Image2CPP converter](https://javl.github.io/image2cpp/):

- Upload a black/white PNG (ideally square, e.g. 100×100 px).
- Select **Monochrome, 1‑bit**.
- Choose **Arduino Code Output**.
- Copy the generated `const unsigned char ... PROGMEM` array into the firmware, replacing the placeholder.

---

## HUD Reflection Flip

To make the display render correctly when reflected on glass, you may need to flip the ST7735 output.  
This is done by editing the `setRotation()` function in the **Adafruit ST7735 library**:

- Open `Adafruit_ST7735.cpp`.
- Find the section where `MADCTL` is written.
- Toggle the `MADCTL_MX` bit to mirror the X axis (left/right).
- Recompile and upload.

This ensures the speed and icons appear correctly when projected onto a windshield.

---

## Building & Flashing

- Open the project in Arduino IDE or PlatformIO.
- Select **ESP32 Dev Module** as the board.
- Install required libraries:
  - `Adafruit GFX`
  - `Adafruit ST7735 and ST7789`
  - `BluetoothSerial` (built into ESP32 Arduino core)
- Connect your ESP32 via USB and upload.

---

## Bluetooth Protocol

- Speed values are sent as floats (e.g. `42.0\n`).
- Speed limits are sent as `LIMIT:XX\n` (e.g. `LIMIT:80\n`).


