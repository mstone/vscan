#!/bin/bash -e
! ./vscan-view /usr/include/signal.h 2>&1 | tee "$0.log" | grep -q Traceback
