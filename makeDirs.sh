#!/bin/bash

start="2000"
end="2015"
for i in `seq $start $end`;
do
echo $i
mkdir -p $i
scp -r coal* $i/
done
