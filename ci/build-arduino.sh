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
arduino-cli core update-index

# Install Arduino AVR core
arduino-cli core install arduino:avr

# Link Arduino library
ln -s $GITHUB_WORKSPACE $HOME/Arduino/libraries/CI_Test_Library

arduino-cli lib install Ethernet
arduino-cli lib install --git-url https://github.com/FortySevenEffects/arduino_midi_library

# Compile all *.ino files for the Arduino Uno
for f in **/*.ino ; do
    arduino-cli compile -b arduino:avr:uno $f
done