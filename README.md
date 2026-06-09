# Claw'd on ESP32 + ST7789

Claude Code's pixel crab mascot, **Claw'd**, animated on a TTGO T-Display
(ESP32 + 240×135 ST7789V color IPS TFT).

![mode](https://img.shields.io/badge/anim-WALK%20·%20STOMP%20·%20FLAG%20·%20GYM%20·%20LOOK%20·%20NAP-D3795A)

## Hardware

- **Board:** TTGO T-Display (ESP32-D0WDQ6, 16 MB flash)
- **Display:** ST7789V, 240×135, SPI
- **Pin map**

  | Signal | GPIO |
  |--------|------|
  | MOSI   | 19   |
  | SCLK   | 18   |
  | CS     | 5    |
  | DC     | 16   |
  | RST    | 23   |
  | BL     | 4    |
  | BTN_L  | 35   |
  | BTN_R  | 0    |

## Animations

Six poses faithful to the official Claw'd animations, auto-cycling every 7 s.
The two onboard buttons switch modes (left = previous, right = next):

1. **WALK** — looks around, crouches, jumps across the screen in an arc, lands, leaps back
2. **STOMP** — tilts left/right stomping, firing confetti bursts
3. **FLAG** — waves a flag while the body sways the opposite way
4. **GYM** — dumbbell curls
5. **LOOK** — eyes dart around
6. **NAP** — eyes closed, breathing, floating "Z"s

## Build & flash

Uses [arduino-cli](https://arduino.github.io/arduino-cli/) with the ESP32 core
and the *GFX Library for Arduino*.

```sh
arduino-cli core install esp32:esp32
arduino-cli lib install "GFX Library for Arduino"

# NOTE: this USB-serial adapter is unreliable at the default 921600 baud,
# so pin the upload speed to 115200.
PORT=/dev/cu.usbserial-XXXXXXXXXX
arduino-cli compile --fqbn esp32:esp32:esp32 claude_mascot
arduino-cli upload  --fqbn esp32:esp32:esp32:UploadSpeed=115200 -p $PORT claude_mascot
```

## Layout

- `claude_mascot/` — the mascot firmware (sprite + animations)
- `tdisplay_test/` — ST7789 bring-up / color-bar test
- `i2c_scan/` — multi-pin I2C scanner used while identifying the display
