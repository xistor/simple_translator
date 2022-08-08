
include ../.inc

CFLAGS = $(PRJCFLAGS)
LIBS = -ltranslator -lcurl -lssl -lcrypto 
# define any directories containing header files other than /usr/include
INCLUDES = -I../include
LDFLAGS += -L$(SERVER_DIR)/lib
# define the C++ source files
SRCS = test.cpp 
# define the C/C++ object files 

OBJS = $(SRCS:.c=.o)
# define the shared library name
TARGET = test

.PHONY: clean
    
all:    $(TARGET)
	@echo  Successfully compiled.

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS)  $(INCLUDES) $(LDFLAGS)  -o $(SERVER_DIR)/$(OUT_DIR)/$(TARGET) $(OBJS)  $(LIBS)

# compile source files to object files


.cpp.o:
	$(CXX) $(CFLAGS) $(INCLUDES) -cpp $<  -o $@
clean:
	$(RM) *.o *.so