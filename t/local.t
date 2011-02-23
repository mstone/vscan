#!/bin/bash -eu
./vscan-scan-dir --config="$0.lua" t 2>&1 > "$0.log"
python "$0.py" "$0.log"
