#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Hardware config
#define HALL_SENSOR_PIN 20  // Hall sensor pin
#define ARM_SWITCH_PIN 9    // Arm switch
#define ARM_LED_PIN 4       // Arm LED
#define BUTTON_PIN 5        // Momentary switch
#define SERVO_PIN 7         // Servo signal

const uint8_t NUM_MAGNETS = 1;
const float WHEEL_RADIUS = 0.1027 / 2.0; // meters
const float CIRCUMFERENCE = 2.0 * PI * WHEEL_RADIUS;

volatile unsigned long lastTime = 0;
volatile unsigned long period = 0;
volatile int hallCounter = 0;

float velocity = 0.0;
float displacement = 0.0;
float acceleration = 0.0;

float takeOffDistance = 0.0;
bool takeOffRecorded = false;

unsigned long prevMicros = 0;

const float LAUNCH_SPEED = 18;  // m/s launch speed threshold
const unsigned long VELOCITY_TIMEOUT_US = 500000; // 0.5 seconds timeout for velocity zero

bool systemArmed = false;
bool servoReleased = false; // True if servo moved to 0°
bool buttonPressed = false;

Servo myServo;

WebServer server(80);

// ISR for Hall sensor pulses
void IRAM_ATTR hallInterrupt() {
  unsigned long now = micros();
  unsigned long diff = now - lastTime;
  if (diff > 1000) { // debounce 1ms
    period = diff;
    lastTime = now;
    hallCounter++;
  }
}

// HTML page with telemetry and take-off distance display
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>VEHICLE TELEMETRY</title>
  <meta charset="UTF-8">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
    }
    h1 {
      font-family: Arial, sans-serif;
      font-size: 3em;
      color: black;
      margin-bottom: 30px;
    }
    table {
      margin: 0 auto;
      border-collapse: collapse;
      font-weight: bold;
    }
    table, th, td {
      border: 2px solid black;
      padding: 15px 30px;
    }
    th {
      font-size: 1.2em;
    }
    td {
      font-size: 1em;
    }
    #velocity {
      font-size: 2em;
      color: red;
    }
    .units {
      font-size: 0.7em;
      font-weight: normal;
      margin-left: 4px;
    }
    button {
      font-size: 1em;
      padding: 10px 25px;
      margin-top: 30px;
      cursor: pointer;
    }
    #takeoffContainer {
      font-size: 1.5em;
      margin-top: 20px;
    }
    .flashing {
      animation: flashRedBlack 0.5s infinite;
    }
    @keyframes flashRedBlack {
      0%, 100% { color: red; }
      50% { color: black; }
    }
  </style>
</head>
<body>
  <h1>VEHICLE TELEMETRY</h1>
  <table>
    <tr>
      <th>Displacement<span class="units">m</span></th>
      <th>Velocity<span class="units">m/s</span></th>
      <th>Acceleration<span class="units">m/s²</span></th>
    </tr>
    <tr>
      <td><span id="displacement">0.00</span></td>
      <td><span id="velocity">0.00</span></td>
      <td><span id="acceleration">0.00</span></td>
    </tr>
  </table>

  <div id="takeoffContainer"></div>

  <button onclick="resetTelemetry()">Reset Telemetry</button>

  <script>
    let flashTimeout;

    async function fetchData() {
      try {
        const res = await fetch('/data');
        const data = await res.json();

        document.getElementById('displacement').textContent = data.displacement.toFixed(2);
        document.getElementById('velocity').textContent = data.velocity.toFixed(2);
        document.getElementById('acceleration').textContent = data.acceleration.toFixed(2);

        updateTakeOffDistance(data);
      } catch (e) {
        console.error('Error fetching data:', e);
      }
    }

    function updateTakeOffDistance(data) {
      const container = document.getElementById("takeoffContainer");
      if (data.takeOffDistance > 0) {
        container.textContent = `Take-Off Distance: ${data.takeOffDistance.toFixed(2)} m`;
        if (data.flashing) {
          container.classList.add("flashing");
          if (flashTimeout) clearTimeout(flashTimeout);
          flashTimeout = setTimeout(() => {
            container.classList.remove("flashing");
          }, 3000);
        }
      } else {
        container.textContent = '';
        container.classList.remove("flashing");
      }
    }

    async function resetTelemetry() {
      await fetch('/reset');
    }

    setInterval(fetchData, 200);
  </script>
</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);

  pinMode(HALL_SENSOR_PIN, INPUT_PULLUP);
  pinMode(ARM_SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ARM_LED_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(HALL_SENSOR_PIN), hallInterrupt, RISING);

  WiFi.softAP("ESP_Telemetry", "God_Speed");
  Serial.println("WiFi AP started: ESP_Telemetry | Password: God_Speed");
  Serial.println("Go to http://192.168.4.1");

  server.on("/", []() {
    server.send_P(200, "text/html", index_html);
  });

  server.on("/data", []() {
    String json = "{\"displacement\":";
    json += displacement;
    json += ",\"velocity\":";
    json += velocity;
    json += ",\"acceleration\":";
    json += acceleration;
    json += ",\"takeOffDistance\":";
    json += takeOffDistance;
    json += ",\"flashing\":";
    json += (servoReleased ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/reset", []() {
    displacement = 0.0;
    velocity = 0.0;
    acceleration = 0.0;
    takeOffDistance = 0.0;
    takeOffRecorded = false;
    servoReleased = false;
    Serial.println("Telemetry Reset");
    server.send(200, "text/plain", "Telemetry Reset");
  });

  server.begin();

  myServo.attach(SERVO_PIN, 500, 2500);
  myServo.write(190);  // Default servo position (Locked)
}

#define ACCELERATION_CLAMP 20.0  // Max abs acceleration allowed (m/s²)

void loop() {
  server.handleClient();

  static int lastHallCount = 0;
  static float prevVelocity = 0.0;
  static float smoothedVelocity = 0.0;

  if (hallCounter != lastHallCount) {
    Serial.printf("Interrupts: %d, period (us): %lu\n", hallCounter, period);
    lastHallCount = hallCounter;
  }

  unsigned long currentMicros = micros();
  float dt = (currentMicros - prevMicros) / 1e6;

  if (dt < 0.0001) return; // ignore unrealistically small dt 

  if (dt > 0.001) {
    prevMicros = currentMicros;

    unsigned long timeSinceLastPulse = currentMicros - lastTime;

    if (timeSinceLastPulse > VELOCITY_TIMEOUT_US) { // Timeout => wheel stopped
      velocity = 0.0;
    } else if (period > 0 && period < 1000000) { // ignore period > 1s
      float timeForOneRevolution = (period * NUM_MAGNETS) / 1e6;
      velocity = CIRCUMFERENCE / timeForOneRevolution;
    } else {
      velocity = 0.0;
    }

    const float alpha = 0.2;
    smoothedVelocity = alpha * velocity + (1 - alpha) * smoothedVelocity;

    // Only integrate displacement if velocity > 0 to prevent drift
    if (smoothedVelocity > 0.01) {
      displacement += smoothedVelocity * dt;
    }

    float deltaV = smoothedVelocity - prevVelocity;
    float accel = deltaV / dt;

    if (accel > ACCELERATION_CLAMP) accel = ACCELERATION_CLAMP;
    else if (accel < -ACCELERATION_CLAMP) accel = -ACCELERATION_CLAMP;

    acceleration = accel;
    prevVelocity = smoothedVelocity;
  }

  // Read arm switch and LED
  systemArmed = (digitalRead(ARM_SWITCH_PIN) == LOW);
  digitalWrite(ARM_LED_PIN, systemArmed ? HIGH : LOW);

  // Read momentary button (active LOW)
  bool currentButtonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // Logic for servo and takeOffDistance:
  // Servo moves to 0 degrees ONLY if:
  // - systemArmed == true
  // - launch speed reached
  // - momentary button released (NOT pressed)
  // Also servo should remain at 0° until system disarmed.

  // Detect button release event
  static bool lastButtonState = false;

  // On button release
  if (lastButtonState && !currentButtonPressed) {
    buttonPressed = false;
    // Check conditions for servo release and take-off distance
    if (systemArmed && smoothedVelocity >= LAUNCH_SPEED && !servoReleased && !takeOffRecorded) {
      myServo.write(0);
      servoReleased = true;
      takeOffDistance = displacement;
      takeOffRecorded = true;
      Serial.println("Servo Released and Take-Off Distance recorded");
    }
  }

  // Update button pressed state
  if (!lastButtonState && currentButtonPressed) {
    buttonPressed = true;
  }
  lastButtonState = currentButtonPressed;

  // If disarmed, servo returns to 190°
  if (!systemArmed && servoReleased) {
    myServo.write(190);
    servoReleased = false;
    takeOffRecorded = false;
    takeOffDistance = 0.0;
    Serial.println("System disarmed, servo reset");
  }
}
