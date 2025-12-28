/*
 * BB-R2 WiFi Control - Bakken Museum Workshop Version
 * 
 * CREDITS:
 * - Droid Design: Michael Baddeley (Mr Baddeley)
 *   patreon.com/mrbaddeley | youtube.com/@MrBaddeley
 *   Creator of the BB-R2 "Baby R2" 3D printable droid family
 * 
 * - Original ESP32 Code: Bjoern Giesler (2023)
 *   Stuttgart Comic Con droid building workshop
 * 
 * - WiFi Control Version: Seán McArdle / Hero Props (2025)
 *   heroprops.art | github.com/SeanMcArdle
 *   Adapted for The Bakken Museum STEAM workshop
 * 
 * Licensed under Apache License 2.0
 */

// ============================================================
// CONFIGURATION - EDIT THIS SECTION FOR EACH DROID
// ============================================================

// DROID IDENTITY (0-16, where 0 = demo unit)
#define DROID_NUMBER 0

// WiFi credentials are auto-generated from DROID_NUMBER:
// DROID_NUMBER 0  → SSID: "R2-BK00", Password: "droidBK00"
// DROID_NUMBER 1  → SSID: "R2-BK01", Password: "droidBK01"
// etc.

// SERVO SPEEDS (1-10, higher = faster response)
#define DRIVE_SPEED 5
#define TURN_SPEED 5
#define DOME_SPEED 5

// FEATURES (comment out to disable)
#define ENABLE_SOUND        // DFPlayer Mini for R2 sounds
#define ENABLE_BATTERY_MON  // Battery voltage monitoring
#define ENABLE_NEOPIXEL     // NeoPixel RGB LED

// DEBUG (uncomment for Serial output)
#define DEBUG_SERIAL

// ============================================================
// PIN DEFINITIONS - ESP32 DevKit (30-pin NodeMCU style)
// ============================================================

#define PIN_LEFT_SERVO   13   // Left wheel (360° continuous)
#define PIN_RIGHT_SERVO  12   // Right wheel (360° continuous)
#define PIN_DOME_SERVO   14   // Dome rotation (180° standard)
#define PIN_NEOPIXEL     4    // NeoPixel data pin
#define PIN_DFPLAYER_RX  16   // DFPlayer TX → ESP32 RX
#define PIN_DFPLAYER_TX  17   // DFPlayer RX ← ESP32 TX (use 1K resistor)
#define PIN_BATTERY      34   // Battery voltage (ADC, optional)

// ============================================================
// END CONFIGURATION - Don't edit below unless you know why
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

#ifdef ENABLE_SOUND
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
HardwareSerial dfSerial(2);  // Use UART2
DFRobotDFPlayerMini dfPlayer;
bool soundAvailable = false;
#endif

#ifdef ENABLE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
uint8_t pixelR = 0, pixelG = 255, pixelB = 255;  // Default: cyan (Clone Wars theme)
bool pixelOn = true;
#endif

// Generate WiFi credentials from droid number
String getDroidSSID() {
  char ssid[12];
  sprintf(ssid, "R2-BK%02d", DROID_NUMBER);
  return String(ssid);
}

String getDroidPassword() {
  char pass[12];
  sprintf(pass, "droidBK%02d", DROID_NUMBER);
  return String(pass);
}

// Servo objects
Servo leftServo;
Servo rightServo;
Servo domeServo;

// Web server on port 80
WebServer server(80);

// Forward declarations
void handleRoot();
void handleCommand();
void handleStatus();

// Current state
int currentDomePos = 90;  // Center position

// Servo control values
const int SERVO_STOP = 90;
const int SERVO_FULL_FWD = 180;
const int SERVO_FULL_REV = 0;

void setup() {
  #ifdef DEBUG_SERIAL
  Serial.begin(115200);
  Serial.println("\n\n=== BB-R2 WiFi Control ===");
  Serial.print("Droid: R2-BK");
  if (DROID_NUMBER < 10) Serial.print("0");
  Serial.println(DROID_NUMBER);
  #endif

  // Initialize NeoPixel
  #ifdef ENABLE_NEOPIXEL
  pixel.begin();
  pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
  pixel.show();
  #endif

  // Initialize servos
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  
  leftServo.setPeriodHertz(50);
  rightServo.setPeriodHertz(50);
  domeServo.setPeriodHertz(50);
  
  leftServo.attach(PIN_LEFT_SERVO, 500, 2400);
  rightServo.attach(PIN_RIGHT_SERVO, 500, 2400);
  domeServo.attach(PIN_DOME_SERVO, 500, 2400);
  
  // Start with everything stopped/centered
  leftServo.write(SERVO_STOP);
  rightServo.write(SERVO_STOP);
  domeServo.write(currentDomePos);

  #ifdef DEBUG_SERIAL
  Serial.println("Servos initialized");
  #endif

  // Initialize DFPlayer
  #ifdef ENABLE_SOUND
  dfSerial.begin(9600, SERIAL_8N1, PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
  delay(1000);  // Give DFPlayer time to boot
  
  if (dfPlayer.begin(dfSerial)) {
    soundAvailable = true;
    dfPlayer.volume(20);  // 0-30
    #ifdef DEBUG_SERIAL
    Serial.println("DFPlayer initialized");
    Serial.print("Files on SD: ");
    Serial.println(dfPlayer.readFileCounts());
    #endif
  } else {
    #ifdef DEBUG_SERIAL
    Serial.println("DFPlayer not found - sound disabled");
    #endif
  }
  #endif

  // Start WiFi Access Point
  String ssid = getDroidSSID();
  String password = getDroidPassword();
  
  WiFi.softAP(ssid.c_str(), password.c_str());
  
  #ifdef DEBUG_SERIAL
  Serial.println("\nWiFi Access Point started:");
  Serial.print("  SSID: ");
  Serial.println(ssid);
  Serial.print("  Password: ");
  Serial.println(password);
  Serial.print("  /Users/seanmcardle/Documents/Hero Props Projects/Hero Props Droid Workshops/Mini Chopper/My Build/Sketches/chopper_motor_test.inoIP: ");
  Serial.println(WiFi.softAPIP());
  #endif

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/status", handleStatus);
  server.begin();

  #ifdef DEBUG_SERIAL
  Serial.println("\nWeb server started - connect and browse to http://192.168.4.1");
  #endif

  // Startup animation
  #ifdef ENABLE_NEOPIXEL
  for (int i = 0; i < 3; i++) {
    pixel.setPixelColor(0, pixel.Color(0, 255, 255));  // Cyan
    pixel.show();
    delay(100);
    pixel.setPixelColor(0, pixel.Color(0, 0, 0));      // Off
    pixel.show();
    delay(100);
  }
  pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
  pixel.show();
  #endif

  // Startup sound
  #ifdef ENABLE_SOUND
  if (soundAvailable) {
    dfPlayer.play(1);  // Play first sound file
  }
  #endif
}

void loop() {
  server.handleClient();
}

// ============================================================
// MOVEMENT FUNCTIONS
// ============================================================

void driveForward() {
  int speed = map(DRIVE_SPEED, 1, 10, 95, 180);
  leftServo.write(speed);
  rightServo.write(180 - speed);  // Opposite direction
}

void driveBackward() {
  int speed = map(DRIVE_SPEED, 1, 10, 85, 0);
  leftServo.write(speed);
  rightServo.write(180 - speed);
}

void turnLeft() {
  int speed = map(TURN_SPEED, 1, 10, 95, 130);
  leftServo.write(180 - speed);  // Reverse
  rightServo.write(180 - speed); // Forward
}

void turnRight() {
  int speed = map(TURN_SPEED, 1, 10, 95, 130);
  leftServo.write(speed);        // Forward
  rightServo.write(speed);       // Reverse
}

void stopDrive() {
  leftServo.write(SERVO_STOP);
  rightServo.write(SERVO_STOP);
}

void domeLeft() {
  currentDomePos = constrain(currentDomePos - (DOME_SPEED * 3), 0, 180);
  domeServo.write(currentDomePos);
}

void domeRight() {
  currentDomePos = constrain(currentDomePos + (DOME_SPEED * 3), 0, 180);
  domeServo.write(currentDomePos);
}

void domeCenter() {
  currentDomePos = 90;
  domeServo.write(currentDomePos);
}

// ============================================================
// WEB SERVER HANDLERS
// ============================================================

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>R2-BK)rawliteral";
  
  html += (DROID_NUMBER < 10 ? "0" : "") + String(DROID_NUMBER);
  
  html += R"rawliteral( Control</title>
  <style>
    * { box-sizing: border-box; touch-action: manipulation; }
    body {
      font-family: -apple-system, Arial, sans-serif;
      background: #1a1a2e;
      color: #0ff;
      margin: 0;
      padding: 15px;
      min-height: 100vh;
    }
    h1 {
      text-align: center;
      font-size: 24px;
      margin: 0 0 15px 0;
      text-shadow: 0 0 10px #0ff;
    }
    .panel {
      background: #16213e;
      border: 2px solid #0f4c75;
      border-radius: 10px;
      padding: 15px;
      margin-bottom: 15px;
    }
    .panel h2 {
      margin: 0 0 10px 0;
      font-size: 14px;
      color: #888;
      text-transform: uppercase;
    }
    .btn-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
    }
    .btn {
      background: #0f4c75;
      border: none;
      border-radius: 8px;
      color: #0ff;
      padding: 20px 10px;
      font-size: 20px;
      cursor: pointer;
      transition: all 0.1s;
    }
    .btn:active { background: #0ff; color: #1a1a2e; }
    .btn.center { background: #1b262c; }
    .sound-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 8px;
    }
    .sound-btn {
      background: #2d4059;
      border: none;
      border-radius: 8px;
      color: #0ff;
      padding: 15px 5px;
      font-size: 12px;
      cursor: pointer;
    }
    .sound-btn:active { background: #0ff; color: #1a1a2e; }
    .status {
      text-align: center;
      padding: 10px;
      font-family: monospace;
      font-size: 12px;
      color: #666;
    }
    .toggle-row {
      display: flex;
      justify-content: space-around;
      margin-top: 10px;
    }
    .toggle-btn {
      background: #2d4059;
      border: 2px solid #0f4c75;
      border-radius: 20px;
      color: #0ff;
      padding: 10px 20px;
      font-size: 14px;
      cursor: pointer;
    }
    .toggle-btn.active {
      background: #0ff;
      color: #1a1a2e;
    }
    .hidden { display: none; }
  </style>
</head>
<body>
  <h1>🤖 R2-BK)rawliteral";

  html += (DROID_NUMBER < 10 ? "0" : "") + String(DROID_NUMBER);

  html += R"rawliteral(</h1>
  
  <div class="panel">
    <h2>Drive</h2>
    <div class="btn-grid">
      <div></div>
      <button class="btn" ontouchstart="cmd('fwd')" ontouchend="cmd('stop')" onmousedown="cmd('fwd')" onmouseup="cmd('stop')">▲</button>
      <div></div>
      <button class="btn" ontouchstart="cmd('left')" ontouchend="cmd('stop')" onmousedown="cmd('left')" onmouseup="cmd('stop')">◄</button>
      <button class="btn center" onclick="cmd('stop')">■</button>
      <button class="btn" ontouchstart="cmd('right')" ontouchend="cmd('stop')" onmousedown="cmd('right')" onmouseup="cmd('stop')">►</button>
      <div></div>
      <button class="btn" ontouchstart="cmd('back')" ontouchend="cmd('stop')" onmousedown="cmd('back')" onmouseup="cmd('stop')">▼</button>
      <div></div>
    </div>
  </div>

  <div class="panel">
    <h2>Dome</h2>
    <div class="btn-grid">
      <button class="btn" onclick="cmd('dome_left')">↶</button>
      <button class="btn center" onclick="cmd('dome_center')">⌾</button>
      <button class="btn" onclick="cmd('dome_right')">↷</button>
    </div>
  </div>

  <div class="panel)rawliteral";

  #ifdef ENABLE_SOUND
  html += R"rawliteral(">
    <h2>Sounds</h2>
    <div class="sound-grid">
      <button class="sound-btn" onclick="cmd('snd1')">Beep 1</button>
      <button class="sound-btn" onclick="cmd('snd2')">Beep 2</button>
      <button class="sound-btn" onclick="cmd('snd3')">Beep 3</button>
      <button class="sound-btn" onclick="cmd('snd4')">Scream</button>
      <button class="sound-btn" onclick="cmd('snd5')">Happy</button>
      <button class="sound-btn" onclick="cmd('snd6')">Sad</button>
      <button class="sound-btn" onclick="cmd('snd7')">Whistle</button>
      <button class="sound-btn" onclick="cmd('snd8')">Alarm</button>
    </div>
  </div>)rawliteral";
  #else
  html += R"rawliteral( hidden"></div>)rawliteral";
  #endif

  html += R"rawliteral(

  <div class="panel">
    <h2>Options</h2>
    <div class="toggle-row">
      <button class="toggle-btn" id="ledBtn" onclick="toggleLED()">💡 Light</button>
      <button class="toggle-btn" onclick="cmd('vol_up')">🔊 Vol+</button>
      <button class="toggle-btn" onclick="cmd('vol_down')">🔉 Vol-</button>
    </div>
    <div class="toggle-row" style="margin-top:8px;">
      <button class="toggle-btn" style="background:#0ff;color:#000;" onclick="cmd('color_cyan')">Cyan</button>
      <button class="toggle-btn" style="background:#f00;" onclick="cmd('color_red')">Red</button>
      <button class="toggle-btn" style="background:#0f0;color:#000;" onclick="cmd('color_green')">Green</button>
      <button class="toggle-btn" style="background:#ff0;color:#000;" onclick="cmd('color_gold')">Gold</button>
    </div>
  </div>

  <div class="status" id="status">Ready</div>

  <script>
    function cmd(c) {
      fetch('/cmd?c=' + c)
        .then(r => r.text())
        .then(t => document.getElementById('status').innerText = t)
        .catch(e => document.getElementById('status').innerText = 'Error');
    }
    
    function toggleLED() {
      cmd('led');
      document.getElementById('ledBtn').classList.toggle('active');
    }

    // Prevent context menu on long press
    document.addEventListener('contextmenu', e => e.preventDefault());
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleCommand() {
  String cmd = server.arg("c");
  String response = "OK: " + cmd;

  // Drive commands
  if (cmd == "fwd") {
    driveForward();
    response = "Driving forward";
  }
  else if (cmd == "back") {
    driveBackward();
    response = "Driving backward";
  }
  else if (cmd == "left") {
    turnLeft();
    response = "Turning left";
  }
  else if (cmd == "right") {
    turnRight();
    response = "Turning right";
  }
  else if (cmd == "stop") {
    stopDrive();
    response = "Stopped";
  }
  // Dome commands
  else if (cmd == "dome_left") {
    domeLeft();
    response = "Dome: " + String(currentDomePos) + "°";
  }
  else if (cmd == "dome_right") {
    domeRight();
    response = "Dome: " + String(currentDomePos) + "°";
  }
  else if (cmd == "dome_center") {
    domeCenter();
    response = "Dome centered";
  }
  // NeoPixel commands
  #ifdef ENABLE_NEOPIXEL
  else if (cmd == "led") {
    pixelOn = !pixelOn;
    if (pixelOn) {
      pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
    } else {
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
    }
    pixel.show();
    response = pixelOn ? "Light ON" : "Light OFF";
  }
  else if (cmd == "color_cyan") {
    pixelR = 0; pixelG = 255; pixelB = 255;
    pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
    pixel.show();
    pixelOn = true;
    response = "Color: Cyan";
  }
  else if (cmd == "color_red") {
    pixelR = 255; pixelG = 0; pixelB = 0;
    pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
    pixel.show();
    pixelOn = true;
    response = "Color: Red";
  }
  else if (cmd == "color_green") {
    pixelR = 0; pixelG = 255; pixelB = 0;
    pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
    pixel.show();
    pixelOn = true;
    response = "Color: Green";
  }
  else if (cmd == "color_gold") {
    pixelR = 255; pixelG = 180; pixelB = 0;
    pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
    pixel.show();
    pixelOn = true;
    response = "Color: Gold";
  }
  #endif
  // Sound commands
  #ifdef ENABLE_SOUND
  else if (cmd.startsWith("snd") && soundAvailable) {
    int sndNum = cmd.substring(3).toInt();
    if (sndNum >= 1 && sndNum <= 99) {
      dfPlayer.play(sndNum);
      response = "Playing sound " + String(sndNum);
    }
  }
  else if (cmd == "vol_up" && soundAvailable) {
    dfPlayer.volumeUp();
    response = "Volume up";
  }
  else if (cmd == "vol_down" && soundAvailable) {
    dfPlayer.volumeDown();
    response = "Volume down";
  }
  #endif

  #ifdef DEBUG_SERIAL
  Serial.println("CMD: " + cmd + " → " + response);
  #endif

  server.send(200, "text/plain", response);
}

void handleStatus() {
  String json = "{";
  json += "\"droid\":\"R2-BK" + String(DROID_NUMBER < 10 ? "0" : "") + String(DROID_NUMBER) + "\",";
  json += "\"dome\":" + String(currentDomePos);
  
  #ifdef ENABLE_NEOPIXEL
  json += ",\"light\":" + String(pixelOn ? "true" : "false");
  json += ",\"color\":[" + String(pixelR) + "," + String(pixelG) + "," + String(pixelB) + "]";
  #endif
  
  #ifdef ENABLE_SOUND
  json += ",\"sound\":" + String(soundAvailable ? "true" : "false");
  #endif
  
  #ifdef ENABLE_BATTERY_MON
  int rawADC = analogRead(PIN_BATTERY);
  float voltage = (rawADC / 4095.0) * 3.3 * 2;  // Assumes voltage divider
  json += ",\"battery\":" + String(voltage, 2);
  #endif
  
  json += "}";
  
  server.send(200, "application/json", json);
}
