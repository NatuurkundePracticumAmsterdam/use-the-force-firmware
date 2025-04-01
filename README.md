# UvA-VU Z-Stage
This repository contains firmware for setups using a load cell+HX711 with an M5Push6060 and M5DinMeter (or any other esp32-based board).
The firmware exposes a serial interface at a baud rate of 115200, through which all communication is achieved.
Serial output is prefixed with a label, to allow for client-side filtering. Below is a short description of all labels.
- ```[INFO]: ```: Informative message. Safe to ignore but useful for debugging.
- ```[ERROR]: ```: An error occurred while processing the command.
- ```[VALUE]: ```: A value read from the load cell.
- ```[TIME;VALUE]: ```: A value read from the load cell, accompanied by a timestamp denoting the number of ms since the first read related to this one.

Any serial command follows the form: 
```"#" <opcode> [argument[,argument]] ";"```. I.e.: a command begins with a ```#```, followed by a 2-character opcode from the list below, optionally followed by up to two comma-separated arguments, ending with a ```;```.

## List of valid opcodes:

 - ```SP```: Set Position. Takes an integer in range [0, 47). 0 mm is the lowest position of the stage, 46 mm the highest. The stage must first be homed in order to set the position.
 - ```GP```: Get position. Takes no argument. Returns the current position in mm. If the current position is not known, UINT8_MAX, or 255, is returned.
 - ```SV```: Set Velocity. Takes an integer in range [1, 200]. Sets the stage velocity in mm/s.
 - ```GV```: Get Velocity. Takes no argument. Returns the current velocity in mm/s.
 - ```GM```: Get Mode. Returns ```raw``` if reading without calibration, or ```calibrated``` if reading with calibration.
 - ```TM```: Toggle mode. Switch between the two modes mentioned above.
 - ```SR```: Single Read. Takes no argument. Returns the load cell's value in the same unit it was calibrated with, or in counts if operating in raw mode.
 - ```CR```: Continuous Read. Takes two integers in range [1, INT_MAX or 2147483647]. The first argument represents the number of reads to perform. The second argument represents the number of milliseconds in between reads. It streams values to the serial port in the format: ```timestamp;value``` where ```timestamp``` is the number of milliseconds since starting execution of the command, and ```value``` is the same as described above.
 - ```HM```: Home. Takes no argument. Homes the stage. This is done by translating downwards until the stop is hit, and then translating upwards 46 mm (the maximum height).
 - ```TR```: Tare. Takes no argument. Sets the load cell's current reading as its offset, effectively zeroing it.
 - ```CL```: Calibrate. Takes no argument. Starts the load cell calibration sequence. Tares the load cell and sets its slope to 1. After calling this command, the caller should apply a known calibration force to the cell, and provide that value as an argument to the below command.
 - ```SF```: Set calibration Force. Takes a float. Calibrates the load cell based on the load cell's current reading and the provided argument. It is the callers responsibility to call ```CL``` beforehand. Calibration is unit-agnostic, and based solely on the argument's unit.
 - ```SC```: Save Configuration. Takes no argument. Writes the current calibration settings to flash memory, so they persist between reboots.

*A note on calibration*: While the use of two separate commands may seem tedious, it is necessary for accurate calibration. This is due to the fact that the load cell must be zeroed **before** any calibrating force is applied.

*A note on stage position*: The hardware does not offer support to get the stage's position when moving. This means that only when the stage is not moving, the actual position can be read out. If the stage **is** moving, the target position will be returned. In order to know the stage's real position during movement, the caller must calculate it based on the set speed and elapsed time.

## Building and flashing the firmware
For convenience, a ```platformio.ini``` file is provided. This makes managing dependencies, building, and flashing fairly straightforward. For instructions on the installation of PlatformIO for your specific platform, please refer to the official documentation.

To build the firmware, run the following command from the root directory: ```pio run```.

To flash the firmware, connect the board via usb, and run the following from the root directory: ```pio run --target=upload```.

To clean up build files, run the following from the root directory: ```pio run --target=clean```.
