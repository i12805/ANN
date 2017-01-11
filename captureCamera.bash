#!/bin/bash

newFile="test"
cho "Capture camera to file $newFile ... "
fswebcam -d /dev/video0 -r 640x480 --jpeg 85 --greyscale --scale 64x60 -F 5 --no-banner ${newFile}.jpg
echo -n "Converting $newFile ... "
gm convert ${newFile}.jpg  -colorspace GRAY -resize 64x60! -normalize -quality 0 ${newFile}.pgm
echo -n "sed ... "
sed '1,3d' ${newFile}.pgm > test.temp
echo -n "move ... "
mv test.temp ${newFile}.pgm
rm -f test.temp
echo "Done."
echo "Passing ${nreFile}.pgm to ann ..."
./ann_main ${newFile}.pgm
echo "Removing all of ${newFile} ..."
#rm -f ${newFile}.pgm ${newFile}.jpg
