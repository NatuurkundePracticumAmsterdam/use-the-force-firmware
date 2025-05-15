# Requires platformio to be installed
# See https://platformio.org/
# Or search Platformio in VSCode exentions

import os

###########################################################
# Update the following paths before running the script. 
# Most times it just requires replacing <user>

# Path to platformio python script, .platformio/penv/Scripts/python.exe
path_py = r"C:\Users\<user>\.platformio\penv\Scripts\python.exe"

# Path to esptools.py, .platformio/packages/tool-esptoolpy/esptool.py
path_esptool = r"C:\Users\<user>\.platformio\packages\tool-esptoolpy\esptool.py"

# Path to boot_app0.bin, .platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin
path_app0 = r"C:\Users\<user>\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"

############################################################
# The following files are found in the compiled folder.
# If relative paths do not work, use the full path name. (VSCode: right click file -> Copy Path)
# Path to bootloader.bin
path_bootloader = r"compiled\bootloader.bin"
# Path to partitions.bin
path_partitions = r"compiled\partitions.bin"
# Path to firmware.bin
path_firmware = r"compiled\firmware.bin"

os.system(f'{path_py} "{path_esptool}" --chip esp32s3 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0000 "{path_bootloader}" 0x8000 "{path_partitions}" 0xe000 "{path_app0}" 0x10000 "{path_firmware}"')
