#!/bin/bash

sed -e 's/^.\+Arduino\.h.\+$/#include <stdint.h>/' ../mdaEPianoData.h >mdaEPianoData.h
sed -i 's/PROGMEM//' mdaEPianoData.h
sed -i 's/const//' mdaEPianoData.h
gcc -o xfade_generator xfade_generator.c
rm mdaEPianoData.h
./xfade_generator >../mdaEPianoDataXfade.h
rm xfade_generator
