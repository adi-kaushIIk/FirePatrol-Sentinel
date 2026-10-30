# 🔥 FirePatrolSentinel
### An Autonomous Fire and Smoke Detection Rover

<p align="center">
  <img src="https://raw.githubusercontent.com/adi-kaushIIk/FirePatrol-Sentinel/6064d2e978173a7ef8cdfd0c3c365ba709b346dd/banner.jpg" width="100%" alt="FirePatrolSentinel Banner">
</p>

> **Project Type:** Robotics & Embedded Systems  
> **Controller:** ESP32  
> **Programming Framework:** Arduino / PlatformIO  
> **Project Status:** Prototype / Educational Project  
> **Team:** Da Vinci Code

---

## 📌 BOT Description

**FirePatrolSentinel** is a smart, mobile fire and smoke detection rover designed to patrol a designated area, detect potential fire hazards, and alert people when dangerous conditions are identified.

The rover uses an ESP32 microcontroller along with multiple sensors, including an ultrasonic sensor, MQ-2 gas sensor, and flame sensor, to monitor its surroundings. It can move around an area, detect obstacles, monitor smoke levels, and identify the presence of flames.

When a potential hazard is detected, the system activates an audible warning through a buzzer. Sensor readings are also transmitted to a web-based dashboard for real-time monitoring.

The main objective of this project is to demonstrate how robotics, embedded systems, and sensor technology can work together to develop a mobile fire-monitoring system rather than using a traditional fire alarm.

---

## ✨ Key Features

- 🔥 **Fire Detection:** Identifies the presence of flames using a flame sensor.
- 💨 **Smoke Detection:** Uses the MQ-2 sensor to monitor smoke and combustible gas levels.
- 📡 **Obstacle Detection:** Measures the distance to nearby objects using an ultrasonic sensor.
- 🤖 **Autonomous Patrolling:** Moves around the designated area while monitoring its surroundings.
- 🔄 **Obstacle Avoidance:** Stops and changes direction when an obstacle is detected.
- 🚨 **Alert System:** Activates a buzzer when a potential fire or smoke hazard is detected.
- 📊 **Live Dashboard:** Transmits sensor readings and rover status to a web-based dashboard.
- ⚙️ **ESP32-Based Control:** Uses the ESP32 as the central processing and control unit.

---

## 👥 Team Information

### Team Name: Da Vinci Code

**Da Vinci Code** is inspired by the great thinker, inventor, and visionary **Leonardo da Vinci**.

His creativity, curiosity, and innovative approach to solving problems continue to inspire engineers, designers, and inventors around the world.

The team was established and is led by **Kaushik Paul**, who is responsible for the development and maintenance of the project.

### Team Member

| Name | Position |
|---|---|
| Kaushik Paul | Founder & Team Lead |

---

## 💻 Member Information

### Kaushik Paul
**Founder | Team Lead | Hardware & Software Developer**

📧 Email: akaushikpaul@gmail.com

**Individual Contributions:**

- Hardware design and assembly.
- 3D CAD modelling and mechanical design.
- Electronics integration and circuit connections.
- ESP32 programming and firmware development.
- Sensor integration and motor control.
- Web dashboard development and integration.
- GitHub repository management.
- Project documentation and maintenance.

---

## 🛠️ Hardware Components

| Component | Purpose |
|---|---|
| ESP32 Development Board | Main controller |
| L298N Motor Driver | Controls motor movement |
| 2 × DC Geared Motors | Drives the rover |
| HC-SR04 Ultrasonic Sensor | Obstacle detection |
| Servo Motor | Rotates the ultrasonic sensor |
| MQ-2 Gas Sensor | Smoke and gas detection |
| Flame Sensor | Flame detection |
| LM2596 Buck Converter | Voltage regulation |
| Buzzer | Audible warning |
| Battery | Power supply |
| Custom Chassis | Mechanical structure |

---

## ⚙️ How It Works

1. The ESP32 initializes the sensors, motor driver, and other connected components.
2. The rover starts moving and continuously monitors its surroundings.
3. The ultrasonic sensor measures the distance to obstacles ahead.
4. When an obstacle is detected, the rover stops, changes direction, and continues its patrol.
5. The MQ-2 and flame sensors monitor the environment for potential fire hazards.
6. If smoke or flame is detected, the warning system activates.
7. Sensor readings and system information are transmitted to the web dashboard.

---

## ⚙️ Motor Selection Justification

For FirePatrolSentinel, we selected **four DC geared motors** to drive the rover.

The selection was based on the movement requirements of the project, including controlled speed, sufficient torque, affordability, and compatibility with the L298N motor driver.

### Why These Motors?

1. **Adequate Torque:** The gearbox helps provide the turning force required to move the rover along with its chassis, battery, and electronic components.
2. **Controlled Movement:** Geared motors allow the rover to operate at a relatively low speed, which is useful for monitoring its surroundings.
3. **Four-Wheel Drive:** Using four motors provides driving force to all four wheels, helping the rover maintain traction.
4. **Directional Control:** With an H-bridge motor driver, the motors can operate in forward and reverse directions.
5. **Compatibility:** DC geared motors can be integrated with the L298N motor driver and controlled through the ESP32.
6. **Cost-Effectiveness:** They provide a practical and affordable solution for a student robotics project.

### Final Justification

We selected four DC geared motors because they offer a suitable combination of torque, controlled movement, traction, and affordability for our four-wheel robotic rover.

---

## 📸 Project Gallery

### Front View

![FirePatrolSentinel Front View](https://github.com/adi-kaushIIk/FirePatrol-Sentinel/blob/e97845e4902fea804e70b974bc665046e9be3ff5/rover-front.jpeg.jpeg)

### Top View

![FirePatrolSentinel Top View](https://github.com/adi-kaushIIk/FirePatrol-Sentinel/blob/e97845e4902fea804e70b974bc665046e9be3ff5/rover-top.jpeg.jpeg)

### Side Perspective

![FirePatrolSentinel Side View](https://github.com/adi-kaushIIk/FirePatrol-Sentinel/blob/e97845e4902fea804e70b974bc665046e9be3ff5/rover-side.jpeg.jpeg)

---

## 🎥 Project Demonstration

**[▶ Watch FirePatrolSentinel in Action on YouTube](https://youtube.com/shorts/h8ddNOI437M)**

---

## 🚀 Getting Started

### Software Requirements

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
- ESP32 board support.
- A modern web browser.

### Installation

**1. Clone the repository**

```bash
git clone https://github.com/adi-kaushIIk/FirePatrol-Sentinel.git
```

**2. Navigate to the project directory**

```bash
cd FirePatrol-Sentinel
```

**3. Open the project in Visual Studio Code**

```bash
code .
```

**4. Install dependencies**

Open the project using PlatformIO. Required libraries are specified in `platformio.ini`.

**5. Connect the ESP32**

Connect the ESP32 to your computer using a USB cable.

**6. Upload the firmware**

Use PlatformIO to build and upload the firmware to the ESP32.

**7. Open the Serial Monitor**

Set the baud rate to **115200** to view sensor readings and system information.

**8. Launch the dashboard**

Open the dashboard using its configured hosting method and establish communication with the rover.

---

## 📂 Project Structure

```text
FirePatrol-Sentinel/
│
├── src/
│   └── main.cpp
│
├── include/
├── lib/
├── test/
│
├── dashboard/
│   ├── index.html
│   ├── style.css
│   └── script.js
│
├── CAD/
│   ├── Fusion-Project/
│   ├── STEP/
│   └── Drawings/
│
├── assets/
│   ├── rover-front.jpeg
│   ├── rover-top.jpeg
│   └── rover-side.jpeg
│
├── diagrams/
│   ├── wiring-diagram.png
│   └── system-architecture.png
│
├── platformio.ini
└── README.md
```

---

## 🔮 Future Improvements

- Integration of a camera for visual monitoring.
- Improved autonomous navigation.
- More advanced obstacle detection.
- Improved sensor accuracy and calibration.
- GPS integration for outdoor monitoring.
- Enhanced wireless monitoring and remote operation.

---

## ⚠️ Safety Notice

FirePatrolSentinel is an educational robotics prototype. It is designed for experimentation and demonstration purposes and is not a certified fire detection or emergency response system.

---

## 📜 License

This project is intended for educational and learning purposes.

---

<p align="center">

# 🔥 FirePatrolSentinel

### Detect. Monitor. Alert.

*Inspired by innovation. Built with robotics.*

**Developed by Kaushik Paul**  
**Da Vinci Code**

</p>
