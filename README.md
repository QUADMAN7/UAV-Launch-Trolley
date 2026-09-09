# IoT-Enabled UAV Launch Trolley (GSE)

A custom-designed Ground Support Equipment (GSE) cart built to solve the high-speed launch constraints ($V_{\text{launch}} \approx 18\text{ m/s}$) of a 2.43 kg AUW pusher-propeller Search and Rescue (SAR) fixed-wing aircraft. 

The system integrates real-time wheel speed telemetry via an ESP32 microcontroller with an automated, weight-triggered folding linkage mechanism to protect a 13" rear-mounted propeller during takeoff.

---

## Technical Specifications

| Subsystem | Component / Technology | Operational Role |
| :--- | :--- | :--- |
| **Control Unit** | ESP32 (Wi-Fi / Dual-Core) | Real-time sensor polling, telemetry hosting, servo actuation |
| **Velocity Sensing** | Neodymium Magnet + Hall Effect Sensor | Wheel RPM, instantaneous velocity, distance, acceleration |
| **Liftoff Detection** | Sub-pad Limit Switches | Instantaneous weight-off-wheels trigger |
| **Clearance System** | Servo-latch + Rubber-tension Parallel Linkage | Folds support arms flat to allow clearance for 13" pusher prop |
| **Telemetry Interface** | ESP32 Access Point + Embedded Web Server | Live Web UI streaming speed and state data over Wi-Fi |

---

## Key Mechatronic Features

### 1. Active Propeller Clearance Mechanism
Because the aircraft utilizes a large 13" rear-mounted pusher propeller, sitting flat on a traditional cart would result in a propeller strike as the aircraft accelerates past the cart during liftoff.

* **Support Carriage:** Parallel-motion linkage support arms hold the fuselage above the cart frame.
* **Weight-Off-Wheels Trigger:** Limit switches placed beneath the support pads detect when aerodynamic lift supports the aircraft's weight.
* **Rapid Retraction:** Upon weight removal, the ESP32 actuates a release servo. Rubber-band tension pulls the parallel linkages down, snapping the support arms completely flat before the spinning 13" prop passes over the cart.

### 2. Sensor Integration & Velocity Calculation
Groundspeed is measured using a latching Hall effect sensor triggered by a neodymium magnet embedded within one of the trolley wheels.

* **Frequency Counting:** Hardware interrupts track wheel revolution intervals ($\Delta t$).
* **Physics Calculations:** The onboard firmware continuously calculates:
  
  $$\text{Velocity } (v) = \frac{2 \pi r}{N \cdot \Delta t}$$

  *(Where $r$ is effective wheel radius and $N$ is the magnet count per revolution)*
* **Kinematics:** Integrates velocity over time to track overall runway displacement ($d$) and acceleration ($a$).

### 3. Wireless IoT Telemetry
The ESP32 operates as a standalone Wi-Fi Access Point (SoftAP) running a lightweight HTTP/WebSocket web server. Ground crews can connect any mobile device or laptop directly to the trolley to view real-time metrics during ground runs.

---

## Operational Flow

```text
[ Ground Acceleration Phase ]
  │  ├── Hall sensor measures wheel RPM -> ESP32 calculates velocity
  │  └── ESP32 streams live speed data to Web UI over Wi-Fi
  ▼
[ Liftoff Event (V_ground ≥ 18 m/s) ]
  │  ├── Aerodynamic lift removes 2.43 kg weight from cradle
  │  └── Under-pad limit switch opens (Weight-Off-Wheels signal)
  ▼
[ Active Clearance Trigger ]
  │  ├── ESP32 receives limit switch signal
  │  ├── Release servo disengages support arm latches
  │  └── Tension springs/rubber bands snap linkages flat
  ▼
[ Safe Separation ]
     └── 13" pusher prop passes cleanly over folded cart
