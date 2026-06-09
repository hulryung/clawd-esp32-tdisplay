# Claw'd on ESP32 + ST7789

Claude Code's pixel crab mascot, **Claw'd**, animated on a TTGO T-Display
(ESP32 + 240×135 ST7789V color IPS TFT).

## Demo (real hardware)

<img src="assets/device_demo.gif" width="360" alt="Claw'd running on the device">

*Recorded on the actual board.* Full-quality clip: [assets/device_demo.mp4](assets/device_demo.mp4)

## Hardware

- **Board:** TTGO T-Display (ESP32-D0WDQ6, 16 MB flash)
- **Display:** ST7789V, 240×135, SPI
- **Pin map**

  | Signal | GPIO |        | Signal | GPIO |
  |--------|------|--------|--------|------|
  | MOSI   | 19   |        | DC     | 16   |
  | SCLK   | 18   |        | RST    | 23   |
  | CS     | 5    |        | BL     | 4    |
  | BTN_L  | 35   |        | BTN_R  | 0    |

## Animations

Nine poses faithful to the official Claw'd animations, auto-cycling every 7 s.
The two onboard buttons switch modes (left = previous, right = next).

For each animation below: the **GIF** is the live loop, the **four stills** are key frames.
All of these are rendered by [`tools/render_anim.py`](tools/render_anim.py), a 1:1 port
of the firmware's sprite/pose math — so the docs match the device pixel-for-pixel.

### 1. WALK
Looks around, crouches, jumps across the screen in an arc, lands, leaps back.

<img src="assets/walk.gif" width="360"><br>
<img src="assets/keys/walk_k0.png" width="150"> <img src="assets/keys/walk_k1.png" width="150"> <img src="assets/keys/walk_k2.png" width="150"> <img src="assets/keys/walk_k3.png" width="150">

### 2. STOMP
Tilts left and right stomping, firing confetti bursts at each peak.

<img src="assets/stomp.gif" width="360"><br>
<img src="assets/keys/stomp_k0.png" width="150"> <img src="assets/keys/stomp_k1.png" width="150"> <img src="assets/keys/stomp_k2.png" width="150"> <img src="assets/keys/stomp_k3.png" width="150">

### 3. FLAG
Waves a flag while the body sways the opposite way.

<img src="assets/flag.gif" width="360"><br>
<img src="assets/keys/flag_k0.png" width="150"> <img src="assets/keys/flag_k1.png" width="150"> <img src="assets/keys/flag_k2.png" width="150"> <img src="assets/keys/flag_k3.png" width="150">

### 4. GYM
Dumbbell curls.

<img src="assets/gym.gif" width="360"><br>
<img src="assets/keys/gym_k0.png" width="150"> <img src="assets/keys/gym_k1.png" width="150"> <img src="assets/keys/gym_k2.png" width="150"> <img src="assets/keys/gym_k3.png" width="150">

### 5. LOOK
Eyes dart left, center, right.

<img src="assets/look.gif" width="360"><br>
<img src="assets/keys/look_k0.png" width="150"> <img src="assets/keys/look_k1.png" width="150"> <img src="assets/keys/look_k2.png" width="150"> <img src="assets/keys/look_k3.png" width="150">

### 6. NAP
Eyes closed, gentle breathing.

<img src="assets/nap.gif" width="360"><br>
<img src="assets/keys/nap_k0.png" width="150"> <img src="assets/keys/nap_k1.png" width="150"> <img src="assets/keys/nap_k2.png" width="150"> <img src="assets/keys/nap_k3.png" width="150">

### 7. WAVE
Waves hello with a swinging hand.

<img src="assets/wave.gif" width="360"><br>
<img src="assets/keys/wave_k0.png" width="150"> <img src="assets/keys/wave_k1.png" width="150"> <img src="assets/keys/wave_k2.png" width="150"> <img src="assets/keys/wave_k3.png" width="150">

### 8. APPLE
A dropped apple falls; Claw'd scuttles over, eats it, and hops happily.

<img src="assets/apple.gif" width="360"><br>
<img src="assets/keys/apple_k0.png" width="150"> <img src="assets/keys/apple_k1.png" width="150"> <img src="assets/keys/apple_k2.png" width="150"> <img src="assets/keys/apple_k3.png" width="150">

### 9. SPIN
Turns around in place.

<img src="assets/spin.gif" width="360"><br>
<img src="assets/keys/spin_k0.png" width="150"> <img src="assets/keys/spin_k1.png" width="150"> <img src="assets/keys/spin_k2.png" width="150"> <img src="assets/keys/spin_k3.png" width="150">

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

## Regenerate the GIFs / PNGs

```sh
python3 -m pip install Pillow
python3 tools/render_anim.py     # writes assets/*.gif and assets/keys/*.png
```

## Layout

- `claude_mascot/` — the mascot firmware (`claude_mascot.ino` + `pose.h`)
- `tools/render_anim.py` — renders the animation GIFs and key-frame PNGs
- `tdisplay_test/` — ST7789 bring-up / color-bar test
- `i2c_scan/` — multi-pin I2C scanner used while identifying the display
- `assets/` — generated GIFs, key-frame PNGs, and the device demo clip
