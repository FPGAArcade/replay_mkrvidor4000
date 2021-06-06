# FPGAArcade Replay - Arduino MKR Vidor 4000

![mkrvidor4000]

With this library you can boot FPGA cores from [FPGAArcade Replay](https://www.fpgaarcade.com/) framework, on the MKR Vidor 4000.  
This gives access to 20+ different classic computers, consoles and arcade machines from the 1980s.

___

### Prerequisites

* [Arduino MKR Vidor 4000](https://store.arduino.cc/arduino-mkr-vidor-4000)
* [Arduino MKR SD Proto Shield](https://store.arduino.cc/mkr-sd-proto-shield) or [Arduino MKR MEM Shield](https://store.arduino.cc/arduino-mkr-mem-shield)
* Micro-USB OTG cable (to connect Keyboard/Mouse)
* Micro-HDMI to HDMI adapter (to connect a monitor / TV)
* Some means to power the board when the USB port is in OTG mode:
  * a Micro-USB OTG cable, with separate power lead, or
  * a LiPo battery, connected via the JST connector, or
  * a 5V lead directly to the MKR pins
* USB to UART serial console cable (for debugging via the MKR pins, but can also be used to power the Vidor)

___

### How to install the library

Add this library to the Arduino IDE, either by using the Library Manager, or by manually installing it as a .ZIP.  
Please consult [the Arduino documentation](https://www.arduino.cc/en/guide/libraries) for further information.

### How to flash the Vidor

In the Arduino IDE, open up the example `Replay_Boot` (listed under menu `File` -> `Examples` -> `PGAArcade Replay MKR Vidor 4000`).
Proceed to `Upload` (under menu `Sketch`) to flash the Replay firmware (make sure the MKR Vidor 4000 is connected via USB).

### How to load an FPGA core

When the firmware starts up, it looks for `replay.ini` on the SDCARD.  
If there is no SDCARD, or if it fails to find the .ini file, the firmware will display an red/green checkered background, and an error message:

![no_sdcard]

Prepare the SDCARD by FAT32/exFAT formatting it, and copy the contents of the [loader core archive](https://build.fpgaarcade.com/releases/cores/V4/loader/latest) to the root of the SDCARD.  
Typically these files would need to be copied over:
```
Archive:  loader_V4_20210531_2203_3676175.zip
 Length   Method    Size  Cmpr    Date    Time   CRC-32   Name
--------  ------  ------- ---- ---------- ----- --------  ----
  414720  Defl:N    21188  95% 03-16-2020 23:54 63dcda76  background.raw
 1244160  Defl:N   353138  72% 03-16-2020 23:54 780f21c2  background_rgb.raw
  510856  Defl:N    84471  84% 06-01-2021 00:03 d3037870  loader.rbf
  846200  Defl:N   744719  12% 03-16-2020 23:54 98dd5e2f  loop.pcm
    4051  Defl:N     1643  59% 05-27-2021 19:29 e0e19228  replay.ini
    4031  Defl:N     1632  60% 06-19-2020 21:23 aedf624d  replay_pal.ini
--------          -------  ---                            -------
 3024018          1206791  60%                            6 files
 ```
 Insert the SDCARD in SD Proto/MEM Shield, and restart the Vidor.  
 If successful it should show something like this:  
 
 ![loader]
 
### Where to find additional cores

More cores can be found here : https://build.fpgaarcade.com/releases/cores/V4/

### How to build from source

This repository only contains the pre-built library; the actual source code lives in the [main source repository](https://github.com/FPGAArcade/replay_firmware).


[mkrvidor4000]: https://store-cdn.arduino.cc/uni/catalog/product/cache/1/image/500x375/f8876a31b63532bbba4e781c30024a0a/a/b/abx00022_iso_3.jpg  "ARDUINO MKR VIDOR 4000"
[no_sdcard]: www/no_sdcard.jpg
[loader]: www/loader.jpg
