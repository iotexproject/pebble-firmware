
# IoTeX Pebble Firmware

The firmware implements the asset tracking application using Pebble hardware designed by IoTeX.

# download v1.4.0

. git clone -b v1.4.0 https://github.com/iotexproject/pebble-firmware

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

# update
. Add a new board: iotex_pebble_hw20, select board name iotex_pebble_hw20ns in NS mode

. Autom generate AES key then write into kmu slot

. Automatically generate ECC key pair, store it in Flash after encryption

. After the pebble is powered on, the ECC public key will be sent to the thigsboard


#  [FOTA(firmware over-the-air)](https://github.com/iotexproject/pebble-firmware/tree/v1.4.0/nrf/samples/nrf9160/http_application_update)

   The HTTP application update sample demonstrates how to do a basic FOTA (firmware over-the-air) update. It uses the FOTA download library to download a file from a remote server and write it to flash
   
