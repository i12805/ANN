#!/bin/bash

#fswebcam -d /dev/video0 -r 640x480 --jpeg 85 --greyscale --scale 14x10 -F 5 --no-banner test.jpg
counter=0
arr=(0)
for i in "$@"
do
   dir=`dirname $i`
   filename=`basename $i`
   newFile=$dir/test$counter.pgm
   echo -n "Converting $i to $newFile ... "
   gm convert $i -colorspace GRAY -resize 64x60! -normalize -quality 0 ${newFile}
   echo -n "sed ... "
   sed '1,3d' ${newFile} > test.temp
   echo -n "move ... "
   mv test.temp ${newFile}
   arr[$counter]=$newFile
   ((counter++))
   rm -f test.temp
   echo "Done."
done
echo "Passing ${arr[*]} to ann ..."
./ann_main ${arr[*]}
echo "Removing files: ${arr[*]} ..."
rm -f ${arr[*]}
