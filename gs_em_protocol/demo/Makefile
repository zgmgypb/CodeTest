####  PLF PROJECT_NAME  LIB_DIR  INC_DIR

TARGET :=$(PROJECT_NAME)

ifeq ($(PLF), arm)
CROSS_COMPILE := arm-linux-
else ifeq ($(PLF), ppc)
PREFIX = ppc_4xx-
else ifeq ($(PLF), ppc1)
CROSS_COMPILE := powerpc-apm-linux-gnu-
else ifeq ($(PLF), zynq)
CROSS_COMPILE := arm-xilinx-linux-gnueabi-
else ifeq ($(PLF), ppc603)
CROSS_COMPILE := powerpc-linux-uclibc-
else ifeq ($(PLF), x86)
CROSS_COMPILE :=
else ifeq ($(PLF), x64)
CROSS_COMPILE :=
else ifeq ($(PLF), mv310)
CROSS_COMPILE := arm-hisiv200-linux-
endif

CC	= $(CROSS_COMPILE)gcc
AR 	= $(CROSS_COMPILE)ar
STRIP 	= $(CROSS_COMPILE)strip

CP 	= cp -f
RM 	= rm -rf
MAKE 	= make
ECHO 	= echo
MKDIR 	= mkdir -p
RMDIR 	= rmdir

TOPDIR 	:= $(PWD)
SRCDIR 	= $(TOPDIR)
SRCDIR	+= $(TOPDIR)/lib

SRCS 	= $(notdir $(foreach temp, $(SRCDIR), $(wildcard $(temp)/*.c)))
OBJS 	= $(patsubst %.c, %.o , $(SRCS))

BUILD_DIR := $(TOPDIR)/objs

INCS	 = *.h 

CFLAGS	= -I$(INC_DIR) 
CFLAGS  += -Os -Wall 

LFLAGS = -lgs_prot_proc_$(PLF) -lpthread -lrt -L$(LIB_DIR)

.PHONY:PRE clean

$(TARGET):PRE $(OBJS)
	cd $(BUILD_DIR) && $(CC) -o $@ $(OBJS) $(LFLAGS)
	-$(CP) $(BUILD_DIR)/$@ $(TOPDIR)
	$(RM) $(BUILD_DIR)

PRE:
	$(MKDIR) $(BUILD_DIR)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $(BUILD_DIR)/$@

clean:
	$(RM) $(BUILD_DIR)
