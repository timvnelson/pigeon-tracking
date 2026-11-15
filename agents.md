# Bird Loft & Release Cage Automation Project  
# AI System Prompt & Architecture: Bird Loft & Release Cage Automation  
  
### Non-Technical Summary  
This project builds an automated, smart management system for homing pigeons/birds split across two main locations: a stationary home loft and a mobile release cage.  
  
At the home loft, an intelligent controller automatically secures entrance doors at night, tracks individual birds returning via leg bands, regulates feeding schedules by opening and closing feeder covers, and watches for water spills. Out in the field, a mobile release cage monitors surrounding weather conditions, tracks its own location via GPS only when it senses it is being moved, and constantly verifies which birds are still inside using a floor grid of scanners.  
  
Both systems are powered by battery backup units so they never fail during a power outage. They communicate wirelessly back to a central home server, which collects all the data and displays an easy-to-read dashboard on your phone or computer.  
  
  
## 1. Project Overview & Workflow  
- **Collaboration Model:** GitHub repository acts as the central memory bank.  
- **Key Files:** ==SYSTEM_PROMPT.md== (hardware specs, pin mappings, core logic) and ==ARCHITECTURE.md== (modular code map).  
- **Branching Strategy:** Use feature branches (e.g., ==feat/new-feature==) for AI exploration/sandboxing before merging into ==main==.  
- **Language:** C/C++ (via Arduino framework) for maximum execution speed, bare-metal efficiency, and optimal battery life (race-to-sleep).  

---
  
## 2. Bird Loft Arduino Project  
- **Microcontroller:** Arduino UNO R4 WiFi (32-bit Renesas RA4M1 + ESP32-S3 coprocessor, 5V GPIO logic, built-in RTC with VRTC coin-cell backup pin, USB-C, Qwiic/STEMMA QT connector).  
- **Entrance Doors:** 5 doors (5V, 1A each). Controlled via an 8-channel 5V relay module. Closed at night based on offline astronomical sunset calculations (==SolarCalculator== / ==Dusk2Dawn== libraries).  
- **RFID Tracking:** Multiple RC522 RFID readers (13.56 MHz, SPI communication, 3.3V VCC, 2-5 cm range) arranged in an array across a 3D-printed waterproof strip. Uses individual Slave Select (SS) pins to identify exact door/sensor positions.  
- **Feeder Control:** Stepper motors controlled via driver modules (e.g., A4988/DRV8825) to regulate feeding access.  
- **Water Leak Sensors:** Integrated to detect overflow.  
- **Power System:**  
    - UPS Module 3S (3x 18650 batteries, 12.6V 2A input charger). Shared with the Release Cage Arduino.  
    - Outputs 5V 5A continuous via the main Type-C/XH2.54 port.  
    - Wiring: 18 AWG stranded pure copper wire. Ground runs to a distribution block; Positive runs to a distribution block, then through relay COM/NO terminals out to doors.  
- **Connectivity & Home Server:** MQTT protocol over Wi-Fi (or future Ethernet shield like W5500) back to a home server (Home Assistant / Node-RED with Mosquitto broker).  

---
  
## 3. Release Cage Arduino Project  
- **Power System:** Runs on the same UPS Module 3S as the Loft Arduino.  
- **Environmental Sensors:** BME280 (Temperature, Humidity, Barometric Pressure/Altimeter) and BH1750 (Sun brightness/Lux). Both share I2C lines (SDA/SCL) with ultra-low power draw.  
- **Motion & GPS:** MPU6050 or ADXL345 accelerometer wired to a hardware interrupt pin. Wakes up the NEO-6M GPS module only when physical movement is detected to conserve battery.  
- **Connectivity:** Cellular module (e.g., SIM800L/SIM7600) when away from home (kept in continuous idle/network standby during daytime 1-minute updates, deep sleep PSM at night); switches to Wi-Fi when at home.  
- **RFID Verification:** Array of RC522 modules spanning the 32" x 17" bottom of the release box to verify bird presence.  
- **Offline Storage:** MicroSD card module logs data during connectivity drops; clears data only after receiving an HTTP 200 OK / MQTT confirmation handshake.  

---
  
## 4. Home Server  
- **Dashboard & Broker:** Runs Home Assistant or Node-RED with a Mosquitto MQTT broker.  
- **Data Storage:** Centralizes and stores all sensor telemetry, bird tracking logs, and system states from both the Loft and Release Cage Arduinos.  
  
  
### These are the products purchased so far:  
[Arduino UNO R4 WiFi [ABX00087] - Renesas RA4M1 + ESP32-S3, Wi-Fi, Bluetooth, USB-C, CAN, 12-bit DAC, OP AMP, Qwiic Connector, 12x8 LED Matrix for Advanced IoT & Embedded Projects](https://www.amazon.com/dp/B0C8V88Z9D?ref=fed_share)  
[Pure Copper 18 AWG Red & Black Bonded Zip Wire Automotive Cable – 100 ft | 100% OFC Double Conductor Duplex for Car Audio Amplifier Speaker Dashboard Harness LED Light Trailer Hookup Primary Wiring](https://www.amazon.com/dp/B0CYFQM79X?ref=fed_share)  
[MILAPEAK (Real 18AWG) 10 Pair 12V 5A DC Pigtail Barrel Plug Connector Cable | 2.1mm x 5.5mm, Male & Female DC Power Pigtail Connectors, for CCTV Security Camera, 12V Power Supply Adapter, LED, Router](https://www.amazon.com/dp/B072BXB2Y8?ref=fed_share)  
[APIELE 15A Screw Terminal Block 4/5/6 Position Dual Row Covered 600V Pre-Insulated Terminal Barrier Strips (6PCS 15A 4P/5P/6P+6PCS Wiring Strips)](https://www.amazon.com/dp/B0D7VRSH1G?ref=fed_share)  
[Double Wire Connector Press the Broken Wire with Pliers | Low Voltage Wire for 16-20AWG,2 Pin Quick Disconnect Electrical Secure Connectors with Locking Buckle](https://www.amazon.com/dp/B0H9YN2RV3?ref=fed_share)  
[ELEGOO 8 Channel DC 5V Relay Module with Optocoupler for Arduino Projects | Compatible with Arduino UNO R3 MEGA 1280 DSP ARM PIC AVR STM32, Active-low Inputs, NO/NC Terminals, Status LEDs](https://www.amazon.com/dp/B09ZQRLD95?ref=fed_share)  
[LAFVIN Uninterruptible Power Supply (UPS) Module for Raspberry Pi/Jetson Nano Supports Charging and Power Output at The Same Time 3S & 5V 5A Output](https://www.amazon.com/dp/B0G2SJPWLR?ref=fed_share)  
￼  
  
  
