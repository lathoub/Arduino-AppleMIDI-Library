#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
# Create directories
mkdir $HOME/Arduino
mkdir $HOME/Arduino/libraries
# Install Arduino IDE
export PATH=$PATH:$GITHUB_WORKSPACE/bin
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
arduino-cli config init
arduino-cli config set library.enable_unsafe_install true
arduino-cli core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index

# Install Arduino AVR core
arduino-cli core install arduino:avr
arduino-cli core install arduino:samd
# arduino-cli core install arduino:esp8266
# arduino-cli core install esp32:esp32:esp32doit-devkit-v1

# Link Arduino library
ln -s $GITHUB_WORKSPACE $HOME/Arduino/libraries/CI_Test_Library

arduino-cli lib install Ethernet
arduino-cli lib install "MIDI library"
arduino-cli lib install --git-url https://github.com/sstaub/Ethernet3.git
arduino-cli lib install EthernetBonjour

# Compile all *.ino files for the Arduino Uno
for f in **/AVR_*.ino ; do
    arduino-cli compile -b arduino:avr:uno $f
done

# Compile all *.ino files for the Arduino Zero
for f in **/SAMD_*.ino ; do
    arduino-cli compile -b arduino:samd:mkrzero $f
done

# Compile all *.ino files for the ESP8266
# for f in **/ESP8266_*.ino ; do
#     arduino-cli compile -b arduino:esp8266:??? $f
# done

# Compile all *.ino files for the ESP32
# for f in **/ESP32_*.ino ; do
#     arduino-cli compile -b arduino:esp32:??? $f
# done
