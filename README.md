# Prism: Interactive Audio-Visual Instrument

**Authors:** Cristian Embleton & Alex Volkov
**Course:** EEC 172 Final Project - University of California, Davis 

## Overview
Prism is a cheap, lightweight device that syncs the musical beats in a room to LED lights. Adding "sight" to sound, Prism makes the music in a room feel more visceral by integrating real-time Digital Signal Processing (DSP), physical interactivity, and dynamic cloud profiles.

## Features
* **Real-time Audio Processing:** Captures analog audio, converts it via an ADC, and runs it through a Fast Fourier Transform (FFT), DSP Filtering, and Frequency Binning.
* **Physical Interactivity:** Uses an on-board accelerometer to capture physical orientation data to dynamically modify the visual math (such as hue, EQ, and wave velocity) in real-time.
* **Over-The-Air (OTA) Configuration:** Interfaces with an AWS Endpoint to save and load custom visualizer sequences without reflashing the firmware[: 61, 62].
* **Dual Display Output:** Drives both a 1.5" Adafruit OLED screen and a 5-meter SK9822 LED strip via SPI.

## Visualizer Modes
Prism uses simple core components to visualize the different dimensions of a song. An IR Receiver utilizes a GPIO interrupt handler for discrete mode switching between the following modes:
1. **Beat Pulse Mode**
2. **Spectrum Analyzer Mode**
3. **Wave Mode**

## Hardware Architecture
The Prism embedded system is powered by a **CC3200 microcontroller**, which acts as the CPU for audio analysis and display generation.

### Bill of Materials
* **Microcontroller:** CC3200-LAUNCHXL LaunchPad 
* **Audio Input:** Adafruit Analog Microphone 
* **Displays:** Adafruit OLED Breakout Board (16-bit Color 1.5") & SK9822 LED strip (5-meters) 
* **Sensors:** On-board BMA222 Accelerometer & IR Receiver
* **Debugging Tools:** Saleae USB Logic Analyzer 

### Hardware Protocols
* **SPI:** A synchronous serial communication interface used for driving high-speed data to the OLED display and the SK9822 LED Strip
* **I2C:** A synchronous serial communication bus used to read 3-axis motion data from the accelerometer

## Software & Cloud Architecture
Prism is developed using **Code Composer Studio (CCS)** and relies on the **CC3200 SDK** (Driverlib: GPIO, SPI, I2C, SysTick)

### The AWS Cloud Hub
Beyond local processing, the central control unit interfaces with the cloud via an AWS Endpoint using HTTP/REST protocols.
* **GET Requests:** The device executes GET requests to fetch JSON payloads, updating internal variables to change active modes, color palettes, or audio thresholds on the fly.
* **POST Requests:** Users can execute POST requests via the IR remote to capture their current interactive state and permanently save it to the cloud as a new custom pattern.
