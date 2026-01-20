# Automatic Machine Lubrication System

An Arduino-based automatic lubrication system for industrial machinery with timer-controlled oil dispensing, manual override, and oil level monitoring.

## Features

- **Timer-based automatic lubrication** - Configurable interval for automatic oil dispensing
- **Forced start** - Manual override button for immediate lubrication
- **Oil level monitoring** - Floater sensor detects oil tank level
- **LED status indicators** - Visual feedback for oil status (OK/EMPTY)
- **State persistence** - Timer state saved to EEPROM, survives power cycles
- **Debug mode** - Serial output for troubleshooting

## Hardware Requirements

| Component | Pin | Description |
|-----------|-----|-------------|
| MOSFET | 3 | Controls oil pump motor |
| Forced Start Button | 4 | Manual lubrication trigger |
| Timer Button | 5 | Enable/disable automatic timer |
| Floater Sensor | 6 | Oil level detection |
| Oil OK LED | 7 | Indicates sufficient oil level |
| Oil EMPTY LED | 8 | Indicates low oil level |

## Configuration

Key parameters in `sketch.ino`:

```cpp
const unsigned long TIMER_INTERVAL = 100000;  // Timer interval (ms)
const unsigned long MOTOR_DURATION = 6019;    // Motor run time (ms)
#define DEBUG true                             // Enable/disable debug output
```

## Dependencies

- [PinChangeInterrupt](https://github.com/NicoHood/PinChangeInterrupt) - For interrupt handling on any pin
- EEPROM (built-in Arduino library)

## Installation

1. Install the PinChangeInterrupt library via Arduino Library Manager
2. Clone this repository or download the files
3. Open `sketch.ino` in Arduino IDE
4. Upload to your Arduino board

## Operation

1. **Power on** - System loads saved timer state from EEPROM
2. **Timer button** - Toggle automatic lubrication timer on/off
3. **Forced start button** - Trigger immediate lubrication cycle
4. **Oil level** - LEDs indicate current oil tank status

## Wokwi Simulation

This project includes Wokwi simulation files for testing without hardware.

## License

MIT License
