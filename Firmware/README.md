# XIAO nRF52840 3-Layer Macropad Firmware

## Pin Allocation (From Schematic)
- **Rows:** R1 -> D0, R2 -> D1, R3 -> D2
- **Columns:** C1 -> D3, C2 -> D6, C3 -> D8
- **I2C Display:** SDA -> D4, SCL -> D5
- **Encoder:** A1 -> D9, B1 -> D10
- **LED Indicator:** LED -> D5 (Shared pin configuration)

## Features
1. **Boot Mode Condition:** Hold down Key 1 (Row 1, Column 1) during startup to enter **Input Device Mode** (USB HID Keyboard). Powering on without holding the key sets it to **Output Device Mode**.
2. **Layer Switching:** Turn the rotary encoder to cycle seamlessly through 3 structural key layers.
3. **OLED Status:** Displays the current operating mode and active layer in real-time.
