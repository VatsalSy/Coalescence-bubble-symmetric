#!/bin/bash

start="2000"
end="2015"

for i in `seq $start $end`;
do

  echo $i
  scp -r *py $i/
  scp -r *get* $i/
  cd $i/
  
  python Facets_symmetric.py $i

  ffmpeg -framerate 60 -pattern_type glob -i 'VideoFacetsOnly/*.png' -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" -c:v libx264 -r 30 -pix_fmt yuv420p $i-Facets.mp4 -y

  cd ../

done