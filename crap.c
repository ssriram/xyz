/*

this is the crap.c file which i use to test crap and once i
feel it doesn't smell like real crap will add it into the core

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xyz.h"




unsigned char rand8[256];
unsigned short int hash(char *str) {
unsigned short int h;
unsigned char h1, h2;
if (*str == 0) return 0;
h1 = *str; h2 = *str + 1;
str++;
while (*str) {
h1 = rand8[h1 ^ *str];
h2 = rand8[h2 ^ *str];
str++;
}
/* h is in range 0..65535 */
h = ((unsigned short int)h1 << 8)|(unsigned short int)h2;
/* use division method to scale */
return h % HASH_TABLE_SIZE
}




int main()
{
    return 0;
}



/*core function definitions*/

z *make_bool(int b)
{
    z *p=make_obj();
    TYPESET(p,tbool);
    p->val.i=(long int)b;
    return p;
}

z *make_inum(long int i)
{
    z *p=make_obj();
    TYPESET(p,tinum);
    p->val.i=i;
    return p;
}

z *make_num(double d)
{
    z *p=make_obj();
    TYPESET(p,tnum);
    p->val.d=d;
    return p;
}

z *make_string(char *str)
{
    z *p=make_obj();
    TYPESET(p,tstring);
    p->val.s=malloc(strlen(str)+1);
    if(!p->val.s){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    strcpy(p->val.s,str);
    return p;
}

z *cons(z *pcar, z *pcdr) //cons a.k.a make_pair
{
    z *p=make_obj();
    TYPESET(p,tpair);
    p->val.pair.car=pcar;
    p->val.pair.cdr=pcdr;
    return p;
}

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


z *findobj(z *objs, char *name)
{
    z *p, *sym;
    sym=objs;
  int idx=0;
    while(!is_null(sym))
    {
        if(strcmp(car(sym)->val.s,name)==0)
        {
            //obj found
      printf("%u %s at %d\n",(unsigned)car(sym),car(sym)->val.s,idx);
            return car(sym);
        }
        sym=cdr(sym);
    idx++;
    }

    //obj not found
    return obj_notok;
}

z *createobj(z *objs, char *name)
{
    z *obj;
    if(objs && name)
    {
        obj=make_obj();
        TYPESET(obj,tsymbol);
        obj->val.s=malloc(strlen(name)+1);
        if(!obj->val.s){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
        strcpy(obj->val.s,name);
        objs=cons(objs,obj);
        return obj;
    }
    fprintf(stderr,"error: invalid objname / memory access\n");
    return obj_notok;
}

z *make_symbol(z *scope, char *name)
{

/*
if modname present in name
    if module present
        if obj present return obj
        else throw error
    else throw error
else
    if main module present
        if obj present return obj
        else create obj
    else throw error
*/

    char modname[30],objname[30];
    z *obj, *tray, *currmod, *currobjs;

    if(strchr(name,':')!=NULL)
    {
        splitmodobj(name,':',modname,objname);

        if(modname)
        {
            //modname present
            tray=gtray;
            while(!is_null(tray))
            {
                currmod=car(tray);
                if(strcmp(car(currmod)->val.s,modname)==0)
                {
                    //module present
                    currobjs=cdr(currmod);
                    obj= findobj(currobjs,objname);
                    if(obj!=obj_notok)
                    {
                        //obj present
                        return obj;
                    }
                    else
                    {
                        fprintf(stderr,"error: undefined object\n");
                        return obj_notok;
                    }
                }
                tray=cdr(tray);
            }
            fprintf(stderr,"error: undefined module\n");
            return obj_notok;
        }
        else
        {
            fprintf(stderr,"error: undefined module or object\n");
            obj= obj_notok;
        }
    }
    else
    {
        //no module name present
        tray=gtray;
        while(!is_null(tray))
        {
            currmod=car(tray);
            if(strcmp(car(currmod)->val.s,"main")==0)
            {
                //main module present
                currobjs=cdr(currmod);
                obj= findobj(currobjs,name);
                if(obj!=obj_notok)
                {
                    //obj present
                    return obj;
                }
                else
                {
                    //obj not present so create it.
                    obj= createobj(currobjs,name);
                    return obj;
                }
            }
            tray=cdr(tray);
        }
        fprintf(stderr,"error: main module not found\n");
        return obj_notok;
    }
}




////////////



z *make_nfn(z *(*pnfn)(struct z *scope, struct z *args))
{
    z *p=make_obj();
    TYPESET(p,tnfn);
    p->val.nfn=pnfn;
    return p;
}

z *make_fn(z *scope, z *args, z *body)
{
    z *p=make_obj();
    TYPESET(p,tfn);
    p->val.fn.args=args;
    p->val.fn.body=body;
    p->val.fn.scope=scope;
    return  p;
}

z *make_iport(FILE *in)
{
    z *p=make_obj();
    TYPESET(p,tiport);
    p->val.in=in;
    return p;
}

z *make_oport(FILE *out)
{
    z *p=make_obj();
    TYPESET(p,toport);
    p->val.out=out;
    return p;
}

void init()
{
    obj_null=make_obj();
    TYPESET(obj_null,tnull);
    obj_true=make_obj();
    TYPESET(obj_true,tbool);
    obj_true->val.i=1;
    obj_false=make_obj();
    TYPESET(obj_false,tbool);
    obj_false->val.i=0;

    symbols=obj_null;
    empty_scope=obj_null;
}

void load_default_symbols(z *scope)
{
    obj_quote=make_symbol(scope,"quote");
    obj_def=make_symbol(scope,"def");
    obj_set=make_symbol(scope,"set");
    obj_ok=make_symbol(scope,"ok");
    obj_notok=make_symbol(scope,"not-ok");
    obj_if=make_symbol(scope,"if");
    obj_fn=make_symbol(scope,"fn");
    obj_do=make_symbol(scope,"do");
    obj_cond=make_symbol(scope,"cond");
    obj_else=make_symbol(scope,"else");
    obj_let=make_symbol(scope,"let");
    obj_and=make_symbol(scope,"and");
    obj_or=make_symbol(scope,"or");
    obj_quasiquote=make_symbol(scope,"quasiquote");
    obj_unquote=make_symbol(scope,"unquote");
    obj_unquotesplice=make_symbol(scope,"unquotesplice");
}



/*========================================================*/

/*dynamic stack*/

int dstack_push(struct dstack *st, void *p)
{
    if(st)
    {
        st->list = malloc(sizeof(struct stacknode));
        if(st->list == 0)
        {
            fprintf(stderr,"dstack error: no memory\n");
            exit(1);
        }
        st->list->p = p;
        st->list->link = st->head;
        st->head= st->list;
        st->size++;

        return 0;
    }
    else
    {
        return 1;
    }
}

void *dstack_pop(struct dstack *st)
{
    if(st)
    {
        if(st->head == 0)
        {
            /*dstack is empty*/
            return 0;
        }
        void *tmp = st->head->p;
        st->list = st->head;
        st->head = st->head->link;
        free(st->list);
        st->size--;
        return tmp;
    }
    else
    {
        return 0;
    }

}

void dstack_pop2(struct dstack *st)
{
    void *p=dstack_pop(st);
    p=0;
}

void dstack_init(struct dstack *st)
{
    if(st)
    {
        st->head = 0;
        st->list = 0;
        st->size=0;
    }
    else
    {}
}

/*dynamic stack*/

/*dynamic hashtable*/

hashnode *htable_create()
{
    hashnode *p=malloc((sizeof(struct hashnode))*HASHSIZE);
    if(!p){fprintf(stderr,"hashmap error: no memory\n");exit(1);}
    return p;
}

void htable_init(hashnode *ht)
{
    int i;
    for(i==0;i<HASHSIZE;i++)
    {
        ht[i].value=0;
        ht[i].next=0;
        ht[i].key=0;
    }
}

int htable_hash(char *str)
{
    int index = 0;
    char *tmp = 0;
    tmp = calloc(strlen(str) + 1, sizeof(char));
    strcpy(tmp, str);
    while(*tmp)
    {
        index += *tmp;
        tmp++;
    }
    index = index % HASHSIZE;
    return index;
}

int htable_hash1(char *str)
{
    int index = 0;
    for(;*str;str++) index= MULTIPLIER * index + *str;
    return index % HASHSIZE;
}

int htable_hash2(char *str,int multi, int items)
{
    unsigned int h_=0;
    for(;*str;str++) h_= multi * h_ + *str;
    return h_ % items;
}

void htable_put(hashnode *ht, char *str, void *v)
{
    int index = 0;

    index = htable_hash1(str);
    if(ht[index].value != 0)
    {
        htable_resolve(ht, index, str, v);
    }
    else
    {
        ht[index].key = calloc(strlen(str) + 1, sizeof(char));
        strcpy(ht[index].key, str);
        ht[index].value=v;
    }
}

void htable_resolve(hashnode *ht, int loc, char *str, void *v)
{
    hashnode *tmp;
    tmp = ht + loc;

    while(tmp->next != 0)
        tmp = tmp->next;
    tmp->next = (hashnode *)malloc(sizeof(hashnode));
    tmp->next->key = calloc(strlen(str) + 1, sizeof(char));
    strcpy(tmp->next->key, str);
    tmp->next->value=v;
    tmp->next->next = 0;
}

void htable_dump(hashnode *ht, FILE *out)
{
    int i = 0;
    hashnode *target;

    for(i = 0; i < HASHSIZE; i++)
    {
        if(ht[i].value != 0)
        {
            target = ht + i;
            while(target)
            fprintf(out,"index[%d] key[%s] value[%u]",i,target->key,(unsigned)target->value);
            target = target->next;
        }
    }
}

int htable_remove(hashnode *ht, char *str)
{
    hashnode *h1;
    hashnode *h2;
    char *tmp = 0;
    int index = 0;

    index = htable_hash1(str);

    /*no element*/
    if(ht[index].value == 0)
        return 0;

    /*single element*/
    if(ht[index].next == 0)
    {
        if(strcmp(ht[index].key, str) == 0)
        {
            /*value found*/
            tmp = ht[index].key;
            free(ht[index].key);
            ht[index].key = 0;
            ht[index].value = 0;
            free(tmp);
        }
    }
    else
    {
        /*chain of elements*/
        h1 = ht + index;
        /*traverse the chain*/
        while(h1->next != 0)
        {
            if(strcmp(h1->next->key, str) == 0)
            {
                h2 = h1->next;
                if(h1->next->next)
                    h1->next = h1->next->next;
                else
                    h1->next = 0;

                h2->key = 0;
                h2->value = 0;
                free(h2);
            }
        }
    }

 return 0;
}

/*dynamic hashtable*/
