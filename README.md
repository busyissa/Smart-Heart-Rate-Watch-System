# Heart Rate Smart Watch System

![POTS BPM Monitor](Photos/Welcome%20to%20POTS%20BPM%20Monitor!%20(1).png)

A wearable heart rate monitoring system built on an ATmega328P that detects pulse signals, calculates BPM, displays results on an OLED screen, and alerts the user to potential Postural Orthostatic Tachycardia Syndrome (POTS) episodes.

## Features

- **Real-time heart rate monitoring** using an analog pulse sensor and ADC
- **BPM calculation** over 10-second sampling windows (beats counted x 6)
- **OLED display** (SSD1306 128x64) showing a startup bitmap, age selection menu, and live BPM readout
- **POTS detection** with configurable thresholds based on age group:
  - **Adult (20+):** alerts if BPM changes by 30 or more between readings
  - **Teen (<20):** alerts if BPM changes by 40 or more between readings
- **Piezo buzzer** plays a 1kHz alert tone for 10 seconds when POTS threshold is exceeded
- **Flashing LED** accompanies the buzzer during alerts
- **Two-button navigation** for menu interaction and age group selection
- **UART serial output** for debugging ADC values and BPM readings

## Hardware

### System Diagram

![System Diagram](Photos/Screenshot%202026-04-25%20142822.png)

### Components

| Component | Description |
|---|---|
| ATmega328P | Microcontroller (Arduino Uno) |
| SSD1306 OLED | 128x64 I2C display |
| Pulse sensor | Analog heart rate sensor |
| Piezo buzzer | Passive buzzer for alerts |
| LED + 330 ohm resistor | Visual alert indicator |
| 2x Push buttons | Menu navigation (internal pull-ups) |

### Pinout

| Arduino Pin | AVR Pin | Function |
|---|---|---|
| A0 | PC0 (ADC0) | Heart rate sensor data |
| A4 | PC4 (SDA) | OLED I2C data |
| A5 | PC5 (SCL) | OLED I2C clock |
| D0 | PD0 (RX) | UART RX (serial monitor) |
| D1 | PD1 (TX) | UART TX (serial monitor) |
| D2 | PD2 | Left button - YES/select |
| D3 | PD3 | Right button - NO/select |
| D8 | PB0 | Piezo buzzer |
| D9 | PB1 | Alert LED |

### Circuit

![Full breadboard circuit](Photos/Screenshot%202026-04-25%20142557.png)

## How It Works

### User Flow

1. **Startup:** A custom bitmap splash screen is displayed on the OLED
2. **Age selection:** Press either button to see "Are you 20 years or older?"
   - Press the **left button (D2)** for YES (Adult)
   - Press the **right button (D3)** for NO (Teen)
3. **Monitoring:** The system begins measuring heart rate and updates the OLED every 10 seconds with the current BPM

![Pulse sensor on finger](Photos/Screenshot%202026-04-25%20142613.png)

![OLED displaying Teen mode with BPM reading](Photos/Screenshot%202026-04-25%20142628.png)

4. **POTS alert:** If the BPM changes by more than the threshold between consecutive readings, the system:
   - Displays "ALERT: SIT DOWN" on the OLED
   - Sounds the piezo buzzer at ~1kHz
   - Flashes the LED on/off in 0.5-second intervals
   - Alert lasts 10 seconds, then returns to the BPM display

![POTS alert with LED and buzzer active](Photos/Screenshot%202026-04-25%20142639.png)

5. **Reset:** Press either button on the BPM screen to return to the startup bitmap

### UART Serial Output

ADC values and BPM readings are printed to the serial monitor for debugging.

![UART serial output showing ADC values](Photos/Screenshot%202026-04-25%20142652.png)

### Heart Rate Detection

The pulse sensor outputs an analog voltage that rises above a threshold (`VOLTTHRESHOLD = 600`) on each heartbeat. The system counts rising edges over a 10-second window using a Timer1 interrupt, then multiplies by 6 to get BPM.

## Project Structure

```
main.c          Merged application source (OLED + heart sensor + menu + alerts)
heartsensor.c   Original heart sensor module (merged into main.c)
oled.c          Original OLED display module (merged into main.c)
Photos/         Project photos and system diagram
```

## Building

This project is written in bare-metal AVR C. Compile with `avr-gcc` targeting the ATmega328P:

```bash
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -o main.elf main.c
avr-objcopy -O ihex main.elf main.hex
avrdude -c arduino -p m328p -P COMX -b 115200 -U flash:w:main.hex
```

Replace `COMX` with your Arduino's serial port.

## Configuration

Key constants in `main.c`:

| Constant | Value | Description |
|---|---|---|
| `F_CPU` | 16000000 | CPU clock frequency (16 MHz) |
| `BAUD` | 9600 | UART baud rate |
| `VOLTTHRESHOLD` | 600 | ADC threshold for pulse detection |
| `SSD1306_ADDR` | 0x3C | OLED I2C address |
