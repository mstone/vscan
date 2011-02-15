#!/bin/bash
./vscan-scan-dir --config="$0.lua" /usr 2>&1 \
  | tee "$0.log" \
  | egrep '^C' \
  | xargs python "$0.py"
