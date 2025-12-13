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
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi Access Point Settings
const char* ssid = "R2-BAKKEN-01";      // Change this for each droid (R2-BAKKEN-02, etc.)
const char* password = "droid123";

// ============== BATTERY MONITOR ==============
#define BATTERY_PIN A0  // ADC pin for voltage divider
#define R1 10000.0      // 10kΩ resistor
#define R2 10000.0      // 10kΩ resistor
#define ADC_MAX_VALUE 4095  // 12-bit ADC resolution
#define BATTERY_MIN_MV 4400  // 4x AA minimum voltage (mV)
#define BATTERY_MAX_MV 6000  // 4x AA maximum voltage (mV)

// Pin Assignments
#define SERVO_LEFT_PIN   2
#define SERVO_RIGHT_PIN  3
#define SERVO_DOME_PIN   4
#define LED_PIN          5

// DFPlayer uses Hardware Serial 1 (adjust RX/TX pins based on your ESP32 board)
// IMPORTANT: These pin numbers are board-specific. Check your ESP32 variant's pinout!
// WIRING NOTE: Connect ESP32 TX to DFPlayer RX, and ESP32 RX to DFPlayer TX
// For Seeed Xiao ESP32C3: Use GPIO 20 (RX) and GPIO 21 (TX)
// For ESP32 DevKit: Use GPIO 16 (RX) and GPIO 17 (TX)
// For other boards: Consult your board's pinout diagram
#define RX 20  // RX pin for DFPlayer - VERIFY for your board!
#define TX 21  // TX pin for DFPlayer - VERIFY for your board!

// ============================================================================
// SERVO CONTROL - Preserving Bjoern's elegant architecture
// ============================================================================

// This struct describes properties for a servo.
typedef struct {
  unsigned int pin;              // The hardware pin the servo is connected to
  
  unsigned int goal;             // The goal for the servo -- this gets set in loop()
  int current;                   // The current servo position -- this gets updated every loop to move towards goal
  unsigned int min, max;
  unsigned int speed;            // The speed to move the servo's current position to its goal
} ServoState;

typedef enum {
  LEFT  = 0,
  RIGHT = 1,
  DOME  = 2
} ServoIndex;

// Here the servo parameters for our three servos are set. The order is given by ServoIndex above.
ServoState servoStates[3] = {
  {SERVO_LEFT_PIN,  90, 90, 40, 150, 1},  // LEFT
  {SERVO_RIGHT_PIN, 90, 90, 40, 150, 1},  // RIGHT
  {SERVO_DOME_PIN,  90, 90, 40, 150, 1}   // DOME
};

// Servo objects
Servo servoLeft;
Servo servoRight;
Servo servoDome;

// ============================================================================
// SOUND AND LED SUPPORT
// ============================================================================

// Object to talk to the sound player
DFRobotDFPlayerMini dfp;
// Serial connection to talk to the sound player (using HardwareSerial 1 to avoid conflict with debug Serial)
HardwareSerial dfpSerial(1);

// LED state
bool ledState = false;

// ============================================================================
// WEB SERVER
// ============================================================================

WebServer server(80);

// HTML for the web interface - Clone Wars/Rebels inspired theme
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>BB-R2 Control</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
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
    .section {
      margin-bottom: 25px;
    }
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
  </style>
</head>
<body>
  <div class="battery-indicator">
    🔋 <span id="batt">--</span>%
  </div>
  <h1>🤖 BB-R2 CONTROL</h1>
  <div class="subtitle">Bakken Museum Workshop Edition</div>
  
  <div class="control-panel">
    <!-- Movement Controls -->
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

    <!-- Dome Controls -->
    <div class="section">
      <h2>Dome Rotation</h2>
      <div class="button-grid">
        <button ontouchstart="sendCmd('DL')" ontouchend="sendCmd('DS')" onmousedown="sendCmd('DL')" onmouseup="sendCmd('DS')">◄<br>Dome Left</button>
        <button onclick="sendCmd('DS')">■<br>Stop</button>
        <button ontouchstart="sendCmd('DR')" ontouchend="sendCmd('DS')" onmousedown="sendCmd('DR')" onmouseup="sendCmd('DS')">►<br>Dome Right</button>
      </div>
    </div>

    <!-- Sound Controls -->
    <div class="section">
      <h2>Sounds</h2>
      <div class="button-grid">
        <button onclick="sendCmd('SND1')">🔊<br>Sound 1</button>
        <button onclick="sendCmd('SND2')">🔊<br>Sound 2</button>
        <button onclick="sendCmd('SND3')">🔊<br>Sound 3</button>
        <button onclick="sendCmd('SNDNEXT')" style="grid-column: span 3;">⏭<br>Next Sound</button>
      </div>
    </div>

    <!-- LED Controls -->
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
    // Battery level thresholds
    const BATTERY_LOW_THRESHOLD = 20;
    const BATTERY_MED_THRESHOLD = 50;
    
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
          el.className = b < BATTERY_LOW_THRESHOLD ? 'battery-low' : b < BATTERY_MED_THRESHOLD ? 'battery-med' : 'battery-good';
        })
        .catch(e => console.log(e));
    }
    // Update every 10 seconds
    setInterval(updateBattery, 10000);
    // Initial read
    updateBattery();
    
    // Prevent scrolling on touch for better control experience
    document.body.addEventListener('touchmove', function(e) {
      e.preventDefault();
    }, { passive: false });
  </script>
</body>
</html>
)rawliteral";

// ============================================================================
// BATTERY MONITOR FUNCTIONS
// ============================================================================

float getBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  float voltage = (raw / (float)ADC_MAX_VALUE) * 3.3;  // ADC reading to voltage at pin
  voltage = voltage * ((R1 + R2) / R2);   // Account for voltage divider
  return voltage;
}

int getBatteryPercent() {
  float v = getBatteryVoltage();
  // Map battery voltage to percentage (4.4V empty to 6.0V full)
  int vMillivolts = (int)(v * 1000);
  int percent = map(vMillivolts, BATTERY_MIN_MV, BATTERY_MAX_MV, 0, 100);
  return constrain(percent, 0, 100);
}

// ============================================================================
// WEB SERVER HANDLERS
// ============================================================================

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleBattery() {
  int percent = getBatteryPercent();
  server.send(200, "text/plain", String(percent));
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
  Serial.println("Command received: " + cmd);
  
  // Movement commands
  if (cmd == "F") {
    // Forward
    servoStates[LEFT].goal = 130;
    servoStates[RIGHT].goal = 130;
  } else if (cmd == "B") {
    // Backward
    servoStates[LEFT].goal = 50;
    servoStates[RIGHT].goal = 50;
  } else if (cmd == "L") {
    // Left turn
    servoStates[LEFT].goal = 50;
    servoStates[RIGHT].goal = 130;
  } else if (cmd == "R") {
    // Right turn
    servoStates[LEFT].goal = 130;
    servoStates[RIGHT].goal = 50;
  } else if (cmd == "S") {
    // Stop
    servoStates[LEFT].goal = 90;
    servoStates[RIGHT].goal = 90;
  }
  
  // Dome commands
  else if (cmd == "DL") {
    // Dome left
    servoStates[DOME].goal = 40;
  } else if (cmd == "DR") {
    // Dome right
    servoStates[DOME].goal = 150;
  } else if (cmd == "DS") {
    // Dome stop
    servoStates[DOME].goal = 90;
  }
  
  // Sound commands
  else if (cmd == "SND1") {
    dfp.play(1);
  } else if (cmd == "SND2") {
    dfp.play(2);
  } else if (cmd == "SND3") {
    dfp.play(3);
  } else if (cmd == "SNDNEXT") {
    dfp.next();
  }
  
  // LED commands
  else if (cmd == "LED_ON") {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
  } else if (cmd == "LED_OFF") {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
  }
}

// ============================================================================
// SERVO MOVEMENT - Preserving Bjoern's smooth interpolation
// ============================================================================

void moveServos() {
  // Move each servo's current position towards its goal
  for(auto& s: servoStates) {
    // Move servo current towards servo goal...
    if(s.current < s.goal) {
      s.current += s.speed;
    } else if(s.current > s.goal) {
      s.current -= s.speed;
    }

    // Enforce limits
    if(s.current < s.min) {
      s.current = s.min;
    }
    if(s.current > s.max) {
      s.current = s.max;
    }

    // Write to the appropriate servo
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
  Serial.begin(115200);
  Serial.println("\n\n==========================================");
  Serial.println("BB-R2 WiFi Control - Bakken Museum Edition");
  Serial.println("Based on BB-R2 ESP32 by Bjoern Giesler, 2023");
  Serial.println("==========================================\n");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize servos
  servoLeft.attach(SERVO_LEFT_PIN);
  servoRight.attach(SERVO_RIGHT_PIN);
  servoDome.attach(SERVO_DOME_PIN);
  
  // Set initial servo positions
  servoLeft.write(90);
  servoRight.write(90);
  servoDome.write(90);
  
  Serial.println("Servos initialized");

  // Initialize DFPlayer
  dfpSerial.begin(9600, SERIAL_8N1, RX, TX);
  delay(1000);  // Give DFPlayer time to initialize
  
  if(!dfp.begin(dfpSerial)) {
    Serial.println("DFPlayer initialization failed!");
    Serial.println("Check connections or continue without sound");
  } else {
    Serial.println("DFPlayer initialized");
    dfp.volume(20);  // Set volume (0-30)
  }

  // Set up WiFi Access Point
  Serial.println("\nConfiguring WiFi Access Point...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/battery", handleBattery);
  
  server.begin();
  Serial.println("\nWeb server started");
  Serial.println("Connect to WiFi and navigate to http://192.168.4.1");
  Serial.println("==========================================\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Update servo positions with smooth interpolation
  moveServos();
  
  // Small delay for stability
  delay(10);
}
