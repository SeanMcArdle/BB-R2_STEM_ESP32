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
- Upload to your ESP
