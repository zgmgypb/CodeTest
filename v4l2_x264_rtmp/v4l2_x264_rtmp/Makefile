LIVE555= ../live555/live
X264= ../live555/x264
all: v4l2Rtmper

LIB_SUFFIX=a
INCLUDES = -I$(LIVE555)/UsageEnvironment/include -I$(LIVE555)/groupsock/include \
	   -I$(LIVE555)/liveMedia/include -I$(LIVE555)/BasicUsageEnvironment/include -I$(X264)
CXXFLAGS= -c -g3 -O0 -fPIC $(INCLUDES)
USAGE_ENVIRONMENT_DIR = $(LIVE555)/UsageEnvironment
USAGE_ENVIRONMENT_LIB = $(USAGE_ENVIRONMENT_DIR)/libUsageEnvironment.$(LIB_SUFFIX)
BASIC_USAGE_ENVIRONMENT_DIR = $(LIVE555)/BasicUsageEnvironment
BASIC_USAGE_ENVIRONMENT_LIB = $(BASIC_USAGE_ENVIRONMENT_DIR)/libBasicUsageEnvironment.$(LIB_SUFFIX)
LIVEMEDIA_DIR = $(LIVE555)/liveMedia
LIVEMEDIA_LIB = $(LIVEMEDIA_DIR)/libliveMedia.$(LIB_SUFFIX)
GROUPSOCK_DIR = $(LIVE555)/groupsock
GROUPSOCK_LIB = $(GROUPSOCK_DIR)/libgroupsock.$(LIB_SUFFIX)
LOCAL_LIBS =	$(LIVEMEDIA_LIB) $(GROUPSOCK_LIB) \
		$(BASIC_USAGE_ENVIRONMENT_LIB) $(USAGE_ENVIRONMENT_LIB)


v4l2Rtmper: rtmper.o sender.o capture.o vencoder.o
	$(CXX) -o $@ $^ -fPIC -lavcodec -lswscale -lavutil  -L../live555/x264 -lx264 -lpthread -lrt -lrtmp

cpp.o:
	$(CXX) $(CXXFLAGS) $<

clean:
	rm -f *.o
	rm -f v4l2Rtmper
