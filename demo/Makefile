BUILD_DIR = ./build
IDIR = $(BUILD_DIR)/lib/include/SDL2
CPP = g++
CPPFLAGS =-I$(IDIR) -Isrc/3dengine -Isrc/gfx -I$(BUILD_DIR)/lib/include/ -Isrc
CPPFLAGS += -std=c++17 -g
CPPFLAGS += -Wno-deprecated-declarations -Wno-reorder
CPPFLAGS += -Wall -Wextra -Wshadow


ODIR = $(BUILD_DIR)/obj
LDIR = $(BUILD_DIR)/lib/lib

LIB_ASAN=
LIBS=-lm -ldl -lpthread -lfreetype -lFontCache
LIBS_S=-lSDL2 -lSDL2_image -lSDL2_ttf

#_DEPS = gfx_engine.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = src/main.o src/gfx/gfx_engine.o 
_OBJ += src/3dengine/t3dlib1.o 
# _OBJ += src/3dengine/t3dlib2.o src/3dengine/t3dlib3.o
_OBJ += src/3dengine/t3dlib4.o src/3dengine/t3dlib5.o
_OBJ += src/3dengine/t3dlib6.o src/3dengine/t3dlib7.o 
_OBJ += src/3dengine/t3dlib8.o src/3dengine/t3dlib9.o
_OBJ += src/3dengine/t3dlib10.o src/3dengine/t3dlib11.o
_OBJ += src/3dengine/t3dlib12.o src/3dengine/t3dlib13.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	@mkdir -p $(ODIR) $(ODIR)/3dengine $(ODIR)/gfx
	$(CPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/%.o: 3dengine/%.cpp $(DEPS)
	@mkdir -p $(ODIR)
	$(CPP) -c -o $@ $< $(CPPFLAGS)

$(BUILD_DIR):
	mkdir $@

test3d: $(OBJ)
	$(CPP) -o $@ $^ $(CPPFLAGS) -L$(LDIR) $(LIB_ASAN) -Wl,-Bstatic $(LIBS_S) -Wl,-Bdynamic $(LIBS)

.PHONY: clean
.DEFAULT_GOAL := test3d

asan: CPPFLAGS += -fsanitize=address
asan: LIB_ASAN = -lasan
asan: test3d

test: $(BUILD_DIR)/test

TEST_OBJECTS = $(filter-out $(BUILD_DIR)/main.o,$(OBJECTS))
$(BUILD_DIR)/test: $(BUILD_DIR)/testmain.o $(TEST_OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) -L$(LDIR) $(LIBS) -lboost_unit_test_framework

$(BUILD_DIR)/testmain.o: tests/testmain.cpp
	$(CPP) -c -o $@ $< $(CPPFLAGS)

clean:
	rm -f $(ODIR)/*.o $(ODIR)/3dengine/*.o $(ODIR)/gfx/*.o *~ core $(INCDIR)/*~ 