#!/bin/bash
CUR_DIR=${PWD}
SRC_ROOT=${CUR_DIR%/*}

export LIB_DIR=${SRC_ROOT}/lib 
export INC_DIR=${SRC_ROOT}/comm

make PROJECT_NAME=gs_prot_proc PLF=x86