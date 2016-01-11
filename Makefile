
PROGRAM_NAME = fontpacker

## Platform name.
UNAME := $(shell uname)

## 32 vs 64 bits.
ARCH := $(shell getconf LONG_BIT)

## x86 vs arm.
PROCESSOR_TYPE := $(shell uname -m)

CC = clang++
CC_FLAGS = -std=c++11 -Wall

OBJ_DIR = build
SRC_DIR = source
INC_DIR = -Iinclude -I/usr/local/include/ -I/usr/local/include/freetype2/

LINKER_INC = -L/usr/local/lib/
LINKER_FLAGS = -laxUtils -lfreetype -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPP_FILES := $(wildcard source/*.cpp)
OBJ_FILES := $(addprefix build/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS :=

all: create_directory $(OBJ_FILES)
	$(CC) $(CC_FLAGS) $(INC_DIR) $(OBJ_FILES) $(LINKER_INC) $(LINKER_FLAGS) -o $(PROGRAM_NAME) 

create_directory:
	@mkdir -p $(OBJ_DIR)

build/%.o: source/%.cpp
	$(CC) $(CC_FLAGS) $(INC_DIR) -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(PROGRAM_NAME)


