#!/bin/bash

fswebcam -d /dev/video0 -r 640x480 --jpeg 85 --greyscale --scale 64x60 -F 5 --no-banner test.jpg
gm convert test.jpg -normalize test.pgm

sed '6d' test.pgm > test.temp
mv test.temp test.pgm

./mm.elf test.pgm
