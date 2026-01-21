# Tetris on LandTiger (NXP LPC1768)

A full-featured, bare-metal implementation of the classic Tetris game, developed for the **LandTiger** development board (NXP LPC1768 ARM Cortex-M3).

This project was created for extrapoints for the course Computer Architectures.

Created for extra points in the *Computer Architectures* course @ Politecnico di Torino.

## 🎮 Features

### Core Mechanics

* **Classic Gameplay**: 7 Tetromino shapes with standard rotation and collision logic.
* **Scoring System**: Points awarded for lines cleared and hard drops. High score tracking.
* **Levels/Speed**: Gravity increases as the game progresses or via potentiometer control.
* **Pause/Resume**: Toggle game state at any time.

### Advanced Mechanics (Power-ups & Malus)

* **⚠️ Random Malus**: If clearing lines too fast (or randomly triggered), the game shifts the board up and inserts a "garbage line" with random holes at the bottom.
* **❄️ Slow Down**: A power-up block (Cyan/Diamond) that, when cleared, slows down gravity for 15 seconds.
* **💥 Half Clear**: A power-up block (Red/Cross) that instantly removes the bottom 50% of the filled lines.

## 🛠 Hardware & Requirements

* **Board**: LandTiger (NXP LPC1768 - ARM Cortex-M3)
* **Display**: 3.2 inch TFT LCD (320x240)
* **Input**: 
  * 5-way Joystick (GPIO/RIT)
  * User Buttons (EINT0, EINT1, EINT2)
  * Potentiometer (ADC)
* **Audio**: Speaker connected to DAC/Timer.
* **IDE**: Keil µVision 5.

## 🕹️ Controls

| Input              | Action                                 |
|:------------------ |:-------------------------------------- |
| **Joystick LEFT**  | Move Piece Left                        |
| **Joystick RIGHT** | Move Piece Right                       |
| **Joystick UP**    | Rotate Piece                           |
| **Joystick DOWN**  | Soft Drop (Accelerate Fall)            |
| **KEY 1**          | **Pause** / **Restart** (if Game Over) |
| **KEY 2**          | **Hard Drop** (Instant Fall & Lock)    |
| **Potentiometer**  | Adjust Base Speed                      |

## 🏗️ Technical Architecture

### Interrupts & Scheduling

* **RIT (Repetitive Interrupt Timer)**: Fires every 50ms. It handles:
  * Input polling (Joystick & Button debouncing).
  * Music sequencer (plays notes based on game state).
  * Power-up timers (Slow down duration, Message timeout).
* **Timer0**: Handles the game gravity (falling pieces). Its interval (`MR0`) changes dynamically based on game speed or Slow Down power-up.
* **NVIC (Nested Vector Interrupt Controller)**: Manages priorities. Critical sections (like spawning a new piece or Hard Dropping) are protected by disabling specific interrupts to prevent **Race Conditions** and "Ghost Blocks".

## 🚀 Installation & Setup

1. Clone this repository.
2. Open the project file (`.uvprojx`) in **Keil µVision**.
3. Ensure the "LandTiger" target is selected.
4. Compile (Build) the project.
5. Connect the board via USB (J-Link/CMSIS-DAP).
6. Flash (Download) the code to the LPC1768.
7. Press the **Reset** button on the board.
8. Press **KEY 1** to start the game!

## 👨‍💻 Author

**Mirko Tenore**

* Project for Computer Architectures Course.
* Accademic Year: 2026

---

*Disclaimer: This project is for educational purposes.*
