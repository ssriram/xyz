#   ___  ___ ___ __ ________
#   \  \/  /<   |  |\___   /
#    >    <  \___  | /    /
#   /__/\_ \ / ____|/_____ \
#         \/ \/           \/
#
#   - a simple hackable scripting language for simply hacking!
#   - xyz is a small scheme like implemention born out of pure hobby hacking.
#       by ssriram
#       http://thenaughtyguy.blogspot.com

all : xyz

xyz : z.o
	gcc z.o -o z

z.o : z.c
	gcc -c z.c

clean :
	rm -rf z.o z xyz.o xyz


