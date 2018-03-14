#!/bin/bash
CUR_DIR=${PWD}
SRC_ROOT=${CUR_DIR%/*}

export LIB_DIR=${SRC_ROOT}/lib 
export INC_DIR=${SRC_ROOT}/lib

make PROJECT_NAME=tcp_server PLF=x86