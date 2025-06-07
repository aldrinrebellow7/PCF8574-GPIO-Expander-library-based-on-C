# STM32 HAl - PCF8574 GPIO Expander Library (C-based)

This repository provides a C-based library for interfacing with the **PCF8574 I/O Expander** using the **I2C communication protocol**. It's designed to offer a straightforward way to add more digital input/output pins to your microcontroller projects.

---

## Overview

The PCF8574 is an 8-bit I/O expander for the I2C-bus. It features 8 quasi-bidirectional I/O pins, which can be configured as either inputs or outputs. This is extremely useful for microcontrollers with limited GPIOs, allowing you to control or read the state of up to 8 (or more, by cascading multiple PCF8574s) pins using just two wires (SDA and SCL) from your microcontroller's I2C bus. This library aims to abstract the I2C communication details, providing easy-to-use functions to read from and write to the PCF8574's pins.

---

## Features

* **Simple control of 8 digital I/O pins** per PCF8574 IC.
* Communicates via the **I2C (Two-Wire Interface)** protocol.
* Supports **reading input states** from the PCF8574.
* Supports **writing output states** to the PCF8574 pins.
* Designed as a **generic C library**, making it highly portable with a proper I2C abstraction layer.
* Handles multiple PCF8574 devices on the same bus (distinguished by their I2C addresses).

---

## Getting Started

### Prerequisites

To use this library, you will need:

* A C-compatible development environment (e.g., STM32CubeIDE, PlatformIO, VS Code with appropriate toolchains, or simply GCC/Clang for a specific MCU).
* A microcontroller development board (e.g., STM32, ESP32, AVR, PIC) with I2C hardware capabilities.
* A **PCF8574 I/O Expander** module or IC.
* A fundamental understanding of **C programming** and **embedded systems**, especially I2C communication.

### Hardware Connections

Connect your PCF8574(s) to your microcontroller's I2C bus:

* **SDA (Serial Data Line):** Connects to your MCU's SDA pin.
* **SCL (Serial Clock Line):** Connects to your MCU's SCL pin.
* **VCC & GND:** Connect to your power supply (typically 3.3V or 5V).
* **A0, A1, A2 (Address Pins):** These pins are used to set the I2C address of the PCF8574. Tie them to VCC or GND to select one of 8 possible addresses (e.g., all low `0x20`, A0 high `0x21`, etc. - check your PCF8574 datasheet for exact base address and address pin mapping, typically `0x20` to `0x27` or `0x38` to `0x3F`). Ensure appropriate pull-up resistors (e.g., 4.7kΩ) are used on SDA and SCL lines.

### Installation

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/aldrinrebellow7/PCF8574-GPIO-Expander-library-based-on-C.git](https://github.com/aldrinrebellow7/PCF8574-GPIO-Expander-library-based-on-C.git)
    ```
2.  **Integrate into your Project:**
    * Copy the library's source files (`PCF8574.c` and `PCF8574.h`) into your microcontroller project's source directories.
    * Ensure your project's build system (Makefile, CMakeLists.txt, or IDE settings) is configured to compile `PCF8574.c` and include the path to `PCF8574.h`.
    * Include the main header file in your application code:
        ```c
        #include "PCF8574.h"
        ```
    * **Crucially:** You will also need to provide implementations for the **I2C abstraction layer functions** (as discussed in the "Porting" section below) for your specific microcontroller.

---

### Usage Example

This example demonstrates how to initialize a PCF8574 and toggle an output pin (e.g., P0) and read an input pin (e.g., P7). This example assumes your microcontroller's I2C peripheral is already initialized.

```c
// Example: Toggle an LED on P0 and read a button on P7 of a PCF8574

#include "PCF8574.h"
#include "i2c_port.h" // Include your custom I2C porting layer header

// Define the I2C address of your PCF8574.
// For example, if A0=GND, A1=GND, A2=GND, the address might be 0x20.
// Consult your PCF8574 datasheet for the exact base address and A0/A1/A2 mapping.
#define PCF8574_ADDRESS 0x20 // Example address

// Define initial state for all 8 pins (0xFF means all high/inputs initially)
#define INITIAL_PIN_STATE 0xFF

int main(void)
{
    // 1. Initialize your microcontroller's I2C peripheral.
    // This part is specific to your MCU (e.g., HAL_I2C_Init() for STM32,
    // Wire.begin() for Arduino, etc.).
    // Make sure your I2C clock speed is set appropriately (e.g., 100kHz or 400kHz).
    I2C_PORT_Init(); // This is a placeholder for your actual I2C init.

    // 2. Initialize the PCF8574 library instance.
    // The library will store the I2C address and set initial pin states.
    PCF8574_Init(PCF8574_ADDRESS, INITIAL_PIN_STATE);

    uint8_t output_state = 0; // State for P0 (0 or 1)
    uint8_t input_value;      // To store the state of P7

    while (1)
    {
        // Toggle output pin P0
        PCF8574_WritePin(0, output_state); // Set P0 to current output_state
        output_state = !output_state;      // Toggle state for next iteration

        // Read input pin P7
        input_value = PCF8574_ReadPin(7); // Read the state of P7

        // Print or use input_value (e.g., if (input_value == 0) { ... })
        // For example, if P7 is connected to a button that pulls low when pressed:
        // if (input_value == 0) {
        //     // Button is pressed
        // } else {
        //     // Button is released
        // }

        // Delay for a short period (e.g., 500ms)
        I2C_PORT_Delay_ms(500); // Placeholder for your actual delay function
    }
}
