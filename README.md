WOKWI Link = https://wokwi.com/projects/479514269985624065


# Arduino Traffic Light Controller

A beginner Arduino project that simulates a traffic light system with a 7-segment countdown display and a switch-controlled Night Mode.

## Features

- Four-phase traffic light sequence:
  - Red: 5 seconds
  - Red + Yellow: 3 seconds
  - Yellow: 3 seconds
  - Green: 5 seconds
- Synchronized blinking of red and yellow LEDs.
- 7-segment display countdown.
- Switch-controlled Night Mode.
- Yellow LED blinks continuously in Night Mode.
- Uses `millis()` for non-blocking timing instead of `delay()`.

## Components

- Arduino-compatible development board
- 1 common-cathode 7-segment display
- 1 red LED
- 1 yellow LED
- 1 green LED
- 1 switch
- Appropriate current-limiting resistors
- Breadboard and jumper wires

## Pin Connections

| Component | Arduino Pin |
|---|---|
| 7-segment segments a–g | D2–D8 |
| Red LED | D9 |
| Yellow LED | D10 |
| Green LED | D11 |
| Night Mode switch | D12 |

The switch uses the Arduino's internal `INPUT_PULLUP` resistor. In the current code, a HIGH reading activates Night Mode.

## How It Works

The controller cycles through four traffic light phases. A 7-segment display shows the remaining time for each phase.

When Night Mode is activated, the normal traffic light sequence stops, the display turns off, and only the yellow LED blinks.

When Night Mode is deactivated, the normal sequence restarts from the red phase.

Timing is handled using Arduino's `millis()` function, allowing the program to check the switch without blocking execution with `delay()`.

## Getting Started

1. Connect the components according to the pin connection table.
2. Open `TrafficLight.ino` in the Arduino IDE.
3. Select your Arduino board and the correct port.
4. Upload the sketch.
5. Power the circuit and test the traffic light sequence and Night Mode.

## Project Images

Add photos of your circuit and completed project in the `Images` folder.

## Learning Objectives

This project was built to practice:

- C/C++ programming for Arduino
- Arrays and functions
- Conditional statements and `switch`
- State-based program flow
- Digital input and output
- Non-blocking timing with `millis()`

## Mahdi Chavoshan

Created as a beginner embedded systems learning project.
