# BB-R2 WiFi Controller v13

Final, production-ready WiFi control system for BB-R2 droid. Designed specifically for The Bakken Museum Droid Building Workshop (December 30-31, 2025).

## Features

- ✅ **WiFi Access Point Mode** - Each droid creates its own network, no router needed
- ✅ **Enhanced Web UI** - Joystick control, dome slider, real-time telemetry
- ✅ **WebSocket Telemetry** - Live data updates at 10Hz
- ✅ **Serial Log Relay** - View debug messages in web interface
- ✅ **Smooth Motion** - Tuned smoothing for natural droid movement
- ✅ **Safety Timeout** - 1000ms timeout prevents runaway on WiFi dropout
- ✅ **Dome Precision** - Uses writeMicroseconds() for smooth dome rotation
- ✅ **Sound Support** - DFPlayer Mini integration (optional)
- ✅ **LED Control** - Built-in LED on/off (expandable)
- ✅ **No Blocking Delays** - Responsive control loop
- ✅ **Workshop Ready** - 8+ character WiFi passwords, clear naming convention

## Quick Start

### 1. Configure Your Droid

Open `BB-R2-WiFi.ino` in Arduino IDE and customize these settings:

```cpp
// Line 38-39: CHANGE FOR EACH DROID
const char* ssid = "droidBK01";        // droidBK01, droidBK02, droidBK03...
const char* password = "droidBK01";    // Must be 8+ characters
```

**Workshop Naming Convention:**
- Use format: `droidBK01`, `droidBK02`, ... `droidBK16`
- All lowercase, consistent pattern
- Password same as SSID for simplicity
- 8+ characters required by WiFi standard

### 2. Install Required Libraries

In Arduino IDE, go to **Tools → Manage Libraries** and install:

- **ESP32Servo** by Kevin Harrington
- **WebSockets** by Markus Sattler
- **DFRobotDFPlayerMini** by DFRobot (optional, for sound)

Built-in libraries (no installation needed):
- WiFi.h
- WebServer.h

### 3. Upload to ESP32

1. Connect ESP32 to computer via USB
2. Select your board: **Tools → Board → ESP32 Arduino → [Your ESP32 model]**
   - Seeed Xiao ESP32C3: **XIAO_ESP32C3**
   - Generic ESP32: **ESP32 Dev Module**
3. Select correct COM port: **Tools → Port → [Your port]**
4. Click **Upload** button (arrow icon)
5. Wait for "Done uploading" message

### 4. Connect and Control

1. **Power on** your droid (4xAA batteries)
2. **Wait 30 seconds** for WiFi to start
3. On your phone/tablet:
   - Open **WiFi Settings**
   - Connect to network: **droidBK01** (or your custom name)
   - Enter password: **droidBK01**
4. Open web browser
5. Navigate to: **http://192.168.4.1**
6. Use joystick and controls to drive your droid!

## Bill of Materials (BOM)

### Core Electronics (~$30-35)

| Component | Quantity | Approx. Cost | Notes |
|-----------|----------|--------------|-------|
| ESP32 board (Seeed Xiao ESP32C3 or DevKit) | 1 | $5-7 | Microcontroller |
| Continuous rotation servos (FS90R or similar) | 3 | $15-18 | 2 drive + 1 dome |
| 4xAA battery holder with switch | 1 | $2-3 | Power supply |
| AA batteries (rechargeable recommended) | 4 | $6-8 | NiMH 1.2V work great |
| 170-point mini breadboard | 1 | $2-3 | Component mounting |
| Male-to-male jumper wires | 10+ | $2-3 | Servo connections |

### Optional Sound System (~$10-12)

| Component | Quantity | Approx. Cost | Notes |
|-----------|----------|--------------|-------|
| DFPlayer Mini MP3 module | 1 | $3-4 | Sound playback |
| Micro SD card (1GB+) | 1 | $3-5 | FAT32 formatted |
| Small speaker (8Ω, 0.5W) | 1 | $2-3 | Audio output |

### Optional Extras (~$1-2)

| Component | Quantity | Approx. Cost | Notes |
|-----------|----------|--------------|-------|
| LED (any color) | 1 | $0.50 | Visual indicator |
| 220Ω resistor | 1 | $0.10 | LED current limiting |

### 3D Printed Parts (Free with printer access)

- Body shell
- Dome (head)
- 2x Foot plates
- 3x Servo mounts
- Battery holder mount

**Total Cost:** $40-50 per droid (depending on options)

## Wiring Guide

### Standard Pin Configuration

```
ESP32 GPIO | Component
-----------|----------------------------------
GPIO 2     | Left servo signal (yellow wire)
GPIO 3     | Right servo signal (yellow wire)
GPIO 4     | Dome servo signal (yellow wire)
GPIO 5     | LED positive (with 220Ω resistor)
GPIO 20*   | DFPlayer RX (optional, board-specific)
GPIO 21*   | DFPlayer TX (optional, board-specific)
GND        | All grounds (servo black, battery -, LED -)
5V or VIN  | Servo power (red wires from battery +)
```

**⚠️ IMPORTANT: DFPlayer pins are board-specific!**

| ESP32 Board | RX Pin | TX Pin | Update Code Lines |
|-------------|--------|--------|-------------------|
| Seeed Xiao ESP32C3 | GPIO 20 | GPIO 21 | Default (no change needed) |
| ESP32 DevKit | GPIO 16 | GPIO 17 | Change lines 60-61 |
| ESP32-S2 | GPIO 18 | GPIO 17 | Change lines 60-61 |
| Other boards | Check pinout | Check pinout | Change lines 60-61 |

**Always verify your board's hardware serial pins before connecting DFPlayer!**

### Breadboard Layout

```
     ESP32
     ┌────┐
     │    │ GPIO 2 ──→ Left Servo (yellow)
     │    │ GPIO 3 ──→ Right Servo (yellow)
     │    │ GPIO 4 ──→ Dome Servo (yellow)
     │    │ GPIO 5 ──→ LED + (through resistor)
     │    │ GND ─────→ Common Ground Rail
     │    │ 5V ──────→ Servo Power Rail
     └────┘

Battery (+) ──→ Servo Power Rail (red wires)
Battery (-) ──→ Common Ground Rail (black wires)
```

### Servo Connections

Each servo has 3 wires:
- **Brown/Black** = Ground → GND rail on breadboard
- **Red** = Power → 5V rail on breadboard (from battery +)
- **Yellow/Orange** = Signal → GPIO pin on ESP32

### DFPlayer Wiring (Optional)

```
DFPlayer Pin | Connect To
-------------|----------------------------------
VCC          | 5V (battery +)
GND          | GND (common ground)
TX           | ESP32 RX (GPIO 20 on Xiao C3)*
RX           | ESP32 TX (GPIO 21 on Xiao C3)*
SPK1         | Speaker wire 1
SPK2         | Speaker wire 2
```

*Note: TX on DFPlayer connects to RX on ESP32, and vice versa!

### LED Wiring (Optional)

```
GPIO 5 ──→ [220Ω Resistor] ──→ LED (+) ──→ LED (-) ──→ GND
```

## Setup Instructions

### Arduino IDE Configuration

1. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to **File → Preferences**
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to **Tools → Board → Boards Manager**
   - Search for "ESP32" and install "ESP32 by Espressif Systems"

2. **Select Your Board**
   - **Tools → Board → ESP32 Arduino → [Your board]**
   - For Seeed Xiao ESP32C3: Select **XIAO_ESP32C3**
   - For generic ESP32: Select **ESP32 Dev Module**

3. **Configure Upload Settings**
   - **Tools → Port** → Select your COM port
   - **Tools → Upload Speed** → 921600 (or 115200 if having issues)
   - **Tools → CPU Frequency** → 160MHz
   - **Tools → Flash Mode** → QIO (if available)

### DFPlayer Sound Setup (Optional)

1. **Format SD Card**
   - Use FAT32 file system
   - Maximum 32GB card size
   - Some cards work better than others (SanDisk recommended)

2. **Add Sound Files**
   - Place MP3 files in root directory (not in folders)
   - Name files: `001.mp3`, `002.mp3`, `003.mp3`, etc.
   - Files must be numbered sequentially starting from 001
   - Supports MP3 format, 8-48kHz sample rate

3. **Test DFPlayer**
   - Insert SD card before powering on
   - Watch Serial Monitor for "DFPlayer initialized" message
   - If initialization fails, check wiring and SD card format

### WiFi Password Requirements

WiFi passwords **must be 8+ characters** (WPA2 standard):

✅ **Good passwords:**
- `droidBK01` (10 characters)
- `droidBK02` (10 characters)
- `starwars1` (9 characters)

❌ **Bad passwords:**
- `droid01` (7 characters - too short!)
- `r2d2` (4 characters - too short!)
- `BB-R2` (5 characters - too short!)

## Tuning & Customization

### Fine-Tuning Motion

The v13 controller includes tuning constants at the top of the code (lines 45-47):

```cpp
#define DRIVE_SMOOTHING 0.15   // Drive motor responsiveness (0.0-1.0)
#define DOME_SMOOTHING 0.10    // Dome motor responsiveness (0.0-1.0)
#define SAFETY_TIMEOUT_MS 1000 // Stop motors after 1000ms of no commands
```

**Adjusting Smoothing:**
- **Higher values (0.2-0.5)** = Faster response, more jerky
- **Lower values (0.05-0.15)** = Slower response, smoother motion
- **Default values** = Tuned for realistic droid behavior

**Safety Timeout:**
- Prevents runaway if WiFi connection drops
- 1000ms (1 second) is optimal for WiFi control
- Increase if experiencing frequent stops
- Decrease for more responsive safety cutoff

### Modifying Movement Speed

To change maximum speed, edit the processCommand function (around line 471):

```cpp
// Current settings (0.2 = slow, 0.8 = fast)
servoStates[LEFT].goal = 0.8;   // Make smaller for slower (e.g., 0.6)
servoStates[RIGHT].goal = 0.8;  // Make larger for faster (e.g., 0.9)
```

### Adjusting Servo Limits

If your servos don't have the same range, adjust these values (lines 75-77):

```cpp
#define SERVO_MIN_US 1000   // Minimum microseconds
#define SERVO_MAX_US 2000   // Maximum microseconds
#define SERVO_CENTER_US 1500 // Center (stopped)
```

### Changing Telemetry Rate

To change telemetry broadcast frequency (line 128):

```cpp
const unsigned long TELEMETRY_INTERVAL_MS = 100; // 10Hz (every 100ms)
// Change to 50 for 20Hz, 200 for 5Hz, etc.
```

## Web Interface Usage

### Joystick Control

- **Drag the stick** to control drive motors
- **Up** = Forward
- **Down** = Backward
- **Left** = Turn left
- **Right** = Turn right
- **Diagonal** = Forward/back while turning
- **Release** = Stop automatically

### Dome Slider

- **Drag slider left** = Dome rotates left
- **Center position** = Dome stops
- **Drag slider right** = Dome rotates right
- Updates in real-time as you move the slider

### Sound Buttons

- **Sound 1/2/3** = Play specific sound file (001.mp3, 002.mp3, 003.mp3)
- **Next** = Play next sound in sequence
- Requires DFPlayer Mini and SD card

### LED Control

- **LED On** = Turn on built-in LED
- **LED Off** = Turn off built-in LED

### Telemetry Display

Shows real-time status:
- **Left/Right/Dome** = Current servo positions (0-100)
- **Safety** = OK or ACTIVE (if timeout triggered)

### Serial Log

Displays debug messages from the droid:
- Connection status
- Command received
- Error messages
- Automatically scrolls to show newest messages

## Troubleshooting

### Can't Upload Code

**Problem:** Arduino IDE shows upload errors

**Solutions:**
1. Check correct board selected in Tools → Board
2. Check correct port selected in Tools → Port
3. Try pressing BOOT button while uploading (some boards)
4. Try lower upload speed (115200 instead of 921600)
5. Try a different USB cable (some are power-only)
6. Make sure no Serial Monitor is open

### Can't Find WiFi Network

**Problem:** Droid's WiFi network doesn't appear

**Solutions:**
1. Wait 30-60 seconds after power-on for WiFi to start
2. Check batteries have enough charge (need 4.8V+)
3. Open Serial Monitor (115200 baud) to see status messages
4. Verify SSID is correct in code (line 38)
5. Ensure password is 8+ characters (line 39)
6. Try power cycling (turn off, wait 5 seconds, turn on)

### Can't Connect to WiFi

**Problem:** Network appears but won't connect

**Solutions:**
1. Verify password is typed correctly (case-sensitive)
2. "Forget" the network on your device and reconnect
3. Ensure password is 8+ characters
4. Try connecting with a different device
5. Check Serial Monitor for error messages
6. Restart both droid and connecting device

### Web Page Won't Load

**Problem:** Connected to WiFi but page doesn't load

**Solutions:**
1. Type exact URL: `http://192.168.4.1` (not https://)
2. Disable mobile data (force phone to use WiFi)
3. Try a different browser (Chrome usually works best)
4. Clear browser cache
5. Wait 10 seconds and try again
6. Check Serial Monitor - server may not have started

### Servos Not Moving

**Problem:** Joystick/controls don't move the droid

**Solutions:**
1. Check battery voltage (need 4.8V minimum, 6V fresh)
2. Verify servo wires connected to correct GPIO pins
3. Check servo power wires connected to battery + (red wires)
4. Verify all grounds connected together (ESP32, servos, battery)
5. Test one servo at a time to isolate problem
6. Check Serial Monitor for safety timeout messages
7. Try moving joystick more (small movements may not trigger)

### Servos Jittery or Weak

**Problem:** Servos move but stutter or lack power

**Solutions:**
1. Replace batteries (need 4.8V+ for full power)
2. Check battery holder connections (clean contacts)
3. Verify all ground wires connected properly
4. Ensure servo power wires go directly to battery + (not through breadboard)
5. Try lower smoothing values in code (more gradual movement)
6. Check for loose connections on breadboard
7. Reduce load on servos (make sure droid moves freely)

### No Sound

**Problem:** DFPlayer not working

**Solutions:**
1. Check Serial Monitor for "DFPlayer init failed" message
2. Verify SD card is inserted and formatted as FAT32
3. Ensure sound files named correctly (001.mp3, 002.mp3, etc.)
4. Check MP3 file format (some encodings don't work)
5. Verify TX/RX wiring (TX → RX, RX → TX)
6. Check DFPlayer pin numbers correct for your board (lines 60-61)
7. Test with fresh SD card (some brands work better)
8. Verify speaker connections (SPK1 and SPK2)

### WiFi Drops or Stutters

**Problem:** Connection unstable, droid stops randomly

**Solutions:**
1. Move closer to droid (WiFi range ~30 feet)
2. Remove obstacles between device and droid
3. Safety timeout working as designed (send commands continuously)
4. Check for WiFi interference (other networks, microwaves, etc.)
5. Increase safety timeout if needed (line 47): `SAFETY_TIMEOUT_MS 2000`
6. Ensure antenna is properly connected (if external antenna)

### Dome Doesn't Move Smoothly

**Problem:** Dome rotation is jerky or jumpy

**Solutions:**
1. v13 uses writeMicroseconds() for smoothest motion
2. Verify dome servo connected to GPIO 4
3. Lower DOME_SMOOTHING value (line 46): try `0.05` or `0.08`
4. Check dome rotates freely (remove and test by hand)
5. Ensure servo isn't stalled against mechanical stop
6. Try different slider positions (some servos have dead zones)

### Battery Drains Too Fast

**Problem:** Batteries don't last long enough

**Solutions:**
1. Use rechargeable NiMH batteries (higher capacity)
2. Turn off droid when not in use (use switch on battery holder)
3. Remove batteries if storing for extended time
4. Check for shorts or incorrect wiring (creates heat)
5. Verify servos not stalling (creates heat and draws extra current)
6. Consider larger capacity batteries (2500mAh vs 1500mAh)

### Serial Log Not Showing

**Problem:** Web interface serial log is empty

**Solutions:**
1. Verify WebSocket connection (status should show "Connected")
2. Check browser console for JavaScript errors (F12)
3. Ensure port 81 not blocked (some networks block WebSocket)
4. Try refreshing the page
5. Check Serial Monitor in Arduino IDE to verify messages being sent

## Advanced Modifications

### Adding More Sound Effects

1. Add more MP3 files to SD card (004.mp3, 005.mp3, etc.)
2. Add buttons to web interface HTML (around line 265):
   ```html
   <button onclick="sendCmd('SND4')">🔊 Sound 4</button>
   ```
3. Add command handler in processCommand (around line 487):
   ```cpp
   else if (cmd == "SND4") {
     dfp.play(4);
   }
   ```

### Adding More LEDs

1. Connect additional LEDs to available GPIO pins
2. Define new pins (around line 56):
   ```cpp
   #define LED2_PIN 6
   #define LED3_PIN 7
   ```
3. Initialize in setup():
   ```cpp
   pinMode(LED2_PIN, OUTPUT);
   pinMode(LED3_PIN, OUTPUT);
   ```
4. Add control commands and HTML buttons

### Variable Speed Control

Replace fixed speed values with slider input:

1. Add speed slider to HTML (around line 220)
2. Create new command: `speed:0.5` (0.0-1.0)
3. Scale movement commands by speed factor

### Autonomous Mode

Add distance sensor and obstacle avoidance:

1. Connect VL53L0X or HC-SR04 sensor
2. Read distance in loop()
3. Stop or turn when obstacle detected
4. Add "Auto" button to web interface

## Workshop Notes for Educators

### Pre-Workshop Setup (per droid)

1. ✅ 3D print all parts and verify fit
2. ✅ Upload code with unique SSID (droidBK01, droidBK02, etc.)
3. ✅ Test WiFi connection and web interface
4. ✅ Verify all servos working properly
5. ✅ Label each kit with droid number
6. ✅ Charge all batteries fully
7. ✅ Prepare spare components (servos, ESP32, batteries)

### Workshop Day Tips

- **Arrive early** to test all equipment
- **Have 2-3 spare complete kits** for failures
- **Label GPIO pins** on each ESP32 with tape
- **Pre-strip wire ends** for younger students
- **Test each droid** before student takes home
- **Take photos** of correct wiring for reference
- **Keep master checklist** of working vs. problem droids

### Common Student Mistakes

1. **Reversed servo wiring** (red and brown swapped)
2. **Forgot to connect grounds** (servos to ESP32)
3. **Wrong GPIO pins** (followed different tutorial)
4. **Forgot to charge batteries**
5. **Typed wrong WiFi password** (case-sensitive)
6. **Used http**s**://192.168.4.1** (should be http://)

### Time Estimates

- **Mechanical assembly:** 30-45 minutes
- **Electronic wiring:** 30-45 minutes
- **Code upload and testing:** 15-30 minutes
- **Total workshop time:** 2-3 hours (with instruction)

### Age Appropriateness

- **Ages 8-10:** Need more assistance, focus on assembly
- **Ages 11-13:** Can handle most wiring with guidance
- **Ages 14+:** Can work more independently, ready for customization

## Technical Specifications

### ESP32 Requirements

- **Flash:** 4MB minimum (code uses ~1MB)
- **RAM:** 320KB SRAM (uses ~50KB)
- **WiFi:** 2.4GHz 802.11 b/g/n
- **GPIO:** Minimum 5 pins (3 servo + LED + optional sensor)
- **Serial:** 1 hardware serial port for DFPlayer (optional)

### Power Requirements

- **ESP32:** 3.3V, ~80-160mA active
- **Servos:** 4.8-6V, ~200mA each idle, 500mA+ stalled
- **DFPlayer:** 3.3-5V, ~20mA idle, 200mA playing
- **Total:** 4xAA (4.8-6V), 1-2A peak current

### Performance

- **Telemetry rate:** 10Hz (100ms updates)
- **Control latency:** ~50-100ms (WiFi + processing)
- **Safety timeout:** 1000ms (1 second)
- **Battery life:** 1-2 hours continuous use (NiMH AA)
- **WiFi range:** ~30 feet (10m) line of sight

### Servo Specifications

**Recommended:** Feetech FS90R or similar continuous rotation servos

- **Voltage:** 4.8-6V
- **Speed:** ~100 RPM at 4.8V, ~130 RPM at 6V
- **Torque:** ~1.3 kg-cm at 4.8V
- **Pulse width:** 1000-2000µs (1500µs = stop)
- **Size:** Standard micro servo (23x12x29mm)

## Version History

### v13 (Current - December 2025)

- ✅ Uses writeMicroseconds() for all servos (especially dome)
- ✅ Increased safety timeout to 1000ms
- ✅ Tuned smoothing: DRIVE_SMOOTHING=0.15, DOME_SMOOTHING=0.10
- ✅ Removed delay(10) from loop for non-blocking operation
- ✅ Added WebSocket support for real-time telemetry
- ✅ Added serial log relay to web interface
- ✅ Enhanced web UI with joystick control
- ✅ Added dome slider for precise control
- ✅ Robust JSON parser (handles no-space JSON)
- ✅ 8+ character password requirement documented
- ✅ Clear workshop naming convention (droidBK01-16)

### Earlier Versions

See git history for previous iterations and development notes.

## Credits & Acknowledgments

**Original Project:**
- **Bjoern Giesler** - Original BB-R2 ESP32 code (Apache 2.0 License)
- **Michael Baddeley** - BB-R2 3D models

**v13 Development:**
- **The Bakken Museum Education Team** - Workshop design and requirements
- **R2 Builders Club** - Technical consultation and testing

**Open Source Libraries:**
- **ESP32 Arduino Core** - Espressif Systems
- **ESP32Servo** - Kevin Harrington
- **WebSockets** - Markus Sattler
- **DFRobotDFPlayerMini** - DFRobot

## License

This code is licensed under **Apache License 2.0**, maintaining compatibility with the original BB-R2 ESP32 project.

```
Copyright 2023-2025 Bjoern Giesler and contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## Support & Resources

### Documentation

- Full workshop documentation: `/docs/WORKSHOP_NOTES.md`
- Repository: https://github.com/SeanMcArdle/BB-R2_STEM_ESP32

### Community

- **R2 Builders Club:** https://astromech.net
- **The Bakken Museum:** https://thebakken.org
- **Arduino Forum:** https://forum.arduino.cc

### Additional Learning

- **ESP32 Documentation:** https://docs.espressif.com
- **Arduino Project Hub:** https://create.arduino.cc/projecthub
- **Adafruit Learn:** https://learn.adafruit.com

---

**May the Force be with you, young makers!** 🤖✨

*For questions about this workshop version, contact The Bakken Museum education department.*
