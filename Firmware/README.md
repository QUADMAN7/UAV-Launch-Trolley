# Firmware Architecture & Control Logic

## 1. System Intent & Engineering Strategy
The primary function of the trolley firmware is to provide real-time launch telemetry and govern the automated release mechanism for the UAV. To ensure safe, reliable operations in field environments, the software architecture was designed around three core principles:

1. **Non-blocking Telemetry:** Measure vehicle speed, displacement, and acceleration continuously without stopping the main control loop.
2. **Fail-Safe Safety Interlocks:** Prevent premature mechanical actuation through a combination of physical hardware states and firmware logic gates.
3. **Zero-Infrastructure Monitoring:** Broadcast live flight metrics directly to operators without relying on external Wi-Fi networks or bulky ground control equipment.

---

## 2. Telemetry & Signal Processing

### Interrupt-Driven Kinematics
To capture high-speed wheel rotations accurately, wheel pulse detection is handled via hardware interrupts (`IRAM_ATTR`). Measuring microsecond intervals (`micros()`) between pulse triggers provides higher accuracy than polling the pin in the main loop.

### Noise Mitigation & Filtering
Raw velocity calculations from small-diameter wheels are inherently prone to mechanical vibration and pulse jitter. To deliver stable control inputs and clean UI visualization, the firmware processes raw sensor signals through two stages:

* **Exponential Moving Average (EMA) Filter:** Smooths transient velocity spikes using a weighting factor ($\alpha = 0.2$). This prioritizes trend stability while retaining low latency for rapid acceleration runs.
* **Dynamic Clamping:** Physical acceleration values are clamped within physical bounds ($\pm 20\text{ m/s}^2$) to eliminate erroneous mathematical noise spikes during rapid signal state changes.
* **Zero-Speed Timeout:** To prevent stale telemetry output when the trolley stops, a $0.5\text{-second}$ pulse timeout automatically resets velocity to zero if no new magnet pulses are registered.

---

## 3. Safety Interlocks & Release State Machine

Accidental release of the parallel linkage during pre-flight handling poses a direct hazard to the airframe and ground crew. To eliminate single-point failures, mechanical release requires three independent conditions to be satisfied simultaneously:
``` text
[ Hardware Arm Switch: ON ] ──┐
├─► [ Logical AND Gate ] ──► [ Command Servo Release ]
[ Weight-Off-Wheels: HIGH ]  ──┤
│
[ Launch Speed >= 18 m/s ]   ──┘
```

1. **Master Physical Arming Switch:** Mechanically isolates the system. When disarmed, the servo is held rigidly at $190^\circ$ (latching the linkage), regardless of sensor inputs.
2. **Audible & Visual State Feedback:** Upon arming, the ESP32 drives an LED and fires a piezo buzzer tone to alert field operators.
3. **Automated Velocity Threshold:** The release servo ($0^\circ$) will only trigger once smoothed ground velocity meets or exceeds the minimum flight speed ($18\text{ m/s}$) AND the weight-off-wheels sensor indicates the aircraft is lifting.

Upon successful release, the firmware latches the current displacement value to permanently record the total ground roll distance required for take-off.

---

## 4. Off-Grid Field User Interface
To eliminate the need for heavy ground station laptops, the ESP32 acts as a standalone Wi-Fi network hosting a HTTP web server. 

* The server delivers a single lightweight HTML/JS dashboard to connected mobile devices.
* Kinematic state data (`displacement`, `velocity`, `acceleration`, `takeOffDistance`) are packaged as lightweight JSON text and served at a `/data` web address that the browser reads automatically
* Client-side JavaScript polls this endpoint asynchronously every $200\text{ ms}$, delivering real-time numerical visual readouts without triggering full page reloads.

---

## 5. Prototype Trade-Offs & Production Enhancements

As an initial functional prototype, the system successfully validated automated launch mechanics in bench testing. For future production iterations, before conducting a real world test, the following software enhancements are planned:

* **Non-Volatile Logging:** Integrate SD card logging to store historical run data locally for post-flight telemetry analysis.
* **Dynamic Speed Calibration:** Allow operators to adjust the threshold launch speed directly from the web interface prior to arming, eliminating the need to re-flash firmware for different airframes.
