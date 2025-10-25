# ESP32 HUD Firmware

This project powers a minimalist heads-up display (HUD) using an ESP32 and an ST7735 TFT screen. It receives real-time speed data and speed limit updates via Bluetooth from a companion Android app, and renders a smooth, readable interface suitable for in-car use. Pretty snappy too if I do say so myself.

---

## Features

- **Bluetooth-connected HUD**: Displays current speed received from your Android device.
- **Speed limit indicator**: Shows a square road-sign-style icon in the bottom-right corner. Blinks red when the driver exceeds the limit.
- **Smooth & Calibrated Speed Display**: Incoming GPS speed is slightly adjusted (+3% +1 km/h) to mimic real car speedometers, then smoothed with interpolation for natural transitions.
- **Connection status indicator**: A small circle in the top-left corner shows green when connected, red when disconnected.
- **Demo mode**: Simulates acceleration and deceleration without needing a phone.
- **Boot animation**: Fades in a logo (Use your own logo by [converting](https://javl.github.io/image2cpp/) it into Monochrome, 1‑bit) with PWM backlight control.

---

## Android Companion App

This firmware is designed to work seamlessly with my [ESP32 HUD Android App](https://github.com/karunish/esp32-hud-app).

---

## 🛠️ Hardware Requirements

- ESP32
- ST7735S TFT Display (SPI) (I tested the code on the Blue tab display)
- PWM-capable backlight pin (The code uses GPIO 19)
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

```cpp
bool DEMO_MODE = false; // Set to true to simulate speed without Bluetooth, aka Demo Mode
float smoothing = 0.15; // Controls speed interpolation responsiveness
//Check the pins for your specific ESP32 module. The code was made to work on the ESP32 Dev Board
