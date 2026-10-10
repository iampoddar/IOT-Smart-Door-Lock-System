# IOT-Smart-Door-Lock-System

## Project Overview

The Smart Door Lock System is an IoT-based access control project built around an **ESP32 microcontroller**, sensors, and electronic components to provide convenient door access and monitoring. By integrating the system with the Blynk IoT platform, users can remotely view sensor readings and control the door through a mobile application.

The system combines keypad-based password verification, servo-controlled locking, motion sensing, obstacle detection, and automatic lighting. It also incorporates a buzzer to alert users when an incorrect password is entered and a physical push button for manual door operation.

### Key Features

* **Password-Based Access:** Uses a 4×4 keypad to authenticate users before unlocking the door.
* **Remote Monitoring and Control:** Connects with the Blynk mobile application for monitoring sensor readings and managing the door lock.
* **Obstacle Detection:** Uses an ultrasonic sensor to measure the distance of nearby objects and help prevent locking when an obstacle is detected.
* **Motion Detection:** Employs a PIR sensor to identify movement near the keypad and update the motion status on the Blynk dashboard.
* **Automatic Lighting:** Uses an LDR sensor to measure ambient light and switches an LED on when the surroundings become sufficiently dark.
* **Unauthorized Access Alert:** Activates a buzzer when an incorrect password is submitted.
* **Physical Door Control:** Includes a push button for manually operating the door lock.

---

## Components Required

| Component                      | Function                                                                  |
| ------------------------------ | ------------------------------------------------------------------------- |
| ESP32 Development Board        | Processes inputs, controls connected devices, and communicates with Blynk |
| 4×4 Matrix Keypad              | Enables password entry and user authentication                            |
| Servo Motor                    | Operates the door-lock mechanism                                          |
| HC-SR04 Ultrasonic Sensor      | Measures distance to detect nearby obstacles                              |
| PIR Motion Sensor              | Detects movement around the keypad area                                   |
| LDR (Light-Dependent Resistor) | Measures ambient illumination                                             |
| LED                            | Provides lighting when the environment is dark                            |
| Buzzer                         | Produces an audible alert for an incorrect password                       |
| Push Button                    | Provides physical control of the door                                     |
| Blynk IoT Platform             | Supports remote monitoring and control through a smartphone               |

---

## Pin Configuration

The following table lists the GPIO connections between the ESP32 and the project's hardware components.

| Component                 | ESP32 GPIO Pin(s) |
| ------------------------- | ----------------- |
| Keypad Rows (R1–R4)       | 19, 18, 17, 16    |
| Keypad Columns (C1–C4)    | 23, 22, 21, 5     |
| Servo Motor Signal        | 33                |
| PIR Motion Sensor         | 13                |
| Ultrasonic Trigger (TRIG) | 25                |
| Ultrasonic Echo (ECHO)    | 26                |
| LDR Analog Output         | 34                |
| LED                       | 4                 |
| Buzzer                    | 2                 |
| Push Button               | 12                |

**Hardware note:** Ensure that the ultrasonic sensor's ECHO output is compatible with the ESP32's 3.3 V GPIO inputs. A suitable voltage divider or level shifter may be required. The servo should also have an appropriate power supply with a shared ground with the ESP32.

---

## Working Principle

### 1. Keypad-Based Authentication

The keypad provides the interface for entering the stored access password. Once the user submits the entered sequence, the ESP32 compares it with the configured password.

If the credentials match, the servo motor rotates to the configured unlocking position. If the password is incorrect, the buzzer sounds for approximately one second to indicate an unsuccessful authentication attempt.

### 2. Motion Monitoring Through the PIR Sensor

The PIR sensor continuously monitors the area near the keypad for movement. When motion is detected, the ESP32 updates the corresponding status on the Blynk dashboard. When movement is no longer detected, the dashboard reports the absence of motion.

This feature provides an additional way to monitor activity around the access point.

### 3. Ultrasonic-Based Obstacle Detection

The HC-SR04 ultrasonic sensor measures the distance between the sensor and nearby objects by transmitting an ultrasonic pulse and measuring the time taken for its echo to return.

Before locking the door, the system checks the measured distance against the configured safety threshold. If an obstacle is detected within the restricted range, the locking operation is prevented and the user receives a corresponding status update.

Invalid distance measurements should also be treated cautiously so that an absent echo is not incorrectly interpreted as a clear area.

### 4. Automatic Lighting Using an LDR

The LDR monitors ambient light intensity and provides an analog reading to the ESP32.

The controller compares this value with a predefined threshold:

* When the LDR reading falls below `200`, the LED switches on.
* When the reading is at or above `200`, the LED switches off.

This functionality provides basic automatic illumination based on the surrounding light conditions. The threshold can be adjusted according to the sensor arrangement and lighting requirements.

### 5. Manual Push-Button Operation

A physical push button allows the user to operate the door locally without relying exclusively on the keypad or mobile application.

Pressing the button toggles the manual control state, allowing the door to be unlocked or a locking operation to be requested. Before locking, the controller checks the ultrasonic sensor reading to determine whether the safety condition is satisfied.

### 6. Remote Access Through Blynk IoT

The Blynk platform connects the ESP32 to a mobile dashboard, allowing users to monitor the system's sensor data and request remote door operations.

The application provides visibility into the door status, motion detection, distance measurements, and ambient light readings. The manual control widget can also be used to request unlocking or locking remotely.

---

## Blynk Virtual Pin Configuration

The project uses five virtual pins to exchange data between the ESP32 and the Blynk application.

| Virtual Pin | Purpose                                           | Suggested Widget       |
| ----------- | ------------------------------------------------- | ---------------------- |
| `V1`        | Displays the ultrasonic sensor's distance reading | Gauge or Value Display |
| `V2`        | Shows whether motion is detected                  | Label or Value Display |
| `V3`        | Displays the current door-lock status             | Label                  |
| `V4`        | Shows the analog light-level reading from the LDR | Gauge or Value Display |
| `V5`        | Sends manual door-control commands                | Switch                 |

The required Blynk datastreams should be created with suitable data types and ranges. For example, `V1` can use an integer datastream for distance, while `V2` and `V3` can use string datastreams for their status messages. The manual control datastream on `V5` should use integer values of `0` and `1`.

---

## Installation and Setup

### Step 1: Prepare the Development Environment

Install the Arduino IDE and configure it for ESP32 development. Install the required libraries:

* `WiFi`
* `Blynk`
* `Keypad`
* `ESP32Servo`

Replace the placeholder Blynk authentication token with the token associated with your own device. Enter the appropriate Wi-Fi network name and password in the source code.

Save the program as an Arduino sketch, typically using the `.ino` extension, and select the correct ESP32 board and serial port before uploading.

### Step 2: Configure the Blynk Dashboard

Create a device and template in the Blynk platform for the Smart Door Lock System. Configure the necessary datastreams and add widgets for distance, motion status, lock status, ambient light level, and manual door control.

Verify that each widget is associated with the correct virtual pin and that its data type matches the values sent or received by the ESP32.

### Step 3: Connect the Hardware

Connect the keypad, servo, PIR sensor, ultrasonic sensor, LDR, LED, buzzer, and push button according to the pin configuration table.

Check the power requirements and logic-level compatibility of each component before powering the circuit. In particular, avoid powering a servo directly from an ESP32 GPIO pin, and ensure that the servo's power supply and ESP32 share a common ground.

### Step 4: Upload and Test the Program

Upload the program to the ESP32 and open the Serial Monitor to inspect system messages and sensor readings.

Perform the following checks:

1. Enter the correct password and verify that the servo unlocks the door.
2. Enter an incorrect password and confirm that the buzzer activates.
3. Trigger the PIR sensor and verify the motion status on the Blynk dashboard.
4. Place an object near the ultrasonic sensor and verify that the configured locking safety condition is enforced.
5. Vary the light level around the LDR and confirm that the LED responds to the threshold.
6. Test the physical push button and the Blynk manual-control switch.
7. Confirm that the dashboard displays the correct sensor readings and door status.

---

## Technologies Used

* **Microcontroller:** ESP32
* **Programming Language:** C++
* **Development Environment:** Arduino IDE
* **IoT Platform:** Blynk
* **Communication:** Wi-Fi
* **Sensors:** HC-SR04 ultrasonic sensor, PIR motion sensor, and LDR
* **Actuator:** Servo motor
* **Input Device:** 4×4 matrix keypad and push button
* **Output Devices:** LED and buzzer

---

## Potential Improvements

The project can be extended with additional capabilities, including:

* Notifications for repeated failed password attempts.
* Temporary lockout after multiple incorrect passwords.
* A configurable automatic-lock timer.
* A more secure password-storage mechanism.
* A battery backup and power-status indicator.
* Event logging for access attempts and motion detection.
* Improved sensor-failure handling and connectivity recovery.
* Additional user authentication methods, such as RFID or fingerprint recognition.

## Conclusion

The Smart Door Lock System demonstrates how an ESP32, multiple sensors, a servo motor, and an IoT platform can be combined to develop an interactive access-control application. Keypad authentication and physical controls support local operation, while Blynk enables remote monitoring and control. The ultrasonic sensor, PIR sensor, LDR, LED, and buzzer add obstacle detection, motion monitoring, automatic illumination, and audible feedback.

The project provides a practical foundation for learning embedded programming, sensor integration, actuator control, and IoT-based automation.

**Disclaimer:** This is an educational prototype. Obstacle detection and password verification alone do not constitute a complete security system. Before using the design for an actual entrance, additional fail-safe behavior, power-loss handling, secure communications, mechanical testing, and reliable locking hardware should be implemented and validated.
