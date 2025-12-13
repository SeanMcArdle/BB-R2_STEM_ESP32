# BB-R2 WiFi Control

WiFi-based control system for BB-R2 droid, designed for The Bakken Museum workshops.

## Quick Start

1. **Configure Your Droid:**
   - Open `BB-R2-WiFi.ino` in Arduino IDE
   - Change SSID if needed (line 27): `const char* ssid = "R2-BAKKEN-01";`
   - Adjust pin assignments if using different ESP32 board

2. **Install Required Libraries:**
   - ESP32Servo (via Arduino Library Manager)
   - DFRobotDFPlayerMini (via Arduino Library Manager)
   - WiFi.h and WebServer.h are built-in for ESP32

3. **Upload to ESP32:**
   - Select your ESP32 board in Tools > Board
   - Select correct COM port
   - Click Upload

4. **Connect and Control:**
   - Power on your droid
   - Connect to WiFi network (default: "R2-BAKKEN-01")
   - Password: `droid123`
   - Open browser: `http://192.168.4.1`
   - Use web interface to control your droid!

## Pin Configuration

Default pin assignments (adjust in code if needed):

```
GPIO 2  → Left Servo
GPIO 3  → Right Servo
GPIO 4  → Dome Servo
GPIO 5  → LED (optional)
GPIO 20 → DFPlayer RX (optional, board-specific)*
GPIO 21 → DFPlayer TX (optional, board-specific)*
```

**Important Note on DFPlayer Pins:**
The RX/TX pin numbers (GPIO 20/21) are configured for Seeed Xiao ESP32C3. 
Other ESP32 boards may use different pins for hardware serial. Common alternatives:
- ESP32 DevKit: GPIO 16 (RX), GPIO 17 (TX)
- Check your specific board's pinout diagram before wiring!

## Customization

### Change WiFi Network Name:
```cpp
const char* ssid = "R2-BAKKEN-02";  // Line 27
```

### Change Password:
```cpp
const char* password = "mypassword";  // Line 28
```

### Adjust Movement Speed:
Modify goal values in `processCommand()` function (lines 295-348)

### Adjust Servo Limits:
Modify min/max values in `servoStates` array (lines 61-65)

## Troubleshooting

**Can't connect to WiFi:**
- Make sure droid is powered on
- Wait 30 seconds after power-on for WiFi to start
- Check SSID and password are correct
- Try forgetting the network and reconnecting

**Servos not moving:**
- Check servo connections to GPIO pins
- Verify external power supply (4xAA batteries) is connected
- Check battery charge level (needs 4.8V+)

**No sound:**
- Verify DFPlayer Mini is connected
- Check SD card is inserted and formatted as FAT32
- Ensure MP3 files are in root directory, named 001.mp3, 002.mp3, etc.
- Check speaker connections

**Web page won't load:**
- Verify you're connected to the droid's WiFi
- Try `192.168.4.1` directly in browser
- Clear browser cache
- Try a different browser

## Features

- ✅ WiFi Access Point (no router needed)
- ✅ Mobile-friendly web interface
- ✅ Smooth servo movement (preserved from original)
- ✅ Tank-style steering (forward, back, left, right)
- ✅ Dome rotation control
- ✅ Sound effects (DFPlayer Mini)
- ✅ LED control
- ✅ Clone Wars/Rebels visual theme

## Credits

Based on the original BB-R2 ESP32 project by Bjoern Giesler (2023)
Modified for The Bakken Museum workshops
Licensed under Apache 2.0

## Learn More

See `../docs/WORKSHOP_NOTES.md` for complete workshop documentation.
