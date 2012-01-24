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

#ifndef XYZC

#define XYZC

/*
*/

#include "xyz.h"

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
    typeset(p,gcmark);
    p->val.ch=v;
    return p;
}

struct z *make_fixnum(mem_pool *mp, long int v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tfixnum);
    typeset(p,gcmark);
    p->val.fixnum=v;
    return p;
}

struct z *make_flonum(mem_pool *mp, double v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tflonum);
    typeset(p,gcmark);
    p->val.flonum=v;
    return p;
}

struct z *make_string(mem_pool *mp, void *v, size_t s)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tnstring);
    typeset(p,gcmark);
    p->val.str.p=v;
    p->val.str.l=s;
    return p;
}

struct z *make_string1(mem_pool *mp, char *s)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tnstring);
    typeset(p,gcmark);
    p->val.str.p=(void *)s;
    p->val.str.l=strlen(s);
    return p;
}

//struct z *make_rstring(mem_pool *mp, char *s);



int main()
{
    int i;
    z *p;
    mem_pool *mp = make_mem_pool(cell_pool,1,1);


    break_mem_pool(mp);
    return 0;
}


#endif
