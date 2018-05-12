SRCS=main.cpp pc.cpp char.cpp field.cpp enemy.cpp effect.cpp mapview.cpp

OBJS=$(SRCS:.cpp=.o)
MOYAILIB=./moyai/libmoyaicl.a
GLFWLIB=./moyai/glfw-3.2/src/libglfw3.a
ALUTLIB=./moyai/libalut.a
SNAPPYLIB=./moyai/libsnappy.a
FTGLLIB=./moyai/libftgl.a
FTLIB=./moyai/freetype-2.4.10/objs/.libs/libfreetype.a
BZ2LIB=./moyai/bzip2-1.0.6/libbz2.a
ZLIB=./moyai/zlib-1.2.7/libz.a
UVLIB=./moyai/libuv-1.20.2/.libs/libuv.a
JPEGLIB=./moyai/jpeg-8d/.libs/libjpeg.a
FRAMEWORKS=-framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation -framework CoreVideo -framework CoreMedia -framework AudioToolbox -framework OpenAL
LIBFLAGS=-L/usr/local/lib -m64  $(MOYAILIB) $(GLFWLIB) $(SNAPPYLIB) $(FTGLLIB) $(FTLIB) $(BZ2LIB) $(ZLIB) $(UVLIB) $(FRAMEWORKS) $(OGGLIB) $(ALUTLIB) $(JPEGLIB)
CFLAGS=-O0 -g  -Wall -m64  -I./moyai/glfw-3.2/include -I./moyai -I./moyai/libuv-1.20.2/include

OUT=k22

all : $(OUT)

$(OUT) : $(OBJS) $(MOYAILIB)
	g++ -o $(OUT) $(OBJS) $(LIBFLAGS) 

main.o : main.cpp
	g++ -c main.cpp $(CFLAGS)
pc.o : pc.cpp
	g++ -c pc.cpp $(CFLAGS)
char.o : char.cpp
	g++ -c char.cpp $(CFLAGS)
field.o : field.cpp
	g++ -c field.cpp $(CFLAGS)
enemy.o : enemy.cpp
	g++ -c enemy.cpp $(CFLAGS)
effect.o : effect.cpp
	g++ -c effect.cpp $(CFLAGS)
mapview.o : mapview.cpp
	g++ -c mapview.cpp $(CFLAGS)

clean:
	rm -f $(OUT) *.o deps.make

depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > deps.make


-include deps.make
