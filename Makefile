# xyz - a small scheme like implementation
#	by ssriram
#	http://thenaughtyguy.blogspot.com


all : xyz

xyz : xyz.o
	gcc xyz.o -o xyz

xyz.o : xyz.c
	gcc -c xyz.c

clean :
	rm -rf xyz.o xyz


