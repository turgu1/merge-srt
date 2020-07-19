#!/bin/bash

gcc -o merge-srt merge-srt.c

RESULT=$?
if [ $RESULT -eq 0 ]; then
  ./merge-srt J-C.srt J-C_data.csv out.srt
else
  echo failed
fi
