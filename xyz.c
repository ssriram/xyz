/*
Copyright (c) 2010-2011 Sriram Srinivasan.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to
whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "xyz.h"


#ifndef XYZC

#define XYZC

/*
*/

mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units)
{
    mem_pool *mp=malloc(sizeof(struct mem_pool));
    if(!mp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    mp->prev=NULL;
    mp->next=NULL;

    if(units && size)
    {
        if(type==cell_pool)
        {
            mp->p=malloc(sizeof(struct z)*cell_pool_size*units);
            if(!(mp->p)){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
            mp->size=cell_pool_size*units;
            mp->used=0;
            //printf("mp at %p\n",mp->p);
        }
        else
        {
            mp->p=malloc(size*units);
            if(!(mp->p)){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
            mp->size=size*units;
            mp->used=0;
        }
        mp->type=type;
    }
    return mp;
}

int break_mem_pool(mem_pool *mp)
{
    free(mp->p);
    //printf("freed mp->p at %p\n",mp->p);
    free(mp);
    //printf("freed mp at %p\n",mp);
}

void mem_pool_expand(mem_pool *mp,mem_pool_type type, size_t size, size_t units)
{
    mem_pool *p=mp;
    mem_pool *m=make_mem_pool(type,size,units);
    p->prev=m;
    m->next=p;
}

void gc_init(mem_pool *mp)
{
}
void gc_gc(mem_pool *mp)
{
    printf("gc_gc called\n");
}

/*
void gc_shutdown(mem_pool *mp);
void gc_push_root(mem_pool *mp, struct z *obj);
struct z *gc_pop_root(mem_pool *mp, struct z *obj);
*/

struct z *make_obj(mem_pool *mp)
{
    struct z *p = NULL;
    struct mem_pool *mp1=mp;
    size_t found=0;

    while (mp1)
    {
        if(mp1->type==cell_pool)
        {
            if(mp1->used == 0)
            {
                p=(struct z*)(mp1->p);
                mp1->used++;
            }
            else if((mp1->used)+1 <= (mp1->size))
            {
                p=(struct z*)(mp1->p);
                found = (mp1->used);
                p=&p[found];
                mp1->used++;
            }
            else
            {
                mp1=mp1->next;
            }

            if(p)
            {
                //printf("used %d, size %d\n",mp1->used,mp1->size);
                return p;
            }
        }
        else
        {
            mp1=mp1->next;
        }
    }
    //trigger gc
    gc_gc(mp);
    //for now just expand the pool for space
    mem_pool_expand(mp,cell_pool,1,1);
    //get new object
    return make_obj(mp);
}

//void break_obj(mem_pool *mp, struct z *o);

struct z *make_char(mem_pool *mp, int v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tchar);
    typeset(p,gcused1);
    p->val.ch=v;
    return p;
}

struct z *make_fixnum(mem_pool *mp, long int v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tfixnum);
    typeset(p,gcused1);
    p->val.fixnum=v;
    return p;
}

struct z *make_flonum(mem_pool *mp, double v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tflonum);
    typeset(p,gcused1);
    p->val.flonum=v;
    return p;
}

struct z *make_string(mem_pool *mp, void *v, size_t s)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tnstring);
    typeset(p,gcused1);
    p->val.str.p=v;
    p->val.str.l=s;
    return p;
}

struct z *make_string1(mem_pool *mp, char *s)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tnstring);
    typeset(p,gcused1);
    p->val.str.p=(void *)s;
    p->val.str.l=strlen(s);
    return p;
}

//struct z *make_rstring(mem_pool *mp, char *s);

/*intrepreter*/



/*zread*/

char linebuff[LINESIZE];
char strbuff[256];
char *currline=linebuff;
char *lastline=linebuff;

char banner[]="    ___  ___ ___.__.________ \n    \\  \\/  /<   |  |\\___   / \n     >    <  \\___  | /    /  \n    /__/\\_ \\ / ____|/_____ \\ \n          \\/ \\/           \\/\n\n";

int is_delim(char *s, int c)
{
    while(*s)
        if (*s++ == c)
            return 0;
        return 1;
}

int is_hex(int ch)
{
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}


int zread_char()
{
    if(currline >= lastline)
    {
        if(feof(ifp))
        {
            fclose(ifp);
            ifp=stdin;
            if(!silent)
                printf("%s",banner);
        }
        strcpy(linebuff,"\n");
        if (fgets(currline = linebuff, LINESIZE, ifp) == NULL)
            if (ifp == stdin)
            {
                if (!silent)
                    fprintf(efp, "escape from xyz...\n");
                exit(0);
            }
        lastline = linebuff + strlen(linebuff);
    }
    return (*currline++);
}

void zread_clear()
{
    currline = lastline = linebuff;
}

void zread_flush()
{
    if(ifp != stdin)
    {
        fclose(ifp);
        ifp=stdin;
    }
    zread_clear();
}

void zread_putback()
{
    currline--;
}

char *zread_str(char *delim)
{
    char *p = strbuff;
    while (is_delim(delim, (*p++ = zread_char())))
        ;
    zread_putback();
    *--p = '\0';
    return strbuff;
}

char *zread_string()
{
    char c, *p = strbuff;
    for (;;)
    {
        if ((c = zread_char()) != '"')
            *p++ = c;
        else if (p > strbuff && *(p-1) == '\\')
            *(p-1) = '"';
        else
        {
            *p = '\0';
            return strbuff;
        }
    }
}

void zread_skipws()
{
    while(isspace(zread_char()))
        ;
    zread_putback();
}

int tokenize()
{
    zread_skipws();
    switch(zread_char())
    {
        case '(': return tok_lparen;
        case ')': return tok_rparen;
        case '[': return tok_lsquare;
        case ']': return tok_rsquare;
        case '{': return tok_lbrace;
        case '}': return tok_rbrace;
        case ':': return tok_collon;
        case ';': return tok_scollon;
        case '\'': return tok_quote;
        case '"': return tok_dquote;
        case '`': return tok_backtick;
        case '!': return tok_excl;
        case '~':
            if(zread_char() == '@')
            {
                return tok_at;
            }
            else
            {
                zread_putback();
                return tok_tilde;
            }
        case '#': return tok_hash;
        case ',': return tok_comma;
        default:
            zread_putback();
        return tok_atom;
    }
}



/*zprint*/






/*zeval*/

#define __error0(s)     __begin                                         \
        param = make_list(defmp, make_string1(defmp, s), obj_null);     \
        optr = op_err0;                                                 \
        return obj_true; __end

#define __error1(s,a)   __begin                                         \
        param = make_list(defmp, (a), s);                               \
        param = make_list(defmp, make_string1(defmp, s), param);        \
        optr = op_err0;                                                 \
        return obj_true; __end

#define __goto(a)       __begin                                         \
        optr = a;                                                       \
        return obj_true; __end

#define __save(a,b,c)   (                                               \
        dump = make_list(defmp, env, make_list(defmp, c, dump)),        \
        dump = make_list(defmp, b, dump),                               \
        dump = make_list(make_fixnum(defmp, (long int)(a)), dump))      \

#define __return(a)     __begin                                         \
        value = a;
        optr =








int main()
{
    int i;
    z *p;
    mem_pool *mp = make_mem_pool(cell_pool,1,1);

    p=make_string1(mp,"this is a very big string");
    //printf("%x,%x\n",typeget(p),tnstring);
    printf("%s",banner);


    break_mem_pool(mp);
    return 0;
}


#endif
