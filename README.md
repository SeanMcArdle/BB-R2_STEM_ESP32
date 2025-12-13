# BB-R2_STEM_ESP32

This is the code base for the BB-R2 STEM project based on ESP32. 

"BB-R2" or Baby R2 is the name of a family of Star Wars inspired mini droids 3D modeled by Michael Baddeley for consumer 3D printers. These droids are intentionally very simple so they can be printed and built in very short time, and (in their basic form) can drive using two foot motors and turn their domes controlled by a standard RC radio control.

## History

The project you are looking at comes out of preparing a live droid building session to be performed during the R2 Builders Club's exhibit at the 2023 Stuttgart Comic Con. The idea was to spend an hour or a bit more with up to 10 inexperienced builders or small builder teams, walk them through specific aspects of droid building like how to choose the right hardware and materials, how to choose between static and moving droids, how to choose power supplies, remote controls, electronics etc., and doing all of that on an actual practical small droid for everyone to take home. Baby R2s are ideally suited for this, and after obtaining Michael Baddeley's permission, I built a prototype and wrote the code for it. That code is this project.

## Design requirements and choices

To be usable for a STEM session during a con, the project had to fulfill the following design goals:

* Be exciting! Now this is very subjective, but for me that meant at least a moving droid with sound.
* Be as affordable as possible (current BOM cost is <50€).
* Be absolutely beginner friendly and safe.
* Be easily extendable, so that participants can have fun with their droid even after the event.

From these requirements, it was clear that a regular RC controller / receiver combo would be out. These can be had very cheap now, but they still would cost a significant fraction of the BOM price, and while they can drive servos they cannot trigger sounds without additional circuitry. Rather, I opted for a droid that 

* Can be controlled with an Android or iOS smart phone via Bluetooth, which just about everyone already has. The app we use for this is Dabble.
* Uses the ESP32 microcontroller as a receiver and logic device, in this case specifically a Seeed Xiao ESP32C3.
* Is powered by 4 AA batteries (ideally rechargeable) instead of LiPos so no charger is requied.
* Provides sound via a DFPlayer MP3 player module and small speakers.
* Drives and moves the dome using continuous rotation servos (this is copied from the original project as described by Michael Baddeley).
* Has the electronics set up on a small breadboard using pin wires, using 3 pins for the servos and 2 pins to control the MP3 player.

## Bakken Museum Workshop Version (WiFi Control v13)

A production-ready WiFi-based control version has been developed specifically for youth STEM workshops at **The Bakken Museum** (Minneapolis, MN) - **December 30-31, 2025**. This version replaces Bluetooth/Dabble control with a self-hosted WiFi Access Point and web-based interface optimized for educational workshops.

### Key Features (v13)

* **WiFi Access Point Mode:** Each droid creates its own WiFi network (e.g., "droidBK01")
* **Enhanced Web UI:** Joystick control, dome slider, real-time telemetry display
* **WebSocket Telemetry:** Live data updates at 10Hz for immediate feedback
* **Serial Log Relay:** View debug messages directly in web interface
* **Smooth Motion Control:** Tuned smoothing (DRIVE=0.15, DOME=0.10) for natural movement
* **Safety Timeout:** 1000ms timeout prevents runaway on WiFi dropout
* **Precision Dome Control:** Uses writeMicroseconds() for smooth, accurate rotation
* **No Blocking Delays:** Responsive control loop for best performance
* **Sound Support:** DFPlayer Mini integration (optional)
* **LED Control:** Built-in LED with web control (expandable)
* **Workshop Ready:** 8+ character passwords, consistent naming convention

### Quick Start Guide

**Location:** All WiFi controller files are in the **`esp32/BB-R2-WiFi/`** directory.

1. **Upload the Code:**
   - Open `esp32/BB-R2-WiFi/BB-R2-WiFi.ino` in Arduino IDE
   - Customize the WiFi name and password (lines 38-39): 
     ```cpp
     const char* ssid = "droidBK01";     // droidBK01, droidBK02, etc.
     const char* password = "droidBK01"; // 8+ characters required
     ```
   - Install required libraries: ESP32Servo, WebSockets, DFRobotDFPlayerMini
   - Upload to your ESP32

2. **Connect to Your Droid:**
   - Power on the droid (4xAA batteries)
   - Wait 30 seconds for WiFi to start
   - On your phone/tablet, connect to WiFi network "droidBK01" (or your custom name)
   - Password: `droidBK01` (or your custom password, 8+ characters)
   - Open browser and go to: `http://192.168.4.1`

3. **Control Your Droid:**
   - Use joystick for intuitive drive control
   - Use dome slider for precise dome rotation
   - Watch real-time telemetry and serial log
   - Trigger sound effects (if DFPlayer installed)
   - Toggle LED on/off

### Pin Assignments

```
ESP32 Pin | Component
----------|----------------------------------
GPIO 2    | Left servo signal
GPIO 3    | Right servo signal
GPIO 4    | Dome servo signal
GPIO 5    | LED (optional)
GPIO 20*  | DFPlayer RX (optional, board-specific)
GPIO 21*  | DFPlayer TX (optional, board-specific)
```

**⚠️ Important:** DFPlayer pins are configured for Seeed Xiao ESP32C3 (GPIO 20/21). Other boards may require different pins:
- **ESP32 DevKit:** GPIO 16 (RX), GPIO 17 (TX)
- Check your board's pinout and update lines 60-61 in the code accordingly!

### Libraries Required

Install via Arduino Library Manager:
* **ESP32Servo** by Kevin Harrington
* **WebSockets** by Markus Sattler
* **DFRobotDFPlayerMini** by DFRobot (optional, for sound)

Built-in (no installation needed):
* WiFi.h (ESP32 built-in)
* WebServer.h (ESP32 built-in)

### Documentation

Comprehensive documentation is available in the `esp32/BB-R2-WiFi/` directory:

* **`esp32/BB-R2-WiFi/README.md`** - Complete guide including:
  - Detailed setup instructions
  - Bill of Materials (BOM)
  - Wiring diagrams and pin configurations
  - Tuning and customization guide
  - Extensive troubleshooting section
  - Workshop preparation tips for educators
  - Advanced modifications and extensions

* **`docs/WORKSHOP_NOTES.md`** - Full workshop curriculum:
  - Day-by-day workshop schedule
  - Preparation checklists
  - Teaching tips and common issues
  - Extension activities and take-home challenges

### Workshop Details

**Event:** BB-R2 Droid Building Workshop  
**Dates:** December 30-31, 2025  
**Location:** The Bakken Museum, Minneapolis, MN  
**Target Audience:** Kids ages 8-14 with parent/guardian assistance

This hands-on STEM workshop teaches:
- Basic electronics and wiring
- Microcontroller programming concepts
- Robotics and servo control
- WiFi networking basics
- Problem-solving and debugging

### Naming Convention for Workshop

For workshops with multiple droids, use this consistent naming scheme:
- **droidBK01** through **droidBK16** (or more as needed)
- All lowercase for consistency
- Same password as SSID for simplicity (must be 8+ characters)
- Easy for students to remember and type

### Version Information

**Current Version:** v13 (December 2025)

**v13 Improvements:**
- Uses `writeMicroseconds()` for all servos (especially dome) for smoother motion
- Increased safety timeout to 1000ms to prevent WiFi-induced stutter
- Tuned smoothing constants: `DRIVE_SMOOTHING=0.15`, `DOME_SMOOTHING=0.10`
- Removed `delay(10)` from loop - fully non-blocking operation
- Added WebSocket support for real-time telemetry broadcast
- Enhanced web UI with joystick and dome slider controls
- Added serial log relay to web interface
- Robust JSON parser handles formats with and without spaces
- Clear documentation of 8+ character password requirement

### Credits

**v13 WiFi Controller** developed for The Bakken Museum Droid Building Workshop (December 2025), based on the original BB-R2 ESP32 project by Bjoern Giesler (2023). Both versions are licensed under Apache 2.0.

**Contributors:**
- Bjoern Giesler - Original BB-R2 ESP32 project
- Michael Baddeley - BB-R2 3D models
- The Bakken Museum Education Team - Workshop design and requirements

---

## Extension ideas

While the droid is already quite cool, there are a number of things that it cannot do, and that would be very useful or fun. This is just an idea collection for would-be hackers and extenders.

* Battery readout. How to do this with a simple voltage divider is described in the Seeed Xiao ESP32C3 web pages. While Dabble's gamepad remote has no way of showing a battery readout, its Bluetooth terminal window can show debug prints generated by the droid.
* Use LiPos instead of AA batteries. This can dramatically extend the runtime of the droid. However LiPos come in 1 cell, giving 3.7V (which is too little to drive the servos), or 2 cell, giving 7.4V (which will roast the ESP32 board), so doing this will require a step down DC/DC converter, like for example a LM2596 circuit, or a BEC (Battery Equivalent Circuit), which is essentially the same thing used in RC models.
* Add sensing! There are many user friendly distance sensors on the market by now, that can be connected to the ESP32 via the I2C bus, so the droid can be upgraded to drive autonomously. This is where the seriously fun stuff begins.