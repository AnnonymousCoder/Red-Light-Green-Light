# Squid Game Project
## Introduction
This project is inspired by the popular series "Squid Game." It is a simulation of the Red Light Green Light game. The goal of this project is to recreate that game with the use of arduino mega with a couple of sensors and actuators.

## Hardware
- Arduino Mega2560 R3
- 1602 LCD
- HC-SR04 Ultrasonic Sensor
- SG90 9g Servo (2)
- 16R Passive Buzzer
- 5mm Green LED
- 5mm Red LED
- 6x6x5mm Push Button

## Installation
1. In terminal Create a new directory and Clone the repository there:
    ```bash
    mkdir [folder_name]
    cd [folder_name]
    git clone https://github.com/AnnonymousCoder/Red-Light-Green-Light.git
    ```

## Usage
### VSCode
- This project was made through the [PlatformIO IDE extension](https://platformio.org/platformio-ide, 'PlatformIO extension site link') in VSCode.
    * [PlatformIO](https://platformio.org/, 'PlatformIO site') streamlines the process of building, uploading, and debugging code for various microcontrollers and embedded boards such as Arduino Mega and many more.

1. Open a new window in VSCode and download the Platform IDE extension. Once downloaded a shortcut to PlatformIO IDE will appear at the primary side bar on the left.
2. Click the extension shortcut, locate the Quick Access tab and click "Open" under PIO Home. PIO Home window should open.
3. Under Quick Access click open Project and locate the folder where the repo was cloned.
![image of VSCode with the PlatformIO extensions](./Resources/pio_home.png, 'PIO Home Window')

4. Use the checkmark and right arrow at the bottom status bar in VSCode to build and upload the code respectively.
![image of the bottom VSCode status bar](./Resources/status_bar.png, 'Bottom Status Bar')