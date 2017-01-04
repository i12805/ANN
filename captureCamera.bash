#!/bin/bash

#fswebcam -d /dev/video0 -r 640x480 --jpeg 85 --greyscale --scale 14x10 -F 5 --no-banner test.jpg
counter=0
arr=(0)
for i in "$@"
do
   dir=`dirname $i`
   filename=`basename $i`
   newFile=$dir/test$counter.pgm
   gm convert $i -resize 64x60! -normalize ${newFile}
   sed '1,3d' ${newFile} > test.temp
   mv test.temp ${newFile}
   arr[$counter]=$newFile
   ((counter++))
   rm -f test.temp
done
./ann_main ${arr[*]}
rm -f ${arr[*]}
