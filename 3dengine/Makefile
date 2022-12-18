TARGET = 3dengine

# -isystem to disable warnings from external libs
IDIR = -I./include -isystem ./lib/include/ -I./lib/include/SDL2

CC = gcc
CFLAGS = $(IDIR)
CFLAGS += -Wall -Wextra -Wshadow

CPP = g++
CPPFLAGS = $(IDIR)
CPPFLAGS += -std=c++17 -g
CPPFLAGS += -Wno-deprecated-declarations -Wno-reorder
CPPFLAGS += -Wall -Wextra -Wshadow

ODIR=obj
LDIR =./lib/lib

LIB_ASAN=
LIBS=-lm -ldl -lpthread -lGL -lz
LIBS_S=-lSDL2 -lSDL2_image -lassimp

BUILD_DIR = build
C_SOURCES =
CPP_SOURCES = src/main.cpp src/gfx_engine.cpp src/object.cpp

LDFLAGS = -L$(LDIR) $(LIB_ASAN) -Wl,-Bstatic $(LIBS_S) -Wl,-Bdynamic $(LIBS)

all: $(BUILD_DIR)/$(TARGET)

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CPP) -c -o $@ $< $(CPPFLAGS)

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CPP) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR):
	mkdir $@

asan: CPPFLAGS += -fsanitize=address
asan: LIB_ASAN = -lasan
asan: $(BUILD_DIR)/$(TARGET)

test: $(BUILD_DIR)/test

TEST_OBJECTS = $(filter-out $(BUILD_DIR)/main.o,$(OBJECTS))
$(BUILD_DIR)/test: $(BUILD_DIR)/testmain.o $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) -lboost_unit_test_framework

$(BUILD_DIR)/testmain.o: tests/testmain.cpp
	$(CPP) -c -o $@ $< $(CPPFLAGS)

.PHONY: clean

clean:
	rm -f $(BUILD_DIR)/* *~ core