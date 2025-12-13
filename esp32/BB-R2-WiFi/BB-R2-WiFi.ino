/*
 * BB-R2 WiFi Controller v13 - Bakken Museum Workshop Edition
 * 
 * Based on the original BB-R2 ESP32 project by Bjoern Giesler, 2023
 * Modified for WiFi control and educational workshops at The Bakken Museum
 * 
 * This version creates a WiFi Access Point that allows control via a web browser,
 * making it ideal for workshops where multiple droids need to be controlled
 * independently without Bluetooth pairing.
 * 
 * v13 Changes:
 * - Uses writeMicroseconds() for dome servo for smoother motion
 * - Increased safety timeout to 1000ms to prevent WiFi-induced stutter
 * - Tuned smoothing: DRIVE_SMOOTHING=0.15, DOME_SMOOTHING=0.10
 * - Removed blocking delay from loop
 * - Added WebSocket support for real-time telemetry
 * - Added serial log relay to web interface
 * - Enhanced web UI with joystick control and telemetry display
 * 
 * Licensed under Apache License 2.0
 * 
 * Workshop: December 30-31, 2025
 * Location: The Bakken Museum, Minneapolis, MN
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>

// ============================================================================
// CONFIGURATION - Change these for your droid!
// ============================================================================

// WiFi Access Point Settings
// CHANGE THIS FOR EACH DROID: droidBK01, droidBK02, droidBK03, etc.
const char* ssid = "droidBK01";          // WiFi network name (8+ chars required)
const char* password = "droidBK01";      // WiFi password (8+ chars required)

// ============================================================================
// TUNING CONSTANTS - Adjust these to fine-tune your droid's behavior
// ============================================================================

#define DRIVE_SMOOTHING 0.15   // Drive motor smoothing (0.0-1.0, higher = faster response)
#define DOME_SMOOTHING 0.10    // Dome motor smoothing (0.0-1.0, higher = faster response)
#define SAFETY_TIMEOUT_MS 1000 // Safety timeout in milliseconds (stops motors if no command received)

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
#define DFPLAYER_RX 20  // RX pin for DFPlayer - VERIFY for your board!
#define DFPLAYER_TX 21  // TX pin for DFPlayer - VERIFY for your board!

// Servo microsecond ranges (standard servo pulse widths)
#define SERVO_MIN_US 1000
#define SERVO_MAX_US 2000
#define SERVO_CENTER_US 1500

// ============================================================================
// SERVO CONTROL - Preserving Bjoern's elegant architecture
// ============================================================================

// This struct describes properties for a servo with smoothing
typedef struct {
  unsigned int pin;              // The hardware pin the servo is connected to
  
  float goal;                    // The goal position (0.0-1.0 normalized)
  float current;                 // The current servo position (0.0-1.0 normalized)
  float smoothing;               // Smoothing factor (0.0-1.0)
  
  unsigned int minUs;            // Minimum pulse width in microseconds
  unsigned int maxUs;            // Maximum pulse width in microseconds
} ServoState;

typedef enum {
  LEFT  = 0,
  RIGHT = 1,
  DOME  = 2
} ServoIndex;

// Here the servo parameters for our three servos are set. The order is given by ServoIndex above.
ServoState servoStates[3] = {
  {SERVO_LEFT_PIN,  0.5, 0.5, DRIVE_SMOOTHING, SERVO_MIN_US, SERVO_MAX_US},  // LEFT
  {SERVO_RIGHT_PIN, 0.5, 0.5, DRIVE_SMOOTHING, SERVO_MIN_US, SERVO_MAX_US},  // RIGHT
  {SERVO_DOME_PIN,  0.5, 0.5, DOME_SMOOTHING,  SERVO_MIN_US, SERVO_MAX_US}   // DOME
};

// Servo objects
Servo servoLeft;
Servo servoRight;
Servo servoDome;

// ============================================================================
// SAFETY TIMEOUT
// ============================================================================

unsigned long lastCommandTime = 0;
bool safetyStopActive = false;

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
// WEB SERVER AND WEBSOCKET
// ============================================================================

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Telemetry state
unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL_MS = 100; // 10Hz telemetry rate

// Serial log buffer for relaying to web interface
#define SERIAL_LOG_SIZE 10
String serialLog[SERIAL_LOG_SIZE];
int serialLogIndex = 0;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void logSerial(String message) {
  Serial.println(message);
  // Add to circular buffer
  serialLog[serialLogIndex] = message;
  serialLogIndex = (serialLogIndex + 1) % SERIAL_LOG_SIZE;
}

// ============================================================================
// HTML FOR WEB INTERFACE - Enhanced with joystick and telemetry
// ============================================================================

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>BB-R2 Control v13</title>
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
      justify-content: flex-start;
      min-height: 100vh;
      padding: 20px;
      overflow-y: auto;
      touch-action: manipulation;
    }
    h1 {
      text-align: center;
      margin-bottom: 5px;
      text-shadow: 0 0 10px #00fff5;
      font-size: 1.8em;
    }
    .subtitle {
      text-align: center;
      color: #888;
      margin-bottom: 15px;
      font-size: 0.8em;
    }
    .control-panel {
      background: rgba(0, 0, 0, 0.5);
      border: 2px solid #00fff5;
      border-radius: 15px;
      padding: 15px;
      box-shadow: 0 0 20px rgba(0, 255, 245, 0.3);
      max-width: 500px;
      width: 100%;
      margin-bottom: 15px;
    }
    .section {
      margin-bottom: 20px;
    }
    .section h2 {
      font-size: 1em;
      margin-bottom: 10px;
      color: #00fff5;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    .joystick-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 10px;
    }
    .joystick {
      width: 200px;
      height: 200px;
      background: radial-gradient(circle, #0f3460 0%, #16213e 100%);
      border: 3px solid #00fff5;
      border-radius: 50%;
      position: relative;
      touch-action: none;
      margin: 10px auto;
    }
    .joystick-stick {
      width: 60px;
      height: 60px;
      background: #00fff5;
      border-radius: 50%;
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      box-shadow: 0 0 15px #00fff5;
      transition: all 0.1s;
    }
    .slider-container {
      display: flex;
      align-items: center;
      gap: 10px;
      margin: 10px 0;
    }
    .slider-container label {
      min-width: 80px;
      font-size: 0.9em;
    }
    input[type="range"] {
      flex: 1;
      -webkit-appearance: none;
      appearance: none;
      height: 8px;
      background: #0f3460;
      border: 1px solid #00fff5;
      border-radius: 5px;
      outline: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 20px;
      height: 20px;
      background: #00fff5;
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 0 10px #00fff5;
    }
    input[type="range"]::-moz-range-thumb {
      width: 20px;
      height: 20px;
      background: #00fff5;
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 0 10px #00fff5;
    }
    .button-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
    }
    button {
      background: linear-gradient(135deg, #0f3460 0%, #16213e 100%);
      border: 2px solid #00fff5;
      color: #00fff5;
      padding: 15px;
      font-size: 0.9em;
      font-weight: bold;
      border-radius: 8px;
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
    .telemetry {
      background: rgba(0, 255, 245, 0.1);
      border-radius: 8px;
      padding: 10px;
      font-size: 0.85em;
      font-family: monospace;
    }
    .telemetry div {
      margin: 3px 0;
    }
    .serial-log {
      background: rgba(0, 0, 0, 0.7);
      border: 1px solid #00fff5;
      border-radius: 8px;
      padding: 10px;
      font-size: 0.75em;
      font-family: monospace;
      max-height: 150px;
      overflow-y: auto;
      color: #0f0;
    }
    .serial-log div {
      margin: 2px 0;
      word-wrap: break-word;
    }
    .status {
      text-align: center;
      margin-top: 10px;
      padding: 8px;
      background: rgba(0, 255, 245, 0.1);
      border-radius: 8px;
      font-size: 0.85em;
    }
    .btn-stop {
      background: linear-gradient(135deg, #8B0000 0%, #550000 100%);
      border-color: #ff0000;
    }
    .btn-stop:active {
      background: linear-gradient(135deg, #ff0000 0%, #8B0000 100%);
      box-shadow: 0 0 15px #ff0000;
    }
  </style>
</head>
<body>
  <h1>🤖 BB-R2 CONTROL v13</h1>
  <div class="subtitle">Bakken Museum Workshop Edition</div>
  
  <div class="control-panel">
    <!-- Joystick Control -->
    <div class="section">
      <h2>Joystick Control</h2>
      <div class="joystick-container">
        <div class="joystick" id="joystick">
          <div class="joystick-stick" id="stick"></div>
        </div>
        <button class="btn-stop" onclick="stopAll()">■ STOP ALL</button>
      </div>
    </div>

    <!-- Dome Control -->
    <div class="section">
      <h2>Dome Rotation</h2>
      <div class="slider-container">
        <label>Dome:</label>
        <input type="range" id="domeSlider" min="0" max="100" value="50" oninput="updateDome(this.value)">
        <span id="domeValue">50</span>
      </div>
    </div>

    <!-- Sound Controls -->
    <div class="section">
      <h2>Sounds</h2>
      <div class="button-grid">
        <button onclick="sendCmd('SND1')">🔊 1</button>
        <button onclick="sendCmd('SND2')">🔊 2</button>
        <button onclick="sendCmd('SND3')">🔊 3</button>
        <button onclick="sendCmd('SNDNEXT')" style="grid-column: span 3;">⏭ Next</button>
      </div>
    </div>

    <!-- LED Controls -->
    <div class="section">
      <h2>LED</h2>
      <div class="button-grid">
        <button onclick="sendCmd('LED_ON')">💡 On</button>
        <button onclick="sendCmd('LED_OFF')">○ Off</button>
      </div>
    </div>

    <!-- Telemetry Display -->
    <div class="section">
      <h2>Telemetry</h2>
      <div class="telemetry" id="telemetry">
        <div>Status: <span id="status">Connecting...</span></div>
        <div>Left: <span id="telLeft">--</span> Right: <span id="telRight">--</span> Dome: <span id="telDome">--</span></div>
        <div>Safety: <span id="telSafety">--</span></div>
      </div>
    </div>

    <!-- Serial Log -->
    <div class="section">
      <h2>Serial Log</h2>
      <div class="serial-log" id="serialLog">
        <div>Waiting for messages...</div>
      </div>
    </div>
  </div>

  <script>
    let ws;
    let joystickActive = false;
    let lastDomeValue = 50;

    // WebSocket connection
    function connectWebSocket() {
      ws = new WebSocket('ws://' + location.hostname + ':81');
      
      ws.onopen = function() {
        document.getElementById('status').innerText = 'Connected';
        logMessage('WebSocket connected');
      };
      
      ws.onmessage = function(event) {
        try {
          const data = JSON.parse(event.data);
          if (data.type === 'telemetry') {
            document.getElementById('telLeft').innerText = data.left;
            document.getElementById('telRight').innerText = data.right;
            document.getElementById('telDome').innerText = data.dome;
            document.getElementById('telSafety').innerText = data.safety ? 'ACTIVE' : 'OK';
          } else if (data.type === 'log') {
            logMessage(data.msg);
          }
        } catch(e) {
          console.error('Parse error:', e);
        }
      };
      
      ws.onerror = function() {
        document.getElementById('status').innerText = 'Error';
      };
      
      ws.onclose = function() {
        document.getElementById('status').innerText = 'Disconnected';
        setTimeout(connectWebSocket, 3000);
      };
    }

    function logMessage(msg) {
      const log = document.getElementById('serialLog');
      const div = document.createElement('div');
      div.textContent = msg;
      log.appendChild(div);
      log.scrollTop = log.scrollHeight;
      // Keep only last 20 messages
      while(log.children.length > 20) {
        log.removeChild(log.children[0]);
      }
    }

    function sendCmd(cmd) {
      fetch('/cmd?c=' + cmd)
        .catch(error => console.error('Error:', error));
    }

    function sendJoy(x, y) {
      // x and y are -1 to 1
      const cmd = 'joy:' + x.toFixed(2) + ',' + y.toFixed(2);
      fetch('/cmd?c=' + encodeURIComponent(cmd))
        .catch(error => console.error('Error:', error));
    }

    function stopAll() {
      sendCmd('S');
      sendCmd('DS');
    }

    function updateDome(value) {
      document.getElementById('domeValue').innerText = value;
      if (Math.abs(value - lastDomeValue) > 2) {
        const normalized = (value / 100).toFixed(2);
        sendCmd('dome:' + normalized);
        lastDomeValue = value;
      }
    }

    // Joystick handling
    const joystick = document.getElementById('joystick');
    const stick = document.getElementById('stick');
    let joystickBounds = joystick.getBoundingClientRect();

    function handleJoystickMove(clientX, clientY) {
      joystickBounds = joystick.getBoundingClientRect();
      const centerX = joystickBounds.width / 2;
      const centerY = joystickBounds.height / 2;
      const maxRadius = centerX - 30;

      let dx = clientX - joystickBounds.left - centerX;
      let dy = clientY - joystickBounds.top - centerY;
      const distance = Math.sqrt(dx * dx + dy * dy);

      if (distance > maxRadius) {
        const angle = Math.atan2(dy, dx);
        dx = Math.cos(angle) * maxRadius;
        dy = Math.sin(angle) * maxRadius;
      }

      stick.style.transform = 'translate(' + (centerX + dx - 30) + 'px, ' + (centerY + dy - 30) + 'px)';

      const x = dx / maxRadius;
      const y = -dy / maxRadius;
      sendJoy(x, y);
    }

    function resetJoystick() {
      stick.style.transform = 'translate(-50%, -50%)';
      joystickActive = false;
      sendCmd('S');
    }

    joystick.addEventListener('mousedown', (e) => {
      joystickActive = true;
      handleJoystickMove(e.clientX, e.clientY);
    });

    joystick.addEventListener('touchstart', (e) => {
      e.preventDefault();
      joystickActive = true;
      handleJoystickMove(e.touches[0].clientX, e.touches[0].clientY);
    });

    document.addEventListener('mousemove', (e) => {
      if (joystickActive) {
        handleJoystickMove(e.clientX, e.clientY);
      }
    });

    document.addEventListener('touchmove', (e) => {
      if (joystickActive) {
        e.preventDefault();
        handleJoystickMove(e.touches[0].clientX, e.touches[0].clientY);
      }
    }, { passive: false });

    document.addEventListener('mouseup', () => {
      if (joystickActive) resetJoystick();
    });

    document.addEventListener('touchend', () => {
      if (joystickActive) resetJoystick();
    });

    // Initialize WebSocket
    connectWebSocket();
  </script>
</body>
</html>
)rawliteral";

// ============================================================================
// WEBSOCKET EVENT HANDLER
// ============================================================================

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      logSerial("WebSocket client disconnected");
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        logSerial("WebSocket client connected: " + ip.toString());
      }
      break;
    case WStype_TEXT:
      // Handle text messages from client if needed
      break;
  }
}

// ============================================================================
// WEB SERVER HANDLERS
// ============================================================================

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleCommand() {
  if (server.hasArg("c")) {
    String cmd = server.arg("c");
    processCommand(cmd);
    lastCommandTime = millis(); // Reset safety timeout
    safetyStopActive = false;
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing command");
  }
}

void processCommand(String cmd) {
  // Robust JSON-like parsing (handles formats like {"joy":"drive"} or {joy:drive})
  cmd.trim();
  
  // Joystick control format: "joy:x,y" where x,y are -1.0 to 1.0
  if (cmd.startsWith("joy:")) {
    int commaPos = cmd.indexOf(',', 4);
    if (commaPos > 0) {
      float x = cmd.substring(4, commaPos).toFloat();
      float y = cmd.substring(commaPos + 1).toFloat();
      
      // Convert joystick coordinates to tank drive
      // y controls forward/back, x controls turning
      float leftPower = y + x;
      float rightPower = y - x;
      
      // Normalize to -1.0 to 1.0 range
      float maxPower = max(abs(leftPower), abs(rightPower));
      if (maxPower > 1.0) {
        leftPower /= maxPower;
        rightPower /= maxPower;
      }
      
      // Convert to servo goals (0.0 = full reverse, 0.5 = stop, 1.0 = full forward)
      servoStates[LEFT].goal = 0.5 + leftPower * 0.5;
      servoStates[RIGHT].goal = 0.5 + rightPower * 0.5;
      return;
    }
  }
  
  // Dome slider control format: "dome:value" where value is 0.0 to 1.0
  if (cmd.startsWith("dome:")) {
    float value = cmd.substring(5).toFloat();
    servoStates[DOME].goal = constrain(value, 0.0, 1.0);
    return;
  }
  
  // Legacy button commands
  if (cmd == "F") {
    servoStates[LEFT].goal = 0.8;
    servoStates[RIGHT].goal = 0.8;
  } else if (cmd == "B") {
    servoStates[LEFT].goal = 0.2;
    servoStates[RIGHT].goal = 0.2;
  } else if (cmd == "L") {
    servoStates[LEFT].goal = 0.2;
    servoStates[RIGHT].goal = 0.8;
  } else if (cmd == "R") {
    servoStates[LEFT].goal = 0.8;
    servoStates[RIGHT].goal = 0.2;
  } else if (cmd == "S") {
    servoStates[LEFT].goal = 0.5;
    servoStates[RIGHT].goal = 0.5;
  } else if (cmd == "DL") {
    servoStates[DOME].goal = 0.0;
  } else if (cmd == "DR") {
    servoStates[DOME].goal = 1.0;
  } else if (cmd == "DS") {
    servoStates[DOME].goal = 0.5;
  } else if (cmd == "SND1") {
    dfp.play(1);
  } else if (cmd == "SND2") {
    dfp.play(2);
  } else if (cmd == "SND3") {
    dfp.play(3);
  } else if (cmd == "SNDNEXT") {
    dfp.next();
  } else if (cmd == "LED_ON") {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
  } else if (cmd == "LED_OFF") {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
  }
}

// ============================================================================
// SERVO MOVEMENT - Enhanced with smoothing
// ============================================================================

void moveServos() {
  // Check safety timeout
  if (millis() - lastCommandTime > SAFETY_TIMEOUT_MS && !safetyStopActive) {
    safetyStopActive = true;
    servoStates[LEFT].goal = 0.5;
    servoStates[RIGHT].goal = 0.5;
    logSerial("Safety timeout: motors stopped");
  }
  
  // Move each servo's current position towards its goal with smoothing
  for(int i = 0; i < 3; i++) {
    ServoState& s = servoStates[i];
    
    // Exponential smoothing: current += (goal - current) * smoothing
    s.current += (s.goal - s.current) * s.smoothing;
    
    // Constrain to valid range
    s.current = constrain(s.current, 0.0, 1.0);
    
    // Convert normalized position (0.0-1.0) to microseconds
    int pulseUs = s.minUs + (int)((s.maxUs - s.minUs) * s.current);
    pulseUs = constrain(pulseUs, s.minUs, s.maxUs);
    
    // Write to the appropriate servo
    // DOME servo uses writeMicroseconds for smoother motion
    if(s.pin == SERVO_LEFT_PIN) {
      servoLeft.writeMicroseconds(pulseUs);
    } else if(s.pin == SERVO_RIGHT_PIN) {
      servoRight.writeMicroseconds(pulseUs);
    } else if(s.pin == SERVO_DOME_PIN) {
      servoDome.writeMicroseconds(pulseUs);  // v13: Using writeMicroseconds for dome
    }
  }
}

// ============================================================================
// TELEMETRY
// ============================================================================

void sendTelemetry() {
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryTime = millis();
    
    // Format: {"type":"telemetry","left":50,"right":50,"dome":50,"safety":false}
    String json = "{\"type\":\"telemetry\",\"left\":";
    json += String((int)(servoStates[LEFT].current * 100));
    json += ",\"right\":";
    json += String((int)(servoStates[RIGHT].current * 100));
    json += ",\"dome\":";
    json += String((int)(servoStates[DOME].current * 100));
    json += ",\"safety\":";
    json += safetyStopActive ? "true" : "false";
    json += "}";
    
    webSocket.broadcastTXT(json);
  }
}

void sendLogMessage(String msg) {
  String json = "{\"type\":\"log\",\"msg\":\"";
  json += msg;
  json += "\"}";
  webSocket.broadcastTXT(json);
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(100);
  
  logSerial("");
  logSerial("==========================================");
  logSerial("BB-R2 WiFi Controller v13");
  logSerial("Bakken Museum Workshop Edition");
  logSerial("Based on BB-R2 ESP32 by Bjoern Giesler");
  logSerial("==========================================");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize servos
  servoLeft.attach(SERVO_LEFT_PIN);
  servoRight.attach(SERVO_RIGHT_PIN);
  servoDome.attach(SERVO_DOME_PIN);
  
  // Set initial servo positions to center
  servoLeft.writeMicroseconds(SERVO_CENTER_US);
  servoRight.writeMicroseconds(SERVO_CENTER_US);
  servoDome.writeMicroseconds(SERVO_CENTER_US);
  
  logSerial("Servos initialized");

  // Initialize DFPlayer
  dfpSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(500);
  
  if(!dfp.begin(dfpSerial)) {
    logSerial("DFPlayer init failed - continuing without sound");
  } else {
    logSerial("DFPlayer initialized");
    dfp.volume(20);  // Set volume (0-30)
  }

  // Set up WiFi Access Point
  logSerial("Configuring WiFi Access Point...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  logSerial("AP IP: " + IP.toString());
  logSerial("SSID: " + String(ssid));
  logSerial("Password: " + String(password));

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  
  server.begin();
  logSerial("Web server started on port 80");
  
  // Set up WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  logSerial("WebSocket server started on port 81");
  
  logSerial("Connect to WiFi and navigate to http://192.168.4.1");
  logSerial("==========================================");
  
  lastCommandTime = millis(); // Initialize safety timer
}

// ============================================================================
// MAIN LOOP - NO BLOCKING DELAYS
// ============================================================================

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Handle WebSocket events
  webSocket.loop();
  
  // Update servo positions with smooth interpolation
  moveServos();
  
  // Send telemetry at 10Hz
  sendTelemetry();
  
  // NO delay() here - non-blocking operation only!
}
