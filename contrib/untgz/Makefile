
untgz: untgz.o  libz.a
	cc -o untgz  untgz.o libz.a

untgz.o: untgz.c zlib.h
	cc -O -c untgz.c

clean:
	rm -f untgz untgz.o
