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

## Bakken Museum Workshop Version (WiFi Control)

A new WiFi-based control version has been developed specifically for youth STEM workshops at The Bakken Museum (Minneapolis, MN). This version replaces Bluetooth/Dabble control with a self-hosted WiFi Access Point and web-based interface.

### Features

* **WiFi Access Point Mode:** Each droid creates its own WiFi network (e.g., "R2-BAKKEN-01")
* **Web-Based Control:** Control via any web browser (phone, tablet, or computer)
* **No App Required:** No need to install Dabble or pair Bluetooth
* **Mobile-Friendly Interface:** Responsive design with touch-friendly buttons
* **Clone Wars Theme:** Dark theme with cyan accents inspired by Star Wars animation
* **Sound Support:** DFPlayer Mini for sound effects (optional)
* **LED Control:** Built-in LED on/off control (expandable)
* **Educational Focus:** Code designed to be readable and teachable

### Getting Started with WiFi Version

1. **Upload the Code:**
   - Open `BB-R2-WiFi/BB-R2-WiFi.ino` in Arduino IDE
   - Customize the WiFi SSID in the code (default: "R2-BAKKEN-01")
   - Upload to your ESP32

2. **Connect to Your Droid:**
   - Power on the droid
   - On your phone/tablet, connect to WiFi network "R2-BAKKEN-XX"
   - Password: `droid123`
   - Open browser and go to: `http://192.168.4.1`

3. **Control Your Droid:**
   - Use the web interface buttons to move forward/back/left/right
   - Control dome rotation
   - Trigger sound effects (if DFPlayer installed)
   - Toggle LED on/off

### Pin Assignments (WiFi Version)

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

*Note: DFPlayer pins are configured for Seeed Xiao ESP32C3. Other boards may require different pins (e.g., GPIO 16/17 for DevKit). Check your board's pinout.

### Libraries Required (WiFi Version)

* WiFi.h (ESP32 built-in)
* WebServer.h (ESP32 built-in)
* ESP32Servo
* DFRobotDFPlayerMini (optional, for sound)

### Workshop Documentation

For educators planning to use this for workshops, see:
* `docs/WORKSHOP_NOTES.md` - Complete curriculum, BOM, and preparation checklist

### Credits

WiFi version developed for The Bakken Museum workshop (December 2025), based on the original BB-R2 ESP32 project by Bjoern Giesler. Both versions are licensed under Apache 2.0.

---

## Extension ideas

While the droid is already quite cool, there are a number of things that it cannot do, and that would be very useful or fun. This is just an idea collection for would-be hackers and extenders.

* Battery readout. How to do this with a simple voltage divider is described in the Seeed Xiao ESP32C3 web pages. While Dabble's gamepad remote has no way of showing a battery readout, its Bluetooth terminal window can show debug prints generated by the droid.
* Use LiPos instead of AA batteries. This can dramatically extend the runtime of the droid. However LiPos come in 1 cell, giving 3.7V (which is too little to drive the servos), or 2 cell, giving 7.4V (which will roast the ESP32 board), so doing this will require a step down DC/DC converter, like for example a LM2596 circuit, or a BEC (Battery Equivalent Circuit), which is essentially the same thing used in RC models.
* Add sensing! There are many user friendly distance sensors on the market by now, that can be connected to the ESP32 via the I2C bus, so the droid can be upgraded to drive autonomously. This is where the seriously fun stuff begins.