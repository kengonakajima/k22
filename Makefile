SRCS=main.cpp pc.cpp char.cpp field.cpp enemy.cpp effect.cpp shipdata.cpp mapview.cpp

OBJS=$(SRCS:.cpp=.o)
MOYAILIB=./moyai/libmoyaicl.a
GLFWLIB=./moyai/glfw-3.1.2/src/libglfw3.a
SNAPPYLIB=./moyai/libsnappy.a
FTGLLIB=./moyai/libftgl.a
FTLIB=./moyai/freetype-2.4.10/objs/.libs/libfreetype.a
BZ2LIB=./moyai/bzip2-1.0.6/libbz2.a
ZLIB=./moyai/zlib-1.2.7/libz.a
UVLIB=/usr/local/lib/libuv.a
LIBFLAGS=-framework Cocoa -framework IOKit -framework OpenGL -framework CoreFoundation -framework CoreVideo -m64  ./moyai/fmod/api/lib/libfmodex.dylib $(MOYAILIB) $(GLFWLIB) $(SNAPPYLIB) $(FTGLLIB) $(FTLIB) $(BZ2LIB) $(ZLIB) $(UVLIB)
CFLAGS=-O0 -g  -Wall -m64  -I./moyai/glfw-3.1.2/include -I./moyai -I/usr/local/include

OUT=k22

all : $(OUT)

$(OUT) : $(OBJS)
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
shipdata.o : shipdata.cpp
	g++ -c shipdata.cpp $(CFLAGS)
mapview.o : mapview.cpp
	g++ -c mapview.cpp $(CFLAGS)

clean:
	rm $(OUT) *.o deps.make

depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > deps.make


-include deps.make
