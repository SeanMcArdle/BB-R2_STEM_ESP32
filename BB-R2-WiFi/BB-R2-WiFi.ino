/*
 * BB-R2 WiFi Control - Bakken Museum Workshop Version
 * 
 * Based on the original BB-R2 ESP32 project by Bjoern Giesler, 2023
 * Modified for WiFi control and educational workshops at The Bakken Museum
 * 
 * This version creates a WiFi Access Point that allows control via a web browser,
 * making it ideal for workshops where multiple droids need to be controlled
 * independently without Bluetooth pairing.
 * 
 * Licensed under Apache License 2.0
 * 
 * Workshop: December 30-31, 2025
 * Location: The Bakken Museum, Minneapolis, MN
 * 
 * SETUP INSTRUCTIONS:
 * 1. Edit config.h to set DROID_NUMBER (0-16)
 * 2. Edit config.h to select your BOARD_TYPE
 * 3. Upload to ESP32
 * 4. Connect to WiFi network "R2-BKxx" (password: droidBKxx)
 * 5. Open browser to http://192.168.4.1
 */

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

#if ENABLE_SOUND
#include <DFRobotDFPlayerMini.h>
#endif

// ============================================================================
// SERVO CONTROL - Preserving Bjoern's elegant architecture
// ============================================================================

typedef struct {
  unsigned int pin;
  unsigned int goal;
  int current;
  unsigned int min, max;
  unsigned int speed;
} ServoState;

typedef enum {
  LEFT  = 0,
  RIGHT = 1,
  DOME  = 2
} ServoIndex;

ServoState servoStates[3] = {
  {SERVO_LEFT_PIN,  SERVO_CENTER, SERVO_CENTER, SERVO_MIN, SERVO_MAX, SERVO_SPEED},
  {SERVO_RIGHT_PIN, SERVO_CENTER, SERVO_CENTER, SERVO_MIN, SERVO_MAX, SERVO_SPEED},
  {SERVO_DOME_PIN,  SERVO_CENTER, SERVO_CENTER, SERVO_MIN, SERVO_MAX, SERVO_SPEED}
};

Servo servoLeft;
Servo servoRight;
Servo servoDome;

// ============================================================================
// SOUND SUPPORT (Optional)
// ============================================================================

#if ENABLE_SOUND
DFRobotDFPlayerMini dfp;
HardwareSerial dfpSerial(1);
bool soundEnabled = false;
#endif

// LED state
bool ledState = false;

// ============================================================================
// WEB SERVER
// ============================================================================

WebServer server(80);

// HTML for the web interface - Clone Wars inspired theme
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>BB-R2 Control</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Arial', sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      color: #00fff5;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      min-height: 100vh;
      padding: 20px;
      touch-action: manipulation;
    }
    h1 {
      text-align: center;
      margin-bottom: 10px;
      text-shadow: 0 0 10px #00fff5;
      font-size: 2em;
    }
    .subtitle {
      text-align: center;
      color: #888;
      margin-bottom: 30px;
      font-size: 0.9em;
    }
    .control-panel {
      background: rgba(0, 0, 0, 0.5);
      border: 2px solid #00fff5;
      border-radius: 15px;
      padding: 20px;
      box-shadow: 0 0 20px rgba(0, 255, 245, 0.3);
      max-width: 500px;
      width: 100%;
    }
    .section { margin-bottom: 25px; }
    .section h2 {
      font-size: 1.2em;
      margin-bottom: 15px;
      color: #00fff5;
      text-transform: uppercase;
      letter-spacing: 2px;
    }
    .button-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
    }
    .button-grid-2 {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 10px;
    }
    button {
      background: linear-gradient(135deg, #0f3460 0%, #16213e 100%);
      border: 2px solid #00fff5;
      color: #00fff5;
      padding: 20px;
      font-size: 1.1em;
      font-weight: bold;
      border-radius: 10px;
      cursor: pointer;
      transition: all 0.2s;
      touch-action: manipulation;
      -webkit-tap-highlight-color: transparent;
      text-transform: uppercase;
    }
    button:active {
      background: linear-gradient(135deg, #00fff5 0%, #0f3460 100%);
      color: #16213e;
      transform: scale(0.95);
      box-shadow: 0 0 15px #00fff5;
    }
    .btn-forward { grid-column: 2; }
    .btn-backward { grid-column: 2; }
    .btn-left { grid-column: 1; }
    .btn-right { grid-column: 3; }
    .btn-stop {
      grid-column: 2;
      background: linear-gradient(135deg, #8B0000 0%, #550000 100%);
      border-color: #ff0000;
    }
    .btn-stop:active {
      background: linear-gradient(135deg, #ff0000 0%, #8B0000 100%);
      box-shadow: 0 0 15px #ff0000;
    }
    .status {
      text-align: center;
      margin-top: 20px;
      padding: 10px;
      background: rgba(0, 255, 245, 0.1);
      border-radius: 8px;
      font-size: 0.9em;
    }
    .battery-indicator {
      position: fixed;
      top: 10px;
      right: 10px;
      padding: 8px 12px;
      background: #16213e;
      border-radius: 8px;
      font-size: 14px;
    }
    .battery-low { color: #ff4444; }
    .battery-med { color: #ffaa00; }
    .battery-good { color: #00ff88; }
    .sound-section { display: none; }
    .sound-enabled .sound-section { display: block; }
  </style>
</head>
<body>
  <div class="battery-indicator">
    🔋 <span id="batt">--</span>%
  </div>
  <h1>🤖 BB-R2 CONTROL</h1>
  <div class="subtitle">Bakken Museum Workshop Edition</div>
  
  <div class="control-panel">
    <div class="section">
      <h2>Movement</h2>
      <div class="button-grid">
        <button class="btn-forward" ontouchstart="sendCmd('F')" ontouchend="sendCmd('S')" onmousedown="sendCmd('F')" onmouseup="sendCmd('S')">▲<br>Forward</button>
        <button class="btn-left" ontouchstart="sendCmd('L')" ontouchend="sendCmd('S')" onmousedown="sendCmd('L')" onmouseup="sendCmd('S')">◄<br>Left</button>
        <button class="btn-stop" onclick="sendCmd('S')">■<br>Stop</button>
        <button class="btn-right" ontouchstart="sendCmd('R')" ontouchend="sendCmd('S')" onmousedown="sendCmd('R')" onmouseup="sendCmd('S')">►<br>Right</button>
        <button class="btn-backward" ontouchstart="sendCmd('B')" ontouchend="sendCmd('S')" onmousedown="sendCmd('B')" onmouseup="sendCmd('S')">▼<br>Back</button>
      </div>
    </div>

    <div class="section">
      <h2>Dome Rotation</h2>
      <div class="button-grid">
        <button ontouchstart="sendCmd('DL')" ontouchend="sendCmd('DS')" onmousedown="sendCmd('DL')" onmouseup="sendCmd('DS')">◄<br>Dome Left</button>
        <button onclick="sendCmd('DS')">■<br>Stop</button>
        <button ontouchstart="sendCmd('DR')" ontouchend="sendCmd('DS')" onmousedown="sendCmd('DR')" onmouseup="sendCmd('DS')">►<br>Dome Right</button>
      </div>
    </div>

    <div class="section sound-section">
      <h2>Sounds</h2>
      <div class="button-grid">
        <button onclick="sendCmd('SND1')">🔊<br>Sound 1</button>
        <button onclick="sendCmd('SND2')">🔊<br>Sound 2</button>
        <button onclick="sendCmd('SND3')">🔊<br>Sound 3</button>
      </div>
    </div>

    <div class="section">
      <h2>LED Control</h2>
      <div class="button-grid-2">
        <button onclick="sendCmd('LED_ON')">💡<br>LED On</button>
        <button onclick="sendCmd('LED_OFF')">○<br>LED Off</button>
      </div>
    </div>

    <div class="status" id="status">Ready to control</div>
  </div>

  <script>
    function sendCmd(cmd) {
      fetch('/cmd?c=' + cmd)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = 'Command: ' + cmd;
        })
        .catch(error => {
          document.getElementById('status').innerText = 'Error: ' + error;
        });
    }
    
    function updateBattery() {
      fetch('/battery')
        .then(r => r.text())
        .then(b => {
          const el = document.getElementById('batt');
          el.innerText = b;
          el.className = b < 20 ? 'battery-low' : b < 50 ? 'battery-med' : 'battery-good';
        })
        .catch(e => console.log(e));
    }
    
    function checkSound() {
      fetch('/status')
        .then(r => r.json())
        .then(s => {
          if(s.sound) document.body.classList.add('sound-enabled');
        })
        .catch(e => {});
    }
    
    setInterval(updateBattery, 10000);
    updateBattery();
    checkSound();
    
    document.body.addEventListener('touchmove', function(e) {
      e.preventDefault();
    }, { passive: false });
  </script>
</body>
</html>
)rawliteral";

// ============================================================================
// BATTERY MONITOR
// ============================================================================

#if ENABLE_BATTERY_MON
float getBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  float voltage = (raw / (float)ADC_RESOLUTION) * 3.3;
  voltage = voltage * ((BATTERY_R1 + BATTERY_R2) / BATTERY_R2);
  return voltage;
}

int getBatteryPercent() {
  float v = getBatteryVoltage();
  int vMillivolts = (int)(v * 1000);
  int percent = map(vMillivolts, BATTERY_MIN_MV, BATTERY_MAX_MV, 0, 100);
  return constrain(percent, 0, 100);
}
#endif

// ============================================================================
// WEB SERVER HANDLERS
// ============================================================================

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleBattery() {
#if ENABLE_BATTERY_MON
  int percent = getBatteryPercent();
  server.send(200, "text/plain", String(percent));
#else
  server.send(200, "text/plain", "100");
#endif
}

void handleStatus() {
  String json = "{";
#if ENABLE_SOUND
  json += "\"sound\":" + String(soundEnabled ? "true" : "false");
#else
  json += "\"sound\":false";
#endif
  json += "}";
  server.send(200, "application/json", json);
}

void handleCommand() {
  if (server.hasArg("c")) {
    String cmd = server.arg("c");
    processCommand(cmd);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing command");
  }
}

void processCommand(String cmd) {
#if DEBUG_SERIAL
  Serial.println("Command: " + cmd);
#endif
  
  // Movement commands - using config values
  if (cmd == "F") {
    servoStates[LEFT].goal = SERVO_CENTER + DRIVE_SPEED;
    servoStates[RIGHT].goal = SERVO_CENTER + DRIVE_SPEED;
  } else if (cmd == "B") {
    servoStates[LEFT].goal = SERVO_CENTER - DRIVE_SPEED;
    servoStates[RIGHT].goal = SERVO_CENTER - DRIVE_SPEED;
  } else if (cmd == "L") {
    servoStates[LEFT].goal = SERVO_CENTER - TURN_SPEED;
    servoStates[RIGHT].goal = SERVO_CENTER + TURN_SPEED;
  } else if (cmd == "R") {
    servoStates[LEFT].goal = SERVO_CENTER + TURN_SPEED;
    servoStates[RIGHT].goal = SERVO_CENTER - TURN_SPEED;
  } else if (cmd == "S") {
    servoStates[LEFT].goal = SERVO_CENTER;
    servoStates[RIGHT].goal = SERVO_CENTER;
  }
  
  // Dome commands
  else if (cmd == "DL") {
    servoStates[DOME].goal = SERVO_CENTER - DOME_SPEED;
  } else if (cmd == "DR") {
    servoStates[DOME].goal = SERVO_CENTER + DOME_SPEED;
  } else if (cmd == "DS") {
    servoStates[DOME].goal = SERVO_CENTER;
  }
  
  // Sound commands
#if ENABLE_SOUND
  else if (cmd == "SND1" && soundEnabled) {
    dfp.play(1);
  } else if (cmd == "SND2" && soundEnabled) {
    dfp.play(2);
  } else if (cmd == "SND3" && soundEnabled) {
    dfp.play(3);
  } else if (cmd == "SNDNEXT" && soundEnabled) {
    dfp.next();
  }
#endif
  
  // LED commands
#if ENABLE_LED
  else if (cmd == "LED_ON") {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
  } else if (cmd == "LED_OFF") {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
  }
#endif
}

// ============================================================================
// SERVO MOVEMENT - Bjoern's smooth interpolation
// ============================================================================

void moveServos() {
  for(auto& s: servoStates) {
    if(s.current < s.goal) {
      s.current += s.speed;
    } else if(s.current > s.goal) {
      s.current -= s.speed;
    }

    if(s.current < s.min) s.current = s.min;
    if(s.current > s.max) s.current = s.max;

    if(s.pin == SERVO_LEFT_PIN) {
      servoLeft.write(s.current);
    } else if(s.pin == SERVO_RIGHT_PIN) {
      servoRight.write(s.current);
    } else if(s.pin == SERVO_DOME_PIN) {
      servoDome.write(s.current);
    }
  }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
#if DEBUG_SERIAL
  Serial.begin(SERIAL_BAUD);
  Serial.println("\n==========================================");
  Serial.println("BB-R2 WiFi Control - Bakken Museum Edition");
  Serial.println("==========================================");
  Serial.print("Droid: ");
  Serial.println(WIFI_SSID);
  Serial.print("Board: ");
#ifdef BOARD_XIAO_ESP32C3
  Serial.println("Xiao ESP32C3");
#elif defined(BOARD_ESP32_DEVKIT)
  Serial.println("ESP32 DevKit");
#endif
  Serial.println("------------------------------------------");
#endif

  // Initialize LED
#if ENABLE_LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
#endif

  // Initialize servos
  servoLeft.attach(SERVO_LEFT_PIN);
  servoRight.attach(SERVO_RIGHT_PIN);
  servoDome.attach(SERVO_DOME_PIN);
  
  servoLeft.write(SERVO_CENTER);
  servoRight.write(SERVO_CENTER);
  servoDome.write(SERVO_CENTER);
  
#if DEBUG_SERIAL
  Serial.println("Servos initialized");
  Serial.print("  Left:  GPIO "); Serial.println(SERVO_LEFT_PIN);
  Serial.print("  Right: GPIO "); Serial.println(SERVO_RIGHT_PIN);
  Serial.print("  Dome:  GPIO "); Serial.println(SERVO_DOME_PIN);
#endif

  // Initialize DFPlayer
#if ENABLE_SOUND
  dfpSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(1000);
  
  if(!dfp.begin(dfpSerial)) {
#if DEBUG_SERIAL
    Serial.println("DFPlayer: Not found (continuing without sound)");
#endif
    soundEnabled = false;
  } else {
#if DEBUG_SERIAL
    Serial.println("DFPlayer: Initialized");
    Serial.print("  RX: GPIO "); Serial.println(DFPLAYER_RX);
    Serial.print("  TX: GPIO "); Serial.println(DFPLAYER_TX);
#endif
    dfp.volume(20);
    soundEnabled = true;
  }
#endif

  // Set up WiFi Access Point
#if DEBUG_SERIAL
  Serial.println("------------------------------------------");
  Serial.println("Starting WiFi Access Point...");
#endif
  
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  
  IPAddress IP = WiFi.softAPIP();
#if DEBUG_SERIAL
  Serial.print("  SSID:     "); Serial.println(WIFI_SSID);
  Serial.print("  Password: "); Serial.println(WIFI_PASS);
  Serial.print("  IP:       "); Serial.println(IP);
#endif

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/battery", handleBattery);
  server.on("/status", handleStatus);
  
  server.begin();
  
#if DEBUG_SERIAL
  Serial.println("------------------------------------------");
  Serial.println("Ready! Connect to WiFi and open browser.");
  Serial.println("==========================================\n");
#endif
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  server.handleClient();
  moveServos();
  delay(10);
}
