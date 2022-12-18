IDIR =./lib/include/SDL2
CPP=g++
CPPFLAGS=-I$(IDIR) -I3dengine -I./lib/include/ -I.
CPPFLAGS+= -std=c++17 -g
CPPFLAGS+= -Wno-deprecated-declarations -Wno-reorder
CPPFLAGS+= -Wall -Wextra -Wshadow


ODIR=obj
LDIR =./lib/lib

LIB_ASAN=
LIBS=-lm -ldl -lpthread -lfreetype -lFontCache
LIBS_S=-lSDL2 -lSDL2_image -lSDL2_ttf

#_DEPS = gfx_engine.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o gfx_engine.o 
_OBJ += 3dengine/t3dlib1.o 
# _OBJ += 3dengine/t3dlib2.o 3dengine/t3dlib3.o
_OBJ += 3dengine/t3dlib4.o 3dengine/t3dlib5.o
_OBJ += 3dengine/t3dlib6.o 3dengine/t3dlib7.o 
_OBJ +=3dengine/t3dlib8.o 3dengine/t3dlib9.o
_OBJ += 3dengine/t3dlib10.o 3dengine/t3dlib11.o
_OBJ += 3dengine/t3dlib12.o 3dengine/t3dlib13.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	@mkdir -p $(ODIR) $(ODIR)/3dengine
	$(CPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/%.o: 3dengine/%.cpp $(DEPS)
	@mkdir -p $(ODIR)
	$(CPP) -c -o $@ $< $(CPPFLAGS)

test3d: $(OBJ)
	$(CPP) -o $@ $^ $(CPPFLAGS) -L$(LDIR) $(LIB_ASAN) -Wl,-Bstatic $(LIBS_S) -Wl,-Bdynamic $(LIBS)

.PHONY: clean

#.DEFAULT_GOAL := test3d
asan: CPPFLAGS += -fsanitize=address
asan: LIB_ASAN = -lasan
asan: test3d

clean:
	rm -f $(ODIR)/*.o $(ODIR)/3dengine/*.o *~ core $(INCDIR)/*~ 