#!/bin/bash

set -e

arm-none-eabi-gdb -x debugger_scripts/gdb_commands
PID=$(ps -a | grep JLinkGDBServer | awk '{print $1;}')
echo "Found JLink GDB server with PID ${PID}"
KILL="kill ${PID}"
eval "$KILL"
