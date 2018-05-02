####  SUBARCH PROJECT_NAME  LIB_DIR  INC_DIR

PROJECT_NAME := font_print
TARGET			:=lib$(PROJECT_NAME)_$(PLF).a
LIBRELEASE_DIR := ../lib

ifeq ($(PLF), arm)
CROSS_COMPILE := arm-linux-
else
CROSS_COMPILE := 
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

VPATH	:= $(SRCDIR)

BUILD_DIR := $(TOPDIR)/objs

INCS	 = $(PROJECT_NAME).h 

CFLAGS	= -I$(INC_DIR)
CFLAGS  += -Os -Wall 

.PHONY:PRE

$(TARGET):PRE $(OBJS)
	cd $(BUILD_DIR) && $(AR) rcs $@ $(OBJS)
	$(CP) $(BUILD_DIR)/$@ $(LIBRELEASE_DIR)
	$(CP) $(INCS) $(LIBRELEASE_DIR)
	$(RM)   $(BUILD_DIR)

PRE:
	$(MKDIR) $(BUILD_DIR)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $(BUILD_DIR)/$@
