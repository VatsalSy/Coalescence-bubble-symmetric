#!/bin/bash

OhOut=("1E-3" "1.58E-03" "2.51E-03" "3.98E-03" "6.31E-03" "1.00E-02" "1.58E-02" "2.51E-02" "3.98E-02" "6.31E-02" "1.00E-01" "1.58E-01" "2.51E-01" "3.98E-01" "6.31E-01" "1.00E+00")
OhIn="1E-5"

RhoIn="1e-3"
LEVEL="12"
tmax="2.0"

start="2000"
end="2015"

for i in `seq $start $end`;
do
cd $i
qcc -O2 -Wall -disable-dimensions coalescenceBubble.c -o coalescenceBubble -lm -fopenmp
export OMP_NUM_THREADS=24
./coalescenceBubble ${OhOut[$i-$start]} $OhIn $RhoIn $LEVEL $tmax
cd ..
done