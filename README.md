# UvA-VU Z-Stage
This repository contains firmware for setups using a load cell+HX711 with an M5Push6060 and M5DinMeter (or any other esp32-based board).
The firmware exposes a serial interface at a baud rate of 115200, through which all communication is achieved. 

Any serial command follows the form: 
```"#" <opcode> [argument] ";"```. I.e.: a command begins with a ```#```, followed by a 2-character opcode from the list below, optionally followed by an argument, ending with a ```;```.

## List of valid opcodes:

 - ```SP```: Set Position. Takes an integer in range [0, 47). 0 mm is the lowest position of the stage, 46 mm the highest.
 - ```GP```: Get position. Takes no argument. Returns the current position in mm. If the current position is not known, UINT8_MAX, or 255, is returned.
 - ```SV```: Set Velocity. Takes an integer in range [1, 200]. Sets the stage velocity in mm/s.
 - ```GV```: Get Velocity. Takes no argument. Returns the current velocity in mm/s.
 - ```GM```: Get Mode. Returns ```raw``` if reading without calibration, or ```calibrated``` if reading with calibration.
 - ```TM```: Toggle mode. Switch between the two modes mentioned above.
 - ```SR```: Single Read. Takes no argument. Returns the position in mm and the load cell's value in the same unit it was calibrated with, or in counts if raw.
 - ```CR```: Continuous Read. Takes an integer in range [1, INT_MAX or 2147483647]. The argument represents the number of milliseconds to continuously read from the load cell. It streams values to the serial port in the format: ```timestamp;value;pos``` where ```timestamp``` is the number of milliseconds since the first read, ```value``` is the same as described above, as well as ```pos```.
 - ```HM```: Home. Takes no argument. Homes the stage. This is done by translating downwards until the stop is hit, and then translating upwards 46 mm (the maximum height).
 - ```TR```: Tare. Takes no argument. Sets the load cell's current reading as its offset, effectively zeroing it.
 - ```CL```: Calibrate. Takes no argument. Starts the load cell calibration sequence. Tares the load cell and sets its slope to 1. After calling this command, the caller should apply a known calibration force to the cell, and provide that value as an argument to the below command.
 - ```SF```: Set calibration Force. Takes a float. Calibrates the load cell based on the load cell's current reading and the provided argument. It is the callers responsibility to call ```CL``` beforehand. Calibration is unit-agnostic, and based solely on the argument's unit.
 
 A note on calibration: While the use of two separate commands may seem tedious, it is necessary for more accurate calibration. This is due to the fact that the load cell must be zeroed **before** any calibrating force is applied.
## Building and flashing the firmware
For convenience, a ```platformio.ini``` file is provided. This makes managing dependencies, building, and flashing fairly straightforward. For instructions on the installation of PlatformIO for your specific platform, please refer to the official documentation.

To build the firmware, run the following command from the root directory: ```pio run```.

To flash the firmware, connect the board via usb, and run the following from the root directory: ```pio run --target=upload```.

To clean up build files, run the following from the root directory: ```pio run --target=clean```.
