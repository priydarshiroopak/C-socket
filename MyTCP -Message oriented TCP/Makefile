# install: libmysocket.a
# 	gcc server.c -o server -L. -lmysocket
# 	gcc client.c -o client -L. -lmysocket

libmysocket.a: mysocket.o
	ar rcs libmysocket.a mysocket.o

mysocket.o: mysocket.c mysocket.h
	gcc -c mysocket.c -o mysocket.o -lpthread

clean:
	rm *.o *.a