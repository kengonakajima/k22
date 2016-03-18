SRCS=main.cpp

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
clean:
	rm $(OUT) *.o deps.make

depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > deps.make


-include deps.make
