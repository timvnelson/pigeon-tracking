# Bird Loft & Release Cage Automation Project  
# AI System Prompt & Architecture: Bird Loft & Release Cage Automation  
  
### Non-Technical Summary  
This project builds an automated, smart management system for homing pigeons/birds split across two main locations: a stationary home loft and a mobile release cage.  
  
At the home loft, an intelligent controller automatically secures entrance doors at night, tracks individual birds returning via leg bands, regulates feeding schedules by opening and closing feeder covers, and watches for water spills. Out in the field, a mobile release cage monitors surrounding weather conditions, tracks its own location via GPS only when it senses it is being moved, and constantly verifies which birds are still inside using a floor grid of scanners.  
  
Both systems are powered by battery backup units so they never fail during a power outage. They communicate wirelessly back to a central home server, which collects all the data and displays an easy-to-read dashboard on your phone or computer.  

---

## 0. Confirmed Hardware Facts (verified by owner; these override anything below)  
Only the project owner adds facts here, after checking the real hardware. If another section disagrees with this one, this one is correct and the other section should be fixed.  

### Entrance doors: Ripom D3786 solar roll-up coop door (Amazon ASIN B0H41MYJFB)  
- **Self-contained unit.** Each door has its own motor, controller, LCD and internal 2500 mAh battery. The Arduino does **not** drive the door motors.  
- **Charging input:** 4.2-5.0 V DC at 1 A (from the door's manual). It was originally fed 5 V by the included solar panel. Rats kept chewing the panel cable, so the panels are being replaced by wired DC from the UPS.  
- **The power input is also the open/close signal.** In Light Sensitive mode the door uses that input as its light sensor: **power present = day = door opens; power removed = night = door closes.** So one relay per door does both jobs:  
    - **Relay ON:** the door charges and opens (daytime).  
    - **Relay OFF:** the door runs on its battery and closes (night).  
    - Each door must stay in **Light Sensitive mode** on its LCD, or this control method will not work.  
    - The Arduino replaces the sun: it switches each door's relay at sunrise and sunset, which it calculates offline.  
- **Power budget:** up to 1 A per door. All 5 doors powered during the day can draw up to 5 A, which is the UPS's whole 5 V 5 A rating before counting the Arduino and relay coils. Turn the doors on one at a time a few seconds apart rather than all at once. If the voltage sags, consider a separate supply for the doors.  
- **Boot and reset safety:** the ELEGOO relay inputs are active-LOW. Firmware must drive each relay pin HIGH (relay off) *before* setting it as OUTPUT. Otherwise a reset or power blip at night can briefly power the doors and make them start opening.  
- **Other built-in controls:** an RF remote (about 165 ft range) and manual push-buttons on the unit still work as manual overrides.  
- **Anti-pinch:** if the door meets resistance while closing, it retracts for 10 s and then tries again. Firmware must not treat a door that is still open shortly after power is removed as a fault until that retry window has passed.  
- **Opening:** 9.5" x 10". Operating range: -15°F to 140°F.  
- **Response time:** the door starts moving a couple of seconds after its power is switched on or off (owner confirmed).  
- **Still unknown (do not guess, ask the owner):**  
    - How much current each door actually draws when its battery is full compared with when it is charging.  

### Pin assignments (owner confirmed)  
| Arduino pin | Connects to | Purpose |  
|---|---|---|  
| D2 | Relay IN1 | Door 1 power |  
| D3 | Relay IN2 | Door 2 power |  
| D4 | Relay IN3 | Door 3 power |  
| D5 | Relay IN4 | Door 4 power |  
| D6 | Relay IN5 | Door 5 power |  

### Location  
Sunrise and sunset use 34.00 N, 118.00 W, Pacific time with US daylight saving. The coordinates are rounded on purpose because this repo is public. Never commit the owner's exact address, ZIP code or precise coordinates.  
  
  
## 1. Project Overview & Workflow  
- **Collaboration Model:** GitHub repository acts as the central memory bank.  
- **Key Files:** ==SYSTEM_PROMPT.md== (hardware specs, pin mappings, core logic) and ==ARCHITECTURE.md== (modular code map).  
- **Branching Strategy:** Use feature branches (e.g., ==feat/new-feature==) for AI exploration/sandboxing before merging into ==main==.  
- **Language:** C/C++ (via Arduino framework) for maximum execution speed, bare-metal efficiency, and optimal battery life (race-to-sleep).  

---
  
## 2. Bird Loft Arduino Project  
- **Microcontroller:** Arduino UNO R4 WiFi (32-bit Renesas RA4M1 + ESP32-S3 coprocessor, 5V GPIO logic, built-in RTC with VRTC coin-cell backup pin, USB-C, Qwiic/STEMMA QT connector).  
- **Entrance Doors:** 5 Ripom D3786 self-powered roll-up doors (see Section 0). One channel of the 8-channel 5V relay module per door switches the UPS 5V supply to that door's power input. Power on opens the door and charges it; power off closes it. The Arduino turns the relays on at sunrise and off at sunset using offline astronomical calculations (==SolarCalculator== / ==Dusk2Dawn== libraries).  
- **RFID Tracking:** Multiple RC522 RFID readers (13.56 MHz, SPI communication, 3.3V VCC, 2-5 cm range) arranged in an array across a 3D-printed waterproof strip. Uses individual Slave Select (SS) pins to identify exact door/sensor positions.  
- **Feeder Control:** Stepper motors controlled via driver modules (e.g., A4988/DRV8825) to regulate feeding access.  
- **Water Leak Sensors:** Integrated to detect overflow.  
- **Power System:**  
    - UPS Module 3S (3x 18650 batteries, 12.6V 2A input charger). Shared with the Release Cage Arduino.  
    - Outputs 5V 5A continuous via the main Type-C/XH2.54 port.  
    - Wiring: 18 AWG stranded pure copper wire. Ground runs to a distribution block; Positive runs to a distribution block, then through relay COM/NO terminals out to each door's power input (4.2-5.0 V, 1 A).  
- **Connectivity & Home Server:** MQTT protocol over Wi-Fi (or future Ethernet shield like W5500) back to the home server (Home Assistant with the Mosquitto MQTT broker; see Section 4).  

---
  
## 3. Release Cage Arduino Project  
- **Power System:** Runs on the same UPS Module 3S as the Loft Arduino.  
- **Environmental Sensors:** BME280 (Temperature, Humidity, Barometric Pressure/Altimeter) and BH1750 (Sun brightness/Lux). Both share I2C lines (SDA/SCL) with ultra-low power draw.  
- **Motion & GPS:** MPU6050 or ADXL345 accelerometer wired to a hardware interrupt pin. Wakes up the NEO-6M GPS module only when physical movement is detected to conserve battery.  
- **Connectivity:** Cellular module (e.g., SIM800L/SIM7600) when away from home (kept in continuous idle/network standby during daytime 1-minute updates, deep sleep PSM at night); switches to Wi-Fi when at home.  
- **RFID Verification:** Array of RC522 modules spanning the 32" x 17" bottom of the release box to verify bird presence.  
- **Offline Storage:** MicroSD card module logs data during connectivity drops; clears data only after receiving an HTTP 200 OK / MQTT confirmation handshake.  

---
  
## Firmware: what the code does today  
Update this section on every feature branch so that once merged it describes what `main` does.  

### Loft (`loft/`, Arduino UNO R4 WiFi)  
**Door schedule.** At sunrise the relays power the doors, which open. At 30 minutes after sunset they cut the power, and the doors close.  
- `loft.ino`: the main loop, the USB serial test commands and the status printout.  
- `config.h`: every setting (location, pins, open/close offsets, stagger delay, NTP intervals).  
- `SolarSchedule.*`: offline NOAA sunrise/sunset maths (no libraries) and the stateless "should the doors be open now?" check. It works out the right state from scratch after any reboot.  
- `DoorController.*`: relay switching. Pins are set to OFF first at boot. Doors open one at a time, 5 s apart; they close all at once.  
- `TimeKeeper.*`: UTC clock in the R4's built-in RTC, synced from NTP over Wi-Fi at boot, then every 6 h (or every 5 min after a failed attempt).  
- `secrets.example.h`: copy it to `secrets.h` (git-ignored) and fill in the Wi-Fi details.  

**Wi-Fi today:** the board connects at boot to fetch the time and stays connected. A dropped connection is only noticed at the next time sync (up to 6 h). That's fine for the doors, which run from the RTC and never need Wi-Fi.  
**Before the MQTT feature (planned `feat/loft-mqtt`):** add a Wi-Fi watchdog that checks the connection every ~10 s and reconnects with back-off without stalling the loop (`WiFi.begin()` blocks for ~10 s, long enough to miss an RFID read). Use `ArduinoMqttClient` with a keep-alive and Last Will so Home Assistant notices when the loft goes offline, and queue events while offline.  

**Rules the code follows:**  
- All scheduling happens in UTC, so daylight saving cannot break it. Local time is used only for printing.  
- If the clock has never been set, the doors stay **closed**.  
- USB serial commands (115200 baud): `status`, `open`, `close`, `auto`, `settime <utc-epoch>`, `sync`. An `open`/`close` override lasts until the next scheduled sunrise or sunset change, so it can't be left on by accident.  
- No third-party libraries. It needs only the "Arduino UNO R4 Boards" core, which includes `RTC.h` and `WiFiS3.h`.  

---

## 4. Home Server  
- **Platform (decided): Home Assistant**, chosen because it integrates with a wide range of other systems and devices. The loft and cage data can sit alongside other smart-home equipment, cameras and notifications, and automations can link them.  
- **Broker:** the Mosquitto MQTT broker, run as a Home Assistant add-on.  
- **Device setup: MQTT Discovery.** The Arduinos publish Home Assistant discovery messages so their doors and sensors appear as entities automatically, with no manual YAML.  
- **Hosting:** Home Assistant should run on its own dedicated machine, booting from an SSD rather than an SD card, because its database writes wear SD cards out.  
- **Optional later:** Node-RED, as a Home Assistant add-on, if visual flow-based automations are wanted.  
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
  
  
