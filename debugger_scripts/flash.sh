#!/bin/bash

set -e

JLinkExe -device LPC1768 -If JTAG -Speed 4000 -JTAGConf -1,-1 -AutoConnect 1 -CommandFile ./debugger_scripts/flash.jlink