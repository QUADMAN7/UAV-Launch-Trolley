# IoT-Enabled UAV Launch Trolley
<img width="4000" height="3000" alt="20260907_122211" src="https://github.com/user-attachments/assets/b2ab7ad9-4b26-42af-a88e-a00bbd395ad7" />

A custom-designed cart built to solve the high-speed launch constraints ($V_{\text{launch}} \approx 18\text{ m/s}$) of a 2.43 kg MTOW pusher-propeller Search and Rescue (SAR) fixed-wing UAV. 

The system integrates real-time wheel speed telemetry via an ESP32 microcontroller with an automated, weight-triggered folding linkage mechanism to protect the 13" rear-mounted propeller during takeoff.

---

## Technical Specifications

| Subsystem | Component / Technology | Operational Role |
| :--- | :--- | :--- |
| **Control Unit** | ESP32 (Wi-Fi AP mode) | Real-time sensor polling, telemetry hosting, servo actuation |
| **Velocity Sensing** | Neodymium Magnet + Hall Effect Sensor (GPIO 20) | Wheel RPM, instantaneous velocity, distance, acceleration |
| **Arming Controls** | Toggle Switch (GPIO 9) / LED (GPIO 4) | Master safety arming input and visual status feedback |
| **Liftoff Detection** | Pad Limit Switch (GPIO 5) | Instantaneous weight-off-wheels trigger |
| **Clearance System** | Servo-latch + Rubber-tension Parallel Linkage | Folds support arms flat to allow clearance for 13" pusher prop |
| **Telemetry Interface** | ESP32 Access Point + Embedded Web Server | Live Web UI streaming speed and state data over Wi-Fi |

---

### Design and Manufacture


---

## Key Mechatronic Features

### 1. Active Propeller Clearance Mechanism
Because the aircraft utilizes a large 13" rear-mounted pusher propeller, sitting flat on a traditional cart would result in a propeller strike as the aircraft accelerates past the cart during liftoff.

* **Support Carriage:** Parallel-motion linkage support arms hold the fuselage above the cart frame.
* **Weight-Off-Wheels Trigger:** A limit switch placed beneath the support pads detect when aerodynamic lift supports the aircraft's weight.
* **Rapid Retraction:** Upon weight removal, the ESP32 actuates a release servo. Rubber-band tension pulls the parallel linkages down, snapping the support arms completely flat before the spinning 13" prop passes over the cart.

### 2. Sensor Integration & Velocity Calculation
Groundspeed is measured using a Hall effect sensor triggered by a neodymium magnet embedded within one of the trolley wheels.

* **Frequency Counting:** Hardware interrupts track wheel revolution intervals ($\Delta t$).
* **Physics Calculations:** The onboard firmware continuously calculates:
  
  $$\text{Velocity } (v) = \frac{2 \pi r}{\Delta t}$$

  *(Where 'r' is wheel radius)*
* **Kinematics:** Integrates velocity over time to track overall runway displacement ($d$) and differentiates for acceleration ($a$).

### 3. Wireless IoT Telemetry
The ESP32 operates as a standalone Wi-Fi Access Point (SoftAP) running a lightweight HTTP/WebSocket web server. 
this allows me to connect a mobile device or laptop directly to the trolley to view real-time metrics during ground runs.

---

## Operational Flow Diagram

```text
[ Ground Acceleration Phase ]
  │  ├── Hall sensor measures wheel RPM -> ESP32 calculates velocity
  │  └── ESP32 streams live velocity, displacement and acceleration data to Web UI over Wi-Fi
  ▼
[ Lift-off Event (V_ground ≥ 18 m/s) ]
  │  ├── Aerodynamic lift removes 2.43 kg weight from cradle
  │  └── Support pad limit switch is in normally open position (Weight-Off-Wheels signal)
  ▼
[ Active Clearance Trigger ]
  │  ├── ESP32 receives limit switch signal
  │  ├── Release servo disengages support arm latch
  │  └── Rubber band folds linkages flat
  ▼
[ Safe Separation ]
     └── 13" pusher prop passes cleanly over folded cart
```
---
## Design Trade-Offs & Alternative Concepts
---
## Problems Encountered
Launch system iteration of ideas:

**1. 13" Propeller Clearance:** The UAV this trolley was designed for has a pusher design and uses a 13" propeller which is problematic when the UAV needs to take off because if it were supported by a box on wheels, the angle at which the UAV would have to climb away from the trolley would be extreme and incredibly difficult to achieve. This left me with two options for the design of the trolley. The first was to design it in such a way that the take-off path of the UAV's propeller would be unobstructed by somehow holding onto the wings instead of the fuselage. The second was to increase the clearance between the trolley and the UAV at the point of take-off. I decided to go with the second option as it offered a solution that was in a smaller and consequently lighter package which offered the benefit of being easier to transport and making it easier for the UAV to accelerate itself and the trolley more quickly (therefore using less runway). To come up with a system to increase the clearance between the UAV and the trolley suppsort pads upon which the UAV sits, i researched different mechanical systems including scissor lifts and .... but decided to go with a parallel motion linkage as it was simpler and could quickly be actuated to go from its highest point to its lowest point

**2. Sensor Saturation:** In my initial design of the wheel that houses the small neodimyium magnets so that the hall effect sensor can calculate the velocity of the trolley, i included 8 magnets as i wanted a high resolution in the accuracy of the velocity reading seeing as the reading would be updated every one eighth of a rotation but in my testing of this sytem i found that this only worked for low velocities (i.e. <5 m/s). Above this velocity, the magnets would come flying past the hall effect sensor too quickly and would saturate the sensor as it did not have the responsiveness needed to react to the rapid change in whether it could detect a magnets presence or not so at high velocities it would would never detect the absence of a magnet in the rotation of the wheel which led to a calculated velocity of 0 m/s as in the code i included a series of logical steps that would mean if the velocity reading doesn't change in a given amount of time, the trolley must have stopped moving. without this logic gate, once the cart slows to a low velocity and stops, unless the magnet swings past the hall effect sensor very slowly (which is unlikely) it would not update the velocity so the reading would be stuck on the low velocity reading from when the magnet previously passed by. to solve the sensor saturation issue i had to reduce the number of magnets in the wheel to just one so that the trolley could record velocities in the range i would expect the UAV to take off at (up to 25 m/s). In reality this system has never been tested and i have a sneaky feeling that at the higher end of that range (>15m/s, the wheels will be spinning so fast that the sensor will reach saturation again despite the reduction in the number of magnets. This issue could easily be solved by either increasing the diameter of the wheels (but that would have the negative impact of racing the centre of gravity making the UAV more unstable and prone to tipping over whilst taking off) or by adding a separate wheel with magnets that is connected to the main wheels via a gear reduction so that the magnets pass by the hall effect sensor more slowly at high velocities.

**3. Premature Retraction**
A bump on a runway could cause the UAV to momentarily bounce off the support pads. If the limit switch opened while accelerating at \(10\text{ m/s}\), the ESP32 would interpret it as a take-off, collapsing the carriage and causing a high-speed prop strike while the drone was still on the cart, potentially damaging the UAV as well as the cart. This was accounted for in the firmware and steps were taken to mitigate this event. I implemented a triple-condition safety interlock in the state machine logic. The ESP32 strictly forbids servo actuation unless two conditions are met simultaneously:
  1. **Master Arm Switch:** The arm switch must be toggled ON. This also has the benefit of allowing you to setup the UAV on the cart without worrying that the cart might fold whilst on the bench
  2. **Weight-Off-Wheels Signal:** Limit switch state registers open.
  3. **Velocity Threshold Interlock:** Real-time Hall sensor speed readout confirms \(V_{\text{ground}} \ge 18\text{ m/s}\) alongside a 100ms debouncing window.

---

## Conclusion
This instrumented ground support trolley successfully bridges the gap between high-speed aerodynamic constraints and safe launch logistics. By combining embedded C++ firmware, Hall-effect sensing, and active mechanical clearance linkages, the system replaces high-risk hand launches with a repeatable, telemetry-monitored ground take-off platform.

Although designed around a specific 2.43 kg fixed-wing SAR airframe, the modular mechatronic architecture serves as a scalable foundation for future Ground Support Equipment across higher-weight unmanned systems.

---

├── Firmware/
│   ├── src/
│   │   ├── main.cpp          # Core loop, ISRs, and state machine
│   │   ├── web_server.cpp    # Wi-Fi AP & WebSocket telemetry streaming
│   │   └── sensors.cpp       # Hall effect speed calculations & switch filtering
│   └── platformio.ini        # PlatformIO environment configuration
├── CAD/
│   ├── Trolley_Assembly.step # Full mechanical cart assembly
│   ├── Linkage_Arm.stl       # 3D printable parallel linkage support
│   └── Wheel_Magnet_Hub.stl  # Custom wheel hub with embedded magnet recess
├── Hardware/
│   └── Wiring_Schematic.pdf  # Pinout diagrams for ESP32, servo, switches, and power
└── README.md
