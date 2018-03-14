#!/bin/bash
CUR_DIR=${PWD}
SRC_ROOT=${CUR_DIR%/*}

export LIB_DIR=${SRC_ROOT}/lib/release 
export INC_DIR=${SRC_ROOT}/lib/release

make PROJECT_NAME=main PLF=x86