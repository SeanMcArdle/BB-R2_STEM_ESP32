# BB-R2 STEM ESP32 — Bakken Museum Workshop Fork (WiFi v13)

This fork adapts BB-R2 for youth workshops at The Bakken Museum (Minneapolis, MN) led by Seán McArdle on Dec 30–31, 2025. It replaces Bluetooth/Dabble control with a self-hosted WiFi Access Point and a browser-based control interface, and adds a polished curriculum so kids can build, wire, connect, and drive their droids in one session.

Seán McArdle (fork author) developed and tuned the workshop version:
- v13 controller with smooth motion (writeMicroseconds for dome, tuned smoothing, 1000 ms safety timeout)
- WebSocket telemetry + web serial log
- Battery monitor feature (voltage divider + UI indicator)
- Full two-day curriculum and session archive for teaching

## Quick Links
- WiFi Controller (code & guide): `esp32/BB-R2-WiFi/README.md`
- Curriculum: `docs/curriculum-droid-workshop.md`
- Session Archive (A→Z story): `docs/session-log-2025-12-13.md`
- Bill of Materials (BOM): `docs/BOM.md`
- Changelog: `docs/CHANGELOG.md`
- Workshop site: https://thebakken.org/break-camps

## Workshop Highlights (Seán McArdle)
- WiFi AP mode (no router): each droid hosts its own network
- Web UI joystick + dome slider; real-time telemetry and logs
- Tuning kids can “feel”: DRIVE_SMOOTHING=0.15, DOME_SMOOTHING=0.10
- Calm safety: 1000 ms timeout avoids “nervous braking” on WiFi hiccups
- Battery monitor (optional): teaches voltage dividers and ADCs
- SSID/password convention: R2-BK01…R2-BK16 / droidBK01…droidBK16 (≥8 chars)

## Quick Start
1) Upload v13
- Open `esp32/BB-R2-WiFi/BB-R2-WiFi.ino` in Arduino IDE
- Set per-droid credentials:
  ```cpp
  const char* DROID_NAME    = "R2-BK01";
  const char* WIFI_PASSWORD = "droidBK01";  // ≥ 8 characters
  ```
- Install libraries: ESP32Servo, WebSockets (optional DFRobotDFPlayerMini)
- Upload to your ESP32 board

2) Connect and control
- Power the droid (4×AA batteries)
- Connect device to SSID (e.g., droidBK01), password same
- Open browser: http://192.168.4.1
- Drive with the joystick; rotate dome with the slider

## Pin Assignments (Workshop variant)
- GPIO 2  → Left drive servo
- GPIO 4  → Right drive servo
- GPIO 5  → Dome servo
- Optional battery monitor: ADC pin (A0) via voltage divider (2×10kΩ)
- Optional sound (DFPlayer): board-specific TX/RX pins (see controller README)

## Version (WiFi v13)
- writeMicroseconds() for dome for smooth motion
- SAFETY_TIMEOUT = 1000 ms
- DRIVE_SMOOTHING = 0.15, DOME_SMOOTHING = 0.10
- Non-blocking loop (removed delay)
- WebSocket telemetry (~10 Hz) and serial log relay
- Robust JSON parsing (handles compact “joy”:“drive”)

## Credits
- Workshop fork, curriculum, and tuning: Seán McArdle
- Original BB-R2 code and concept: Bjoern Giesler
- 3D models: Michael Baddeley
- Hosted by: The Bakken Museum (Minneapolis)

---

## Original Project Background (Bjoern Giesler)

The upstream BB-R2 STEM ESP32 project (by Bjoern Giesler) laid the foundation:
- Beginner-friendly droid using continuous rotation servos
- Bluetooth/Dabble control with DFPlayer audio
- 4×AA power and breadboard wiring
- Clean, well-commented code under Apache 2.0

This fork summarizes BG’s story here for context and moves the workshop-specific WiFi variant to the forefront. For BG’s full history, design rationale, and original Bluetooth-based implementation, please visit the upstream repository via the fork network and read his detailed README.

License: Apache 2.0 (compatible with the original project)

---

## Extension Ideas
- LEDs with non-blocking patterns (blink, breathe, pulse)
- Sound effects via DFPlayer Mini (MP3 files 001.mp3, 002.mp3…)
- Distance sensors (VL53L0X, HC-SR04) for autonomous mode
- Battery alternatives (LiPo with proper voltage regulation)
