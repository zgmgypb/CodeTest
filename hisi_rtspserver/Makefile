# target source
SRC  := $(wildcard *.c) 
OBJ  := $(SRC:%.c=%.o)

#TARGET := $(OBJ:%.o=%)
TARGET := ipc_demo
.PHONY : clean all

all: $(TARGET)

#MPI_LIBS := $(REL_LIB)/libmpi.a
MPI_LIBS := 

#$(TARGET):%:%.o $(COMM_OBJ)
$(TARGET):$(OBJ) $(COMM_OBJ)
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(MPI_LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS)
	$(STRIP) $(TARGET)

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
