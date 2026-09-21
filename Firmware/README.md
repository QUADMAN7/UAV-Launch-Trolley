## Firmware Architecture & Software Development Strategy

To develop a robust, field-ready control system for the trolley, I adopted an iterative, modular software engineering strategy. Rather than attempting to write a monolithic codebase from scratch, I systematically built, tested, and validated individual subsystems—first isolating real-time telemetry processing, then developing safety-critical actuation state machines, and finally fusing them into a unified firmware architecture. Leveraging modern AI-augmented engineering workflows, I used AI tools as a virtual pair-programmer to rapidly scaffold syntax, boilerplate Wi-Fi server code, and HTML dashboard layouts. This human-in-the-loop approach drastically accelerated development velocity, allowing me to concentrate my engineering efforts on control logic optimization, signal filtering, hardware safety interlocks, and empirical hardware-in-the-loop (HIL) validation.

### Phase 1: Kinematics & Real-Time Telemetry Pipeline

Development began by establishing a high-frequency telemetry pipeline to accurately measure cart displacement, velocity, and acceleration. Running on the ESP32, the initial firmware utilized hardware interrupts (IRAM_ATTR) bound to a Hall effect sensor to measure microsecond-level rotational periods between wheel-mounted magnets. To ensure reliable real-time calculations, I implemented a 1 ms software debounce window alongside a 0.5-second pulse timeout mechanism to prevent stale speed readings when coming to a complete stop.Raw sensor data from high-speed rotation is inherently noisy; to address this, I integrated an exponential moving average (EMA) filter ($\alpha = 0.2$) to smooth velocity calculations before numerical integration into displacement. Furthermore, I implemented an acceleration clamp ($\pm 20 \text{ m/s}^2$) to eliminate mathematical spikes caused by discrete pulse timing. To make this data actionable in the field without external network infrastructure, I configured the ESP32 as a standalone Wi-Fi SoftAP hosting an asynchronous web server. This server dynamically served a lightweight HTML/JavaScript dashboard, serving live JSON telemetry over HTTP REST endpoints so ground operators could monitor vehicle performance in real time from any mobile device.

```text
+-------------------------------------------------------------------------------+
|                             ESP32 FIRMWARE PIPELINE                           |
|                                                                               |
|  +--------------------+      +--------------------+      +-----------------+  |
|  | Hall Effect Sensor | ---> | Hardware Interrupt | ---> | Microsecond     |  |
|  | (Pulse Detection)  |      | (IRAM_ATTR ISR)    |      | Period Tracking |  |
|  +--------------------+      +--------------------+      +-----------------+  |
|                                                                   |           |
|                                                                   v           |
|  +--------------------+      +--------------------+      +-----------------+  |
|  | ESP32 SoftAP       | <--- | Exponential        | <--- | Velocity & Accel|  |
|  | Web Telemetry UI   |      | Smoothing Filter   |      | Calculation     |  |
|  +--------------------+      +--------------------+      +-----------------+  |
+-------------------------------------------------------------------------------+
```

### Phases 2 & 3: Safety Interlocks & Actuation Prototyping

In parallel, I developed the physical safety and release state machines across secondary prototypes to validate hardware interlocks. The primary objective was to eliminate accidental actuation risks during pre-flight handling. I implemented a strict hardware hierarchy: a heavy-duty mechanical arming switch was configured as a master gate, requiring physical engagement before any servo commands could be processed.To provide unambiguous human-machine feedback, engaging the arming switch drove an onboard status LED and triggered an audible piezo buzzer tone to alert ground crew in bright sunlight conditions. System actuation was governed by a 555-style pulse-width control using an analog servo. When disarmed, the control loop locked the servo at $190^\circ$ (holding the linkage latch); once armed and triggered by the weight-off-wheels limit switch, the servo swept to $0^\circ$, releasing the mechanical claw under elastomeric tension.

### Phase 4: Full System Integration & Automated Launch Logic

The final engineering stage brought kinematics, web networking, and physical interlocks into a single integrated firmware build. The unified software continuously evaluates vehicle dynamics against automated launch criteria.

```text
                                  [ SYSTEM DISARMED ]
                                           |
                                  (Arm Switch Flipped)
                                           v
                                   [ SYSTEM ARMED ]
                             (LED ON / Audible Buzzer Tone)
                                           |
                +--------------------------+--------------------------+
                |                                                     |
    (Speed < 10 m/s OR Button Held)                        (Speed >= 10 m/s AND
                |                                            Button Released)
                v                                                     v
    [ Maintain Holding State ]                             [ AUTOMATED LAUNCH ]
      (Servo at 190 degrees)                               - Servo Sweeps to 0 degrees
                                                           - Capture Take-off Distance
                                                           - Visual UI Alert Flashes
```

Under full system integration, the ESP32 constantly monitors whether the system is armed, whether the weight-off-wheels button has been released, and whether the smoothed velocity has crossed a predefined launch threshold (e.g., 10 m/s). Only when all conditions are simultaneously satisfied does the firmware command the servo to release, instantly dropping the parallel linkage flat. Upon successful release, the code latches the exact displacement value to record the true take-off run distance, pushing this metric to the live web UI alongside a visual alert. If the system is disarmed at any point, the servo automatically resets to its safe holding position, guaranteeing predictable, repeatable, and fail-safe operation during field testing.
