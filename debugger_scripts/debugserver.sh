#!/bin/bash

set -e

JLinkGDBServerCLExe -device LPC1768  > /dev/null 2>&1 &
wait -n
