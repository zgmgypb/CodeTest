#! /bin/bash

CUR_DIR=${PWD}

#export PLF=mv310 # 使用 MV310 用这个 
export PLF=x86
export LIB_DIR=${CUR_DIR}/lib 
export INC_DIR=${CUR_DIR}/comm

cd ${LIB_DIR} && rm -rf *

cd ${CUR_DIR}/comm && make PROJECT_NAME=gs_comm 
cd ${CUR_DIR}/em_broadcast && make PROJECT_NAME=gs_em_protocol 
cd ${CUR_DIR}/prot_proc && make PROJECT_NAME=gs_prot_proc 

export INC_DIR=${CUR_DIR}/lib
cd ${CUR_DIR}/tcp_server && make PROJECT_NAME=tcp_server 

# 生成一个 .a 文件，方便调用和调试
if [ ${PLF} = "arm" ]; then
CROSS_COMPILE=arm-linux-
elif [ ${PLF} = "ppc" ]; then
CROSS_COMPILE=ppc_4xx-
elif [ ${PLF} = "ppc1" ]; then
CROSS_COMPILE=powerpc-apm-linux-gnu-
elif [ ${PLF} = "zynq" ]; then
CROSS_COMPILE=arm-xilinx-linux-gnueabi-
elif [ ${PLF} = "ppc603" ]; then
CROSS_COMPILE=powerpc-linux-uclibc-
elif [ ${PLF} = "x86" ]; then
CROSS_COMPILE=
elif [ ${PLF} = "x64" ]; then
CROSS_COMPILE=
elif [ ${PLF} = "mv310" ]; then
CROSS_COMPILE=arm-hisiv200-linux-
fi

PROJECT_NAME=gs_prot_proc
rm -rf ${LIB_DIR}/release
mkdir -p ${LIB_DIR}/release
cp -f ${LIB_DIR}/*.a ${LIB_DIR}/release
cp -f ${LIB_DIR}/gs_dev_if.h ${LIB_DIR}/release
cp -f ${LIB_DIR}/gs_comm.h ${LIB_DIR}/release
cd ${LIB_DIR}/release
for file in *.a
do
	${CROSS_COMPILE}ar -x $file
done
rm -f *.a
${CROSS_COMPILE}ar rcs lib${PROJECT_NAME}_${PLF}.a *.o
rm -f *.o

#生成 demo 文件
export INC_DIR=${CUR_DIR}/lib/release
export LIB_DIR=${CUR_DIR}/lib/release
cd ${CUR_DIR}/demo && make PROJECT_NAME=main 
