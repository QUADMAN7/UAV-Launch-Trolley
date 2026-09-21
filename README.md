# IoT-Enabled UAV Launch Trolley
<img width="4000" height="3000" alt="20260907_122211" src="https://github.com/user-attachments/assets/b2ab7ad9-4b26-42af-a88e-a00bbd395ad7" />

## Project Overview & Goal
This repository details the development of an automated ground support trolley engineered to launch gearless, fixed-wing Unmanned Aerial Vehicles (UAVs). The project originated from a critical operational constraint encountered on a custom aircraft I designed and manufactured in a previous project. Design trade-offs on that airframe resulted in higher wing loading than originally intended, requiring the 2.43 kg aircraft to achieve a high take-off airspeed of at least 18 m/s. Because the airframe utilizes a rear-pusher configuration spinning a large 13-inch propeller, manual hand launching was completely impossible due to the severe risk of propeller strikes to the operator and catastrophic low-speed stalls. To eliminate these operational risks without adding parasitic weight or landing gear to the airframe itself, I engineered an instrumented launch trolley capable of safely accelerating the UAV to speed and actively dropping away the moment aerodynamic lift is achieved.

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

## Design and Iterations
To establish the optimal launch method for the aircraft, I began by conducting a trade-off study across several concept architectures. I evaluated bungee systems, launch rails, catapults, and various airframe-mounted landing gear options, including both fixed and jettison-able assemblies. Ultimately, I eliminated catapults, rails, and onboard landing gear because they either imposed weight and drag penalties on the airframe—which directly inflated the required take-off velocity—or introduced high mechanical complexity with a elevated risk of damaging the aircraft during separation. Selecting an external ground support trolley provided the best systems-level solution: it added zero flight mass to the aircraft, offered reusable launch capabilities without airframe fatigue, and delivered a simple, robust footprint for field operations.

The cart chassis was designed to use easily machinable materials. A structural plywood plank acts as the base to which everything is mounted. Atop this base is a parallel-motion linkage engineered to drop the airframe support cradles instantly upon lift-off. To ensure the linkage folded flat with minimal mechanical resistance, I integrated precision ball bearings into all primary pivot joints to eliminate binding. The linkage is held in its elevated position by a servo-operated latch claw controlled by the ESP32. Upon receiving a release command, the servo disengages the claw, allowing a pre-tensioned rubber band to snap the mechanism flat. Using an elastomeric element provided a rapid, high-reliability restoring force without the weight, complexity, or power draw of active motorized retraction. To maximize stability during ground roll, the cradle pads were custom-designed as exact negative moulds of the aircraft's lower fuselage, ensuring a snug, load-distributing fit.

Beneath the main chassis, the running gear consists of dual axles supported by four low-friction bearing blocks. Sizing the wheels required a careful balance between ground clearance and roll stability: they needed to be large enough to roll over minor runway imperfections, yet compact enough to keep the overall centre of gravity low and prevent tipping instabilities during high-speed acceleration. During the initial design phase, I considered adding an active steering system linked directly to the aircraft’s rudder control channel, however, after analysing the failure modes and mass budget, I chose to omit ground steering. Eliminating the extra actuation hardware simplified the system architecture, saved critical weight, and ensured predictable, straight-line tracking driven purely by the aircraft's own thrust line.

---

## Electronics
To govern the real-time telemetry, safety interlocks, and mechanical release, the trolley utilizes a custom embedded system centred around an ESP32 microcontroller. Power management is handled by an onboard 9V battery paired with a DC-DC buck converter, stepping the voltage down to a regulated 5V rail to reliably power the microcontroller, sensors, and actuation hardware.

The sensing suite consists of two primary inputs designed to feed the onboard kinematics and state tracking logic. A Hall effect sensor tracks wheel rotations to calculate real-time ground speed, while a physical limit switch embedded beneath the support pads acts as a weight-off-wheels sensor to confirm whether the aircraft is actively sitting on the trolley or not. System actuation is intentionally streamlined to minimize power draw and weight, relying on a single high-torque analogue servo. When triggered by the ESP32's safety logic, this servo disengages the primary latching mechanism, allowing the parallel-motion linkage to collapse flat under elastomeric tension.

For field operation and ground crew safety, the human-machine interface incorporates a heavy-duty mechanical arming toggle switch, a visual status LED, and an audible piezo buzzer. Recognizing that bright outdoor sunlight and high-glare field environments can easily obscure visual LED feedback during launch operations, I integrated a buzzer that provides unambiguous audible confirmation when the trolley is armed. This dual-sensory confirmation guarantees that ground operators can definitively verify the system's state before initiating a high-speed take-off run.


---

## How It Was Built
The physical trolley was constructed using a hybrid material selection composed of 12mm structural plywood, 3D-printed PLA, flexible TPU, and lightweight aluminum tubing. A 12mm plywood board forms the central chassis, serving as a flat, rigid structural base to anchor all mechanical linkages, axle blocks, and electronic subassemblies.

The majority of the functional components—including the parallel-motion linkage arms, wheel hubs, and bearing housings—were additive-manufactured using PLA. While PETG offers higher impact resistance, desktop 3D printing constraints (specifically thermal warping on larger geometries) made PLA the more reliable choice for maintaining tight dimensional tolerances across the joint linkages. To complement the rigid PLA parts, flexible TPU was strategically applied where compliance and energy absorption were necessary. TPU was used for the tyres to dampen runway vibrations, for the soft bump-stop that catches the linkage when it snaps flat, and for the airframe support pads that conform to the aircraft's fuselage.

To span the support footprint of the aircraft without adding unnecessary rotating mass or printing oversized plastic parts, I used 8mm aluminium tubing for both the axles and the upper cradle support frame. Utilizing aluminium tubes provided a significantly stiffer, lighter, and simpler structural bridge than attempting to 3D print a massive PLA equivalent. The custom-moulded TPU support pads were permanently bonded to the 8mm aluminium bars using high-strength epoxy. The entire mechanical assembly is secured using M3 hardware, integrating heat-set brass inserts directly into the 3D-printed components to deliver robust, reusable threaded joints that withstand repeated operational loads.

Before final physical assembly, the embedded control hardware and sensor circuits were prototyped and validated on a breadboard. This allowed me to debug the hardware interrupts, refine the ESP32 safety logic, and verify the servo release timing in code prior to mounting the electronics into the trolley chassis.

---

## Key Mechatronic Features

### 1. Active Propeller Clearance Mechanism
Because the aircraft utilizes a large 13" rear-mounted pusher propeller, sitting flat on a traditional cart would result in a propeller strike as the aircraft accelerates past the cart during lift-off.

* **Support Carriage:** Parallel-motion linkage support arms hold the fuselage above the cart frame.
* **Weight-Off-Wheels Trigger:** A limit switch placed on the rear support pad detects when the aircraft begins to lift off.
* **Rapid Retraction:** Upon weight removal (and if the velocity is above 18m/s), the ESP32 actuates the release servo. Rubber-band tension pulls the parallel linkages down, snapping the support arms completely flat before the spinning 13" prop passes over the cart.

### 2. Sensor Integration & Velocity Calculation
Groundspeed is measured using a Hall effect sensor triggered by a neodymium magnet embedded within one of the trolley wheels.

* **Frequency Counting:** Hardware interrupts track wheel revolution intervals ($\Delta t$).
* **Physics Calculations:** The onboard firmware continuously calculates:
  
  $$\text{Velocity } (v) = \frac{2 \pi r}{\Delta t}$$

  *(Where 'r' is wheel radius)*
* **Kinematics:** ESP32 integrates velocity over time to track overall runway displacement ($d$) and differentiates for acceleration ($a$).

### 3. Wireless IoT Telemetry
The ESP32 operates as a standalone Wi-Fi Access Point running a lightweight HTTP/WebSocket web server. 
This allows me to connect a mobile device or laptop directly to the trolley to view real-time metrics during ground runs.

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
## Problems Encountered
**1. 13" Propeller Clearance:** The UAV this trolley was designed for has a pusher design and uses a 13" propeller which is problematic when the UAV needs to take-off because if it were supported by a box on wheels, the angle at which the UAV would have to climb away from the trolley to clear the propeller would be extreme and incredibly difficult to achieve. This left me with two options for the design of the trolley. The first was to design it in such a way that the take-off path of the UAV's propeller would be unobstructed by somehow holding onto the wings instead of the fuselage. The second was to increase the clearance between the trolley and the UAV at the point of take-off. I decided to go with the second option as it offered a solution that was in a smaller and, consequently, lighter package which has the benefit of being easier to transport and for the UAV to accelerate itself and the trolley more quickly (therefore using less runway). To come up with a system to increase the clearance between the UAV and the trolley support pads upon which the UAV sits, I researched different mechanical systems such as scissor lifts but decided to go with a parallel motion linkage as it was simpler and could quickly be actuated to go from its highest point to its lowest point.

**2. Sensor Saturation:** In my initial design of the wheel speed tracking system, I embedded eight evenly spaced neodymium magnets into the wheel hub to maximize velocity measurement resolution. During high-speed testing, however, I discovered that this high-density arrangement only functioned reliably at low speeds under 5 m/s. Beyond this threshold, the rapid rotational frequency overwhelmed the Hall effect sensor's frequency response. The sensor could no longer react quickly enough to detect the magnetic gaps between pulses, causing it to saturate and latch into a continuous detection state.

This physical saturation directly interacted with my firmware's safety logic. To prevent the system from holding a stale speed reading after coming to a complete stop, I had programmed a timeout mechanism that reset the calculated velocity to 0 m/s if no pulse interrupts were registered within a set time window. Consequently, when sensor saturation occurred at speed, the lack of toggling state transitions caused the code to interpret the continuous signal as a stopped cart, abruptly dropping the telemetry speed output to 0 m/s mid-run.

To resolve this sensor bandwidth issue, I modified the wheel hub to house only a single neodymium magnet, lowering the pulse frequency enough to accommodate target takeoff velocities up to 25 m/s. While this change successfully restored function for standard testing, a critical evaluation of the system suggests that at the higher end of the operational envelope—exceeding 15 to 18 m/s—the sensor may approach its hardware response limits once again due to the high RPM of the small wheels.

In evaluating future design iterations to eliminate this failure mode entirely, I analyzed two potential engineering solutions. The first option—increasing the main wheel diameter to lower the rotational RPM—was ruled out because a larger wheel elevates the cart's center of gravity, significantly increasing the risk of dynamic tip-over during high-speed acceleration runs. The superior engineering approach would be to integrate a secondary sensor wheel connected to the main axle via a mechanical gear reduction. This would allow the magnets to pass the Hall effect sensor at a lower, controlled frequency during high-speed runs while maintaining a low center of gravity and preserving full signal integrity across the entire operational envelope.

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
## Repository structure
```text
├── Media # Picture and renders of final product
├── Firmware
├── CAD/
│   ├── Trolley_Assembly.step # Full mechanical cart assembly
│   ├── Linkage_Arm.stl       # 3D printable parallel linkage support
│   └── Wheel_Magnet_Hub.stl  # Custom wheel hub with embedded magnet recess
└── README.md
```
