# MATRIX Voice ESP32 as WebThing

The current project shows how to build and upload any firmware for the [MatrixVoice ESP32 board](https://www.matrix.one/products/voice) using [PlatformIO](https://platformio.org/).

**note**: The next documentation is based on [Program MATRIX Voice ESP32 with VS Code Using PlatformIO](https://www.hackster.io/matrix-labs/program-matrix-voice-esp32-with-vs-code-using-platformio-3dd498). You don't need the IDF toolchain or any library setup, PlatformIO will do it for you.

---

## Prerequisites

### PlatformIO software

First install [PlatformIO](http://platformio.org/), an open source ecosystem for IoT development, and its command line tools (Windows, MacOS and Linux). Also, you may need to install [git](http://git-scm.com/) on your system (PC).


### MATRIX Voice software

You should have a RaspberryPi with `MATRIX Voice` software. Please run the following in your RaspberryPi shell or ssh:

##### Add debian repository key:

```bash
curl https://apt.matrix.one/doc/apt-key.gpg | sudo apt-key add -
echo "deb https://apt.matrix.one/raspbian $(lsb_release -sc) main" | sudo tee /etc/apt/sources.list.d/matrixlabs.list
```

##### Update your repository and packages:
```bash
sudo apt-get update
sudo apt-get upgrade
```
#####  Install the MATRIX init package:
```bash
sudo apt install matrixio-creator-init
```
#####  Reboot your Raspberry Pi:
```bash
sudo reboot
```
##### SSH back into the pi, execute this command. You may need to use `sudo`:
```bash
voice_esp32_enable
```

### Installing initial firmware for OTA

Return to your PC and clone this repository:

```bash
git clone https://github.com/esp32-mozilla-webthing
cd esp32-mozilla-webthing
```

**NOTE:** plase change `platformio.ini` and set your `SSID` and `PASSWORD` like this:

```python
'-DWIFI_SSID="MyWifiSsid"'
'-DWIFI_PASS="MyWifiPassw"'
```
##### Building
```bash
pio run
```
##### Upload

Enter the `ota` directory and upload the firmware. Please replace the `IP` parameter with your `Raspberry Pi`'s IP like this:

```bash
cd ota
./install.sh 192.168.178.65
```

The console output should be like this:
```bash
user$ ./install.sh 192.168.178.65

Loading firmware: ../.pio/build/esp32dev/firmware.bin

-----------------------------------
esptool.py wrapper for MATRIX Voice
-----------------------------------
esptool.py v2.7
Serial port /dev/ttyS0
Connecting....
Chip is ESP32D0WDQ6 (revision 1)
Features: WiFi, BT, Dual Core, Coding Scheme None
Crystal is 40MHz
MAC: 30:ae:a4:07:6f:7c
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Auto-detected Flash size: 4MB
Wrote 32768 bytes at 0x00001000 in 3.0 seconds (86.2 kbit/s)...
Hash of data verified.
Wrote 966656 bytes at 0x00010000 in 90.4 seconds (85.6 kbit/s)...
Hash of data verified.
Wrote 16384 bytes at 0x00008000 in 1.5 seconds (86.5 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
done

[SUCCESS] Please disconnect your MatrixVoice from the RaspberryPi and reconnect it alone for future OTA updates.
```
(end of `Prerequisites`)

---

> This particular example does not include code to enable over the air reprogramming of the MATRIX Voice ESP32. You can find the code required for this in our other examples.

## Troubleshooting

#### Uploading issues

If `pio run --target upload` does not work, please check the `MVID` parameter, it should be a short name, or you can pass the ESP32 MATRIX Voice's IP in the `upload_port` parameter in the `platformio.ini` file.

#### Building issues

For a complete `clean` of the project to get the latest version of the libraries, please test the following commands:

```javascript
git pull
pio run -t clean && rm -r .pio
pio run
```
