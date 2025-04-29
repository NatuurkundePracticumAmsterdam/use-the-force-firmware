# Firmware Use the Force
This repository contains firmware for setups using a load cell+HX711 with an M5Push6060 and M5DinMeter (esp32-based board).

The firmware exposes a serial interface at a default baud rate of 115200, through which all communication is achieved.

On the M5DinMeter, port A is used for the M5Push6060 (RS485 Unit) and port B is used for the M5 Weight Unit. (Ports names can be found on stickers on the side)

Serial output is prefixed with a label, to allow for client-side filtering. Below is a short description of all labels.
- ```[INFO]:```: Informative message. Safe to ignore but useful for debugging.
- ```[ERROR]:```: An error occurred while processing the command.
- ```[VALUE]:```: A value read from the load cell.
- ```[TIME;VALUE]:```: A value read from the load cell, accompanied by a timestamp denoting the number of ms since the first read related to this one.
- ```[VEL]:```: Motor velocity in $\frac1{60}$ mm/s.
- ```[POS]:```: Motor position in mm.

Any serial command follows the form: 
```"#" <opcode> [argument[,argument]] ";"```. I.e.: a command begins with a ```#```, followed by a 2-character opcode from the list below, optionally followed by up to two comma-separated arguments, ending with a ```;```.

## List of valid opcodes:
### 0 Arguments
 - ```AB```: Abort continuous read. If called while a continuous read is active, at most one more read will occur.
 
 - ```ST```: Stop motor. If called will simulate counts going above threshold. Most times when called while moving will crash stepper motor controller, requiring disconnection of power. Else can home.
 **!!! DOES NOT WORK DURING HOME !!!**

 - ```GP```: Get position. Returns the current position in mm. If the current position is not known, UINT8_MAX (255) is returned.

 - ```GV```: Get Velocity. Returns the current velocity as $\frac1{60}$ mm/s.

 - ```SR```: Single Read. Returns the load cell's read value.

 - ```TR```: Tare. Takes no argument. Sets the load cell's current reading as its offset, effectively zeroing it. Only works for interface.

  - ```HM```: Home. Takes no argument. Homes the stage. This is done by translating downwards until the stop is hit, and then translating upwards 1 mm.

 - ```CZ```: Set Counts Zero (for maximum counts). This point will be seen as a zero. Make sure to keep the load cell with the same direction facing up, as values between which side is up may differ a lot.
 
 - ```CM```: Set Counts Maximum. A way to set the abort limit by hanging the maximum allowed load on the load cell. The order of ```CZ``` and ```CM``` is not important, but it is recommended to do both.

 - ```VR```: Get firmware version.

 - ```ID```: Get motor ID that is used for the stepper motor. 
---

### 1 Argument
 - ```SP```: Set Position. Takes an integer in range [0, 47). 0 mm is the lowest position of the stage, 46 mm the highest. The stage must first be homed in order to set the position.
 
 - ```SV```: Set Velocity. Takes an integer in range [1, 200]. Sets the stage velocity in $\frac1{60}$ mm/s.

 - ```SF```: Set calibration Force. Takes a float. Calibrates the load cell based on the load cell's current reading and the provided argument. Only affects interface. Gets saved in memory and used on startup.

 - ```UX```: Updates interface text x offset.

 - ```UY```: Updates interface text y offset.

 - ```UL```: Updates interface text line height.

 - ```UU```: Updates interface displayed unit. Maximum of 8 chars, 2 to 4 recommended.
---

### 2 Arguments
 - ```CR```: Continuous Read. Takes two integers in range [1, INT_MAX or 2147483647]. The first argument represents the number of reads to perform. The second argument represents the milliseconds in between reads. 
---

*A note on calibration*: While the use of two separate commands may seem tedious, it is necessary for accurate calibration. This is due to the fact that the load cell must be zeroed **before** any calibrating force is applied.

*A note on stage position*: The hardware does not offer support to get the stage's position when moving. This means that only when the stage is not moving, the actual position can be read out. If the stage **is** moving, the target position will be returned. In order to know the stage's real position during movement, the caller must calculate it based on the set speed and elapsed time.

## Building and flashing the firmware
For convenience, a ```platformio.ini``` file is provided. This makes managing dependencies, building, and flashing fairly straightforward. For instructions on the installation of PlatformIO for your specific platform, please refer to the official documentation.

To build the firmware, run the following command from the root directory: ```pio run```.

To flash the firmware, connect the board via usb, and run the following from the root directory: ```pio run --target=upload```.

To clean up build files, run the following from the root directory: ```pio run --target=clean```.

## Build Flags
*Only noteworthy flags are mentioned, most other build flags should be kept as is.*

#### `DVERSION`
Version of the firmware that is running. This is the version that gets called ```VR```. It is highly adviced that the version increases per commit that changes the software as to help with debugging.

#### `DLOOP_DELAY`
Delay after each "loop" (in ms). A loop consists of the following actions:
- Read values from load cell
- Send back a value if one waas reqeuested (with ```SR``` or ```CR```)
- Update interface if loop count has been reached (see `DINTERFACE_LOOP_INTERVAL`)

#### `DSERIAL_TIMEOUT`
When a command, or any other message, is send over the serial port, the device will wait an amount of milliseconds from when the first bit arrives. A value too low will not read full commands. Rule of thumb: minimum $\frac{\text{max bits}}{\text{baudrate}}$ ms

#### `DINTERFACE_LOOP_INTERVAL`
Amount of loops that has to pass.

#### `DNUM_READS`
Amount of times the load cell gets polled per reading.

#### `DCHECK_INTERVAL_MULT`
Multiplier for how often the load cell has to be polled.

#### `DCOMMAND_DISPLAY_DURATION`
Time that a command is displayed instead of showing current readings.

#### `DINIT_MAX_COUNTS` & `DINIT_MAX_COUNTS_ZERO`
Initial maximum counts and base value may be set during building. These can be changed with ```CM``` and ```CZ``` respectively.

#### `DINTERFACE_X_OFFSET`, `DINTERFACE_Y_OFFSET` & `DINTERFACE_LINE_HEIGHT`
Offset for text, these can also be set with the commands ```UX```, ```UY``` and ```UL``` respectively.

#### `DFONT_SIZE` & `DFONT_TYPE`
Font options. Types can be found in TFT user setup header file.

