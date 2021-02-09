#!/bin/sh 

arduino-cli board list

arduino-cli compile --fqbn arduino:avr:uno ecuMog

arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno ecuMog