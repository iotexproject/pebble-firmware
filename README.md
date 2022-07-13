
# IoTeX Pebble Firmware

The firmware implements the asset tracking application using Pebble hardware designed by IoTeX.

# download v1.4.0

. git clone -b v1.4.0   https://github.com/iotexproject/pebble-firmware.git

. cd  pebble-firmware

. git clone -b v2.4.0-ncs1  https://github.com/nrfconnect/sdk-zephyr  zephyr

. cd  zephyr

. git branch manifest-rev

# How to securely sign firmware images for booting by MCUboot

1. Run cmd.exe in Windows and bash shell in Linux.

2. Go to the directory of imgtool.

   cd    xxx/v1.4.0/bootloader/mcuboot/scripts  ('xxx' is your NCS SDk directory ) .

3. Installation dependencies

   pip3 install --user -r  requirements.txt

4. Generate a keypair

   python3  imgtool.py  keygen -k root-ec-p256.pem -t ecdsa-p256

5.  Copy file 'root-ec-p256.pem' to  "xxx/v1.4.0/bootloader/mcuboot"  directory and overwrite the original file ('xxx' is your NCS SDk directory ). 


6.   Recompile the firmware, the new firmware will be signed with the new key and verified at startup



