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
                printf("%c",name[i]);
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


/*

z *findobj(z *objs, char *name)
{
    z *p, *sym;
    sym=objs;
	int idx=0;
    while(!is_null(sym))
    {
        if(strcmp(car(sym)->val.s,name)==0)
        {
			printf("%u %s at %d\n",(unsigned)car(sym),car(sym)->val.s,idx);
            return car(sym);
        }
        sym=cdr(sym);
		idx++;
    }
    p=make_obj();
    TYPESET(p,tsymbol);
    p->val.s=malloc(strlen(name)+1);
    if(!p->val.s){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    strcpy(p->val.s,name);
    objs=cons(objs,p);
    return p;
}

z *make_symbol(char *name)
{
    char modname[30],objname[30];
    z *obj, *tray, *currmod, *currobjs;;
    splitmodobj(name,':',modname,objname);
    if(modname)
    {
        tray=gtray;
        while(!is_null(tray))
        {
            currmod=car(tray);
            if(strcmp(car(currmod).val->s,modname)==0))
            {
                currobjs=cdr(currmod);
                return findobj(currobjs,objname);
            }
            tray=cdr(tray);
        }
        fprintf(stderr,"error: undefined module\n");
        return obj_notok;
    }
    else
    {
        tray=gtray;
        while(!is_null(tray))
        {
            currmod=car(tray);
            if(strcmp(car(currmod).val->s,"main")==0))
            {
                currobjs=cdr(currmod);
                return findobj(currobjs,objname);
            }
            tray=cdr(tray);
        }
        fprintf(stderr,"error: main module not found\n");
        return obj_notok;
    }
}

*/




//z *make_list()






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
