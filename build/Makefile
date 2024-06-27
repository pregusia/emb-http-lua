CXX=/usr/bin/gcc
CFLAGS=-DEPOLL -O3 -c
LDFLAGS=-static
INCLUDES=-I/path/to/lua-5.4.4/src
OBJS=/path/to/lua-5.4.4/src/liblua.a /usr/lib/libm.a

all: emb-http-lua

emb-http-lua: log.o vfs.o luaapp.o main.o mime.o utils.o hashmap.o
	$(CXX) $(LDFLAGS) log.o vfs.o luaapp.o main.o mime.o utils.o hashmap.o $(OBJS) -o emb-http-lua 

log.o: ../src/log.c ../src/log.h
	$(CXX) $(CFLAGS) -o log.o ../src/log.c

vfs.o: ../src/vfs.c ../src/vfs.h ../src/log.h ../src/utils.h
	$(CXX) $(CFLAGS) -o vfs.o ../src/vfs.c

luaapp.o: ../src/luaapp.c ../src/luaapp.h ../src/vfs.h ../src/log.h ../src/utils.h
	$(CXX) $(CFLAGS) -o luaapp.o ../src/luaapp.c

main.o: ../src/main.c ../src/utils.h ../src/vfs.h ../src/log.h ../src/luaapp.h
	$(CXX) $(CFLAGS) -o main.o ../src/main.c

mime.o: ../src/mime.c ../src/mime.h ../src/utils.h ../src/vfs.h
	$(CXX) $(CFLAGS) -o mime.o ../src/mime.c

utils.o: ../src/utils.c ../src/utils.h
	$(CXX) $(CFLAGS) -o utils.o ../src/utils.c

hashmap.o: ../src/hashmap.c ../src/hashmap.h
	$(CXX) $(CFLAGS) -o hashmap.o ../src/hashmap.c


clean:
	rm -f *.o
	rm -f emb-http-lua


