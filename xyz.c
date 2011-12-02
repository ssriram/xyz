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

#define XYZC 1-03dec2011

/*
*/

#include "xyz.h"

mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units)
{
    if(type==cell_pool)
    {
        size=cell_pool_size;
    }

    mem_pool *mp=NULL,*mp1=NULL,*mp2=NULL;
    void *vp;
    if(units && size)
    {
        //printf("size %d \nunits %d\n",size,units);
        mp=malloc(sizeof(mem_pool));
        if(!mp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
        vp=calloc(1,size);
        if(!vp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
        mp->type=type;
        mp->size=size;
        mp->p=vp;
        mp->prev=NULL;
        mp->next=NULL;
        mp1=mp;
        //printf("unit: %d@ %p\n",units,vp);

        if(type==cell_pool)
        {

            /*
            *  <----------------------><---------------------------------------------------->
            *   list of free pointers           list of objects
            */

            if(mp)
            {
                int psize=sizeof(struct z*);
                int osize=sizeof(struct z);
                int objs=(cell_pool_size/(psize+osize));
                int i=0;

                void *base=(void *)mp->p;
                struct z **lstart=(struct z**)base;
                struct z **lend=lstart+objs-1;
                struct z *ostart=(struct z*)((void *)lend+5);
                struct z *oend=ostart+objs-1;

                mp->freelist=*lend;

/*                 printf("size of pointer %d\nsize of object %d\n",psize,osize);
 *                 printf("objs %d\ntotal memory %d\n",objs,cell_pool_size);
 *                 printf("memory start %p\nmemory end %p\n",mp->p,(mp->p+cell_pool_size));
 *                 printf("list start %p\nlist end %p\n",lstart,lend);
 *                 printf("object start %p\nobject end %p\n",ostart,oend);
 */

                for(i=0;i<objs;i++)
                {
                    *lstart=ostart;
                    lstart++;
                    ostart++;
                }

            }
        }

        for(units--;units>0;units--)
        {
            mp2=malloc(sizeof(mem_pool));
            if(!mp2){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
            vp=calloc(1,size);
            if(!vp){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
            mp2->type=type;
            mp2->size=size;
            mp2->p=vp;
            mp2->prev=mp1;
            mp2->next=NULL;
            mp1->next=mp2;
            mp1=mp2;
            //printf("unit: %d@ %p\n",units,vp);

            if(type==cell_pool)
            {

                /*
                *  <----------------------><---------------------------------------------------->
                *   list of free pointers           list of objects
                */

                if(mp)
                {
                    int psize=sizeof(struct z*);
                    int osize=sizeof(struct z);
                    int objs=(cell_pool_size/(psize+osize));
                    int i=0;

                    void *base=(void *)mp->p;
                    struct z **lstart=(struct z**)base;
                    struct z **lend=lstart+objs-1;
                    struct z *ostart=(struct z*)((void *)lend+5);
                    struct z *oend=ostart+objs-1;

                    mp->freelist=*lend;

/*                     printf("size of pointer %d\nsize of object %d\n",psize,osize);
 *                     printf("objs %d\ntotal memory %d\n",objs,cell_pool_size);
 *                     printf("memory start %p\nmemory end %p\n",mp->p,(mp->p+cell_pool_size));
 *                     printf("list start %p\nlist end %p\n",lstart,lend);
 *                     printf("object start %p\nobject end %p\n",ostart,oend);
 */

                    for(i=0;i<objs;i++)
                    {
                        *lstart=ostart;
                        lstart++;
                        ostart++;
                    }
                }
            }

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
            mp2=mp1->prev;
            free(mp1->p);
            //printf("freed memory@ %p\n",mp1->p);
            free(mp1);
            //printf("freed unit@ %p\n",mp1);
            mp1=mp2;
        };
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

/*void gc_init(mem_pool *mp);
void gc_gc(mem_pool *mp);
void gc_shutdown(mem_pool *mp);

void push_root(struct z *obj);
struct z *pop_root(struct z *obj);

struct z *make_obj();
void break_obj(struct z *o);

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

z *make_obj()
{
    z *p;
    p=calloc(1,sizeof(struct z));
    if(!p){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    return p;
}

int main()
{
    mem_pool *m=make_mem_pool(cell_pool,cell_pool_size,15);
    return break_mem_pool(m);
}


#endif
