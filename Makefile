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

xyz : xyz.o
	gcc xyz.o -o xyz

xyz.o : xyz.c
	gcc -c xyz.c

clean :
	rm -rf xyz.o xyz


