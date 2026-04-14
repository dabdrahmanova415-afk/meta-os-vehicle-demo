# Meta-OS for Vehicle Demonstrator
Meta-Operating System for vehicle demonstrator on Arduino Uno with FreeRTOS, encoder, servo, and CSV logging.

## Overview
This project implements a lightweight **Meta-Operating System** on an Arduino Uno using FreeRTOS. It demonstrates:
- Task isolation (critical vs non‑critical)
- Dynamic mode switching (NORMAL, ECO, SAFE)
- Brake priority and fault recovery
- Energy‑saving ECO mode (suspends non‑critical tasks)
- CSV data logging for analysis

## Hardware Required
- Arduino Uno
- Rotary encoder (KY‑040 or similar)
- Tactile button
- SG90 servo motor
- 3 LEDs (green, yellow, red)
- 3 resistors 220Ω
- Breadboard and jumper wires

## Wiring Diagram
| Component | Arduino Pin |
|-----------|-------------|
| Encoder CLK | 11 |
| Encoder DT | 12 |
| Encoder GND | GND |
| Encoder VCC | 5V |
| Brake button | 2 and GND |
| Green LED | 6 (with 220Ω to GND) |
| Yellow LED | 7 (with 220Ω to GND) |
| Red LED | 8 (with 220Ω to GND) |
| Servo signal | 9 |
| Servo VCC | 5V |
| Servo GND | GND |

## Installation
1. Install Arduino IDE
2. Install libraries:
   - Arduino_FreeRTOS (by Phillip Stevens)
   - Encoder (by Paul Stoffregen)
   - Servo (built‑in)
3. Open `meta_os_demo.ino`
4. Select board: Arduino Uno
5. Upload

## Usage
- Turn the encoder → servo moves (throttle simulation)
- Press the button → SAFE mode (servo stops, red LED)
- After ~80 seconds → ECO mode (yellow LED, servo limited to 90°, non‑critical task suspended)
- Release button → return to NORMAL (or ECO if battery low)

## Data Logging
Open Serial Monitor (115200 baud). CSV output:
