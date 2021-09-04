#!/bin/bash

# Source this files to setup the required environment for this SDK

export SUMERU_DIR="$( cd -- "$(dirname "${BASH_SOURCE}")" >/dev/null 2>&1 ; pwd -P )"
export PATH=${PATH}:${SUMERU_DIR}/bin
export PATH=${PATH}:${SUMERU_DIR}/riscv-gnu-toolchain/bin

export SUMERU_UART_DEVICE=/dev/ttyUSB0
