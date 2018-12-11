#!/bin/bash

cd ..
make -j10

while true
do
  ./da_proc 1 ./membership testLOG
  python tests/test_log.py 2>&1 | tee tests/output.txt
  if [ "X$?" != "X0" ]
  then
    echo "Error, please see output.txt"
    exit 1
  fi
done
