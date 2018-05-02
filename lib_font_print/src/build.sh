#!/bin/bash
CUR_DIR=${PWD}
SRC_ROOT=${CUR_DIR%/*}
TOP_ROOT=${SRC_ROOT%/*}

export LIB_DIR=${SRC_ROOT}/
export INC_DIR=${SRC_ROOT}/


#make PLF=arm
make PLF=x86
