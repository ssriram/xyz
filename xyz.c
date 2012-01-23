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

#define XYZC 1-23jan2012

/*
*/

#include "xyz.h"

void init_cell_pool(mem_pool *mp)
{
    void *p;
    int i;
    p=malloc(sizeof(struct z*)*cell_pool_size);
    if(!p){fprintf(stderr,"problem: insufficient memory\n");exit(1);}

    struct z **zpp=(struct z**)p;
    struct z *zp=(struct z *)mp->p;

    for(i=0;i<cell_pool_size;i++)
    {
        zpp[i]=&zp[i];
    }
    mp->f=zpp;
}

mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units)
{
    mem_pool *mp=NULL;
    void *vp;
    int i;

    if(units && size)
    {
        mp=(struct mem_pool *)malloc(sizeof(mem_pool)*units);
        if(!mp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}

        for(i=0;i<units;i++,mp++)
        {
            if(i==0)
            {
                mp->prev=NULL;
                mp->next=&mp[i+1];
            }
            else if(i==(units-1))
            {
                mp->prev=&mp[i-1];
                mp->next=NULL;
            }
            else
            {
                mp->next=&mp[i+1];
                mp->prev=&mp[i-1];
            }

            if(type==cell_pool)
            {
                vp=malloc(sizeof(struct z)*cell_pool_size);
                if(!vp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
                mp->p=vp;
                mp->size=cell_pool_size;
                init_cell_pool(mp);
            }
            else
            {
                vp=malloc(size);
                if(!vp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
                mp->p=vp;
                mp->size=size;
            }
            mp->type=type;
        }
    }
    return mp;
}

int break_mem_pool(mem_pool *mp)
{
    mem_pool *mp1=NULL,*mp2=NULL;
    if(mp)
    {
        mp1=mp;
        while(mp1->next)
        {
            mp1=mp1->next;
        };
        while(mp1->prev)
        {
            if(mp1->type==cell_pool)
            {
                free((void *)mp1->f);
            }

            mp2=mp1->prev;
            free(mp1->p);
            //printf("freed memory@ %p\n",mp1->p);
            free(mp1);
            //printf("freed unit@ %p\n",mp1);
            mp1=mp2;
        };

        if(mp1->type==cell_pool)
        {
            free((void *)mp1->f);
        }

        free(mp1->p);
        //printf("freed memory@ %p\n",mp1->p);
        free(mp1);
        //printf("freed unit@ %p\n",mp1);
        mp=NULL;
        return 0;
    }
    return -1;
}

void mem_pool_expand(mem_pool *mp,mem_pool_type type, size_t size, size_t units)
{
    mem_pool *p=mp;
    while(p->next){p=p->next;};
    mem_pool *m=make_mem_pool(type,size,units);
    p->next=m;
    m->prev=p;
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

    return p;
}

//void break_obj(mem_pool *mp, struct z *o);

/*
struct z *make_char(int v);
struct z *make_fixnum(long int v);
struct z *make_flonum(double v);
struct z *make_string(void *v, size_t s);
struct z *make_symbol(void *v, size_t s);
struct z *make_regex(void *v, size_t s);
struct z *make_list(struct z *pcar, struct z *pcdr);
struct z *make_vector(size_t s);
struct z *make_hashmap(size_t s);
struct z *make_iport(FILE *in);
struct z *make_oport(FILE *out);
struct z *make_fn(struct z *scope, struct z *body);
struct z *make_nfn(z *(*pnfn)(struct z *scope));
*/



int main()
{
    mem_pool *m=make_mem_pool(cell_pool,cell_pool_size,2);
    int i;
    z *p;
    for(i=1;i<=4098;i++)
    {
        p=make_obj(m);
        //if(i==1 || i>4093) printf("obj %d at %p\n",i,p);
    }
    return break_mem_pool(m);
}


#endif
