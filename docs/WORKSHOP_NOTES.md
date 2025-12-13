# BB-R2 Workshop Notes - The Bakken Museum

## Workshop Overview

**Event:** BB-R2 Droid Building Workshop  
**Dates:** December 30-31, 2025  
**Location:** The Bakken Museum, Minneapolis, MN  
**Duration:** 2-day format  
**Target Audience:** Kids ages 8-14 (with parent/guardian assistance)

## Workshop Goals

This hands-on STEM workshop introduces participants to:
- Basic electronics and wiring
- Microcontroller programming concepts
- Robotics and servo control
- WiFi networking basics
- 3D printing applications
- Problem-solving and debugging

## Day 1: Assembly & Basic Control

### Session 1: Introduction (30 minutes)
- Welcome and safety briefing
- Overview of BB-R2 droid project
- Introduction to components
- Star Wars inspiration and droid history

### Session 2: Mechanical Assembly (1 hour)
- Identify 3D printed parts
- Assemble body and dome
- Install servo motors
- Mount battery pack
- Basic wiring safety

### Session 3: Electronics & Wiring (1 hour)
- Introduction to breadboards
- ESP32 microcontroller overview
- Connect servos to breadboard
- Wire power from battery pack
- Optional: Add DFPlayer for sound
- Optional: Add LED for lights

### Session 4: First Power-Up (30 minutes)
- Safety check of all connections
- Power on the droid
- Connect to WiFi network
- Test basic movements
- Troubleshooting common issues

## Day 2: Programming & Customization

### Session 1: Understanding the Code (45 minutes)
- What is Arduino code?
- How WiFi Access Points work
- Understanding servo control
- The web interface

### Session 2: Customization (1 hour)
- Change WiFi network name
- Modify movement speeds
- Add custom sounds (if DFPlayer installed)
- LED patterns (if LED installed)
- Dome movement fine-tuning

### Session 3: Advanced Features (45 minutes)
- Magnetic dome modification (demonstration)
- Battery life tips
- Future expansion ideas
- Taking it further at home

### Session 4: Showcase & Take Home (30 minutes)
- Droid parade
- Troubleshooting clinic
- Q&A with instructors
- Resources for continued learning

## Hardware Bill of Materials (BOM)

### Per Droid Kit:

**Core Components:**
- 1x Seeed Xiao ESP32C3 (or compatible ESP32 board) - ~$5-7
- 3x Continuous rotation servos (e.g., FS90R) - ~$15-18
- 1x 4xAA battery holder with switch - ~$2-3
- 4x AA batteries (rechargeable recommended) - ~$6-8
- 1x 170-point mini breadboard - ~$2-3
- 10x Male-to-male jumper wires - ~$2-3
- 10x Male-to-female jumper wires - ~$2-3

**Optional Sound System:**
- 1x DFPlayer Mini MP3 player module - ~$3-4
- 1x Micro SD card (1GB or larger) - ~$3-5
- 1x Small speaker (8Ω, 0.5W) - ~$2-3

**Optional Lights:**
- 1x LED (any color) - ~$0.50
- 1x 220Ω resistor - ~$0.10

**3D Printed Parts:**
- Body (main shell)
- Dome (head piece)
- Foot plates (2x)
- Servo mounts (3x)
- Battery holder mount

**Assembly Hardware:**
- 6x M3 screws (various lengths)
- Small zip ties for cable management

**Total Cost Per Kit:** ~$40-60 (depending on options)

**Tools Needed (shared):**
- Small Phillips screwdriver
- Wire strippers
- Scissors
- Hot glue gun (optional, for securing components)

## Pin Assignments

### Standard Configuration:

```
ESP32 Pin | Component
----------|----------------------------------
GPIO 2    | Left servo signal
GPIO 3    | Right servo signal  
GPIO 4    | Dome servo signal
GPIO 5    | LED (optional)
GPIO 20*  | DFPlayer RX (optional, board-specific)
GPIO 21*  | DFPlayer TX (optional, board-specific)
GND       | Common ground (servos, battery -)
5V        | Servo power (from battery +)
```

### Notes:
- Always connect grounds together
- Servos need external power (4xAA = ~6V)
- ESP32 runs on 3.3V or 5V depending on model
- Use breadboard to distribute power/ground
- **DFPlayer pin numbers are board-specific** - verify against your ESP32 variant's pinout
  - Seeed Xiao ESP32C3: GPIO 20 (RX), GPIO 21 (TX) recommended (as configured in code)
  - ESP32 DevKit: GPIO 16 (RX), GPIO 17 (TX) recommended
  - Always check your board's documentation and update the pin definitions in the code!

## WiFi Network Details

Each droid creates its own WiFi Access Point:

- **Default SSID:** R2-BAKKEN-01 (increment for each droid)
- **Password:** droid123
- **IP Address:** 192.168.4.1
- **Web Interface:** http://192.168.4.1

**Connecting:**
1. Turn on the droid
2. Open WiFi settings on phone/tablet
3. Find network "R2-BAKKEN-xx"
4. Enter password: droid123
5. Open web browser
6. Navigate to 192.168.4.1

## Magnetic Dome Modification

### Overview
The magnetic dome modification allows tool-free dome removal for easy access to internal components. This is planned as a future enhancement.

### Components Needed:
- Small neodymium magnets (6mm diameter, 2mm thick)
- 4-6 magnets per droid
- Hot glue or epoxy
- Magnetic tape (alternative option)

### Installation:
1. Identify dome attachment points
2. Glue magnets to body (alternating polarity)
3. Glue corresponding magnets to dome
4. Test alignment and holding strength
5. Ensure dome still rotates freely

### Benefits:
- Easy access for maintenance
- Quick battery changes
- No screws to lose
- Better for young builders

### Safety Notes:
- Keep magnets away from electronics during installation
- Supervise children with small magnets
- Consider magnetic tape for younger age groups

## Preparation Checklist

### 2 Weeks Before:

- [ ] Order all electronic components
- [ ] Print 3D parts for all participants
- [ ] Test print quality and fit
- [ ] Prepare sound files for DFPlayer
- [ ] Test one complete assembly
- [ ] Create emergency spare kits (2-3 extra of everything)

### 1 Week Before:

- [ ] Assemble instructor demonstration unit
- [ ] Pre-load DFPlayer SD cards with sounds
- [ ] Organize components into individual kits
- [ ] Label all kits clearly
- [ ] Prepare troubleshooting guide
- [ ] Test WiFi in workshop space
- [ ] Print instruction handouts

### Day Before:

- [ ] Set up workshop tables
- [ ] Distribute tools at stations
- [ ] Test power outlets
- [ ] Charge all rechargeable batteries
- [ ] Set up projection screen for demonstrations
- [ ] Prepare sign-in sheet
- [ ] Safety equipment ready (first aid, fire extinguisher)
- [ ] Set up take-home bags/boxes

### Day Of:

- [ ] Arrive 1 hour early
- [ ] Test demonstration droid
- [ ] Set up welcome table
- [ ] Distribute name tags
- [ ] Brief all volunteers/assistants
- [ ] Final safety check

## Teaching Tips

### For Instructors:

1. **Start with the End in Mind:** Show a working droid first
2. **Safety First:** Emphasize safe handling of batteries and tools
3. **Pair Programming:** Encourage kids to work in pairs
4. **Celebrate Small Wins:** Test each component as it's added
5. **Embrace Mistakes:** Debugging is part of learning
6. **Go Slow:** Better to finish with understanding than rush
7. **Have Spares:** Things will break, be prepared
8. **Document Everything:** Take photos for participants

### Common Issues & Solutions:

**Droid won't move:**
- Check battery connection
- Verify servo wiring
- Check battery charge level
- Ensure WiFi is connected

**Can't connect to WiFi:**
- Verify droid is powered on
- Check SSID spelling
- Wait 30 seconds after power-on
- Try forgetting network and reconnecting

**Servos jittery or weak:**
- Check battery voltage (needs 4.8V+)
- Tighten servo connections
- Reduce load on servos
- Replace batteries

**No sound:**
- Check DFPlayer connections
- Verify SD card is inserted
- Check sound files are MP3 format
- Test speaker connections

## Extension Activities

### For Advanced Participants:

1. **Autonomous Mode:** Add distance sensors for obstacle avoidance
2. **Pattern Programming:** Create dome choreography
3. **Sound Effects:** Record custom sound effects
4. **LED Patterns:** Program blinking/fading effects
5. **Range Extension:** Add WiFi range extender
6. **Battery Monitor:** Add voltage sensing
7. **Speed Control:** Add variable speed sliders to web interface

### Take-Home Challenges:

- Program a dance routine
- Add multiple LEDs for "light show"
- Create a phone app instead of web interface
- Design custom 3D printed accessories
- Build an obstacle course
- Team challenge: synchronized droids

## Resources for Continued Learning

### Online:
- Arduino Project Hub: arduino.cc/projecthub
- ESP32 Documentation: docs.espressif.com
- Adafruit Learning System: learn.adafruit.com
- The Bakken Museum online resources: thebakken.org

### Books:
- "Arduino Workshop" by John Boxall
- "Make: Electronics" by Charles Platt
- "Getting Started with ESP32" by various authors

### Communities:
- R2 Builders Club: astromech.net
- Arduino Forum: forum.arduino.cc
- Adafruit Discord: adafru.it/discord

## Safety Guidelines

### Electrical Safety:
- Never connect power backwards
- Don't mix batteries (all same type/brand)
- Remove batteries when storing
- Keep liquids away from electronics

### Physical Safety:
- Use tools as intended
- Keep small parts away from young children
- Supervise use of hot glue guns
- Wear safety glasses when cutting wires

### Workshop Rules:
- Stay at your station
- Ask for help when stuck
- Clean up spills immediately
- Report any injuries to instructor

## Credits & Acknowledgments

**Original Project:**
- Bjoern Giesler - Original BB-R2 ESP32 code (Apache 2.0 License)
- Michael Baddeley - BB-R2 3D models

**Workshop Development:**
- The Bakken Museum Education Team
- R2 Builders Club Minnesota Chapter
- Workshop volunteers and supporters

**Special Thanks:**
- All the young makers who inspire us
- Parents and guardians for their support
- Open source community

## License

This workshop documentation is provided under Creative Commons BY-SA 4.0

The BB-R2 WiFi code is licensed under Apache 2.0, maintaining compatibility with the original project.

3D models used with permission from Michael Baddeley.

---

*For questions or feedback about this workshop, contact The Bakken Museum education department.*

**May the Force be with you, young makers!** 🤖✨
