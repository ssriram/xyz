/*

this is the crap.c file which i use to test crap and once i
feel it doesn't smell like real crap will add it into the core

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void splitmodobj(char *name, char schar, char *modname, char *objname)
{
    int i = 0;
    int j = 0;
    int flag = 0;

    while (name[i] != '\0')
    {
        if (flag == 0)
        {
            if (name[i] == schar)
            {
                flag = 1;
                modname[i] = '\0';
            }
            else
            {
                modname[i] = name[i];
            }
        }
        else
        {
            objname[j++] = name[i];
            objname[j]='\0';
        }
    i++;
    }
}

int main()
{
    char m[30],o[30];
    char *n="module1:abc:sdffgdf";

    splitmodobj(n,':',m,o);

    printf("%s\n",m);
    printf("%s\n",o);
}


/*
tray
    module
        symbol table

    main
        symbol table

tray => (main mod1 mod2)
cmodule => main

( (mod1 (a b c d)) (mod2 (e f)) (main (g h a b)) )

*/

