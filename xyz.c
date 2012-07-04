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
OM SAntiH SAntiH SAntiH
saha nA vavatu saha nW b'unaktu
saha vIryam karavAvahY
tEjas vinAvad'Itamastu mA vidviS'AvahY
OM SAntiH SAntiH SAntiH
*/



/*memory stuff*/

mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units)
{
    mem_pool *mp=malloc(sizeof(struct mem_pool));
    if(!mp){fprintf(stderr,"<make_mem_pool> problem: insufficient memory\n");exit(1);}
    mp->prev=NULL;
    mp->next=NULL;

    if(units && size)
    {
        if(type==cell_pool)
        {
            mp->p=malloc(sizeof(struct z)*cell_pool_size*units);
            if(!(mp->p)){fprintf(stderr,"<make_mem_pool> problem: insufficient memory\n");exit(1);}
            mp->size=cell_pool_size*units;
            mp->used=0;
            //printf("mp at %p\n",mp->p);
        }
        else
        {
            mp->p=malloc(size*units);
            if(!(mp->p)){fprintf(stderr,"<make_mem_pool> problem: insufficient memory\n");exit(1);}
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

/*constructors*/

struct z *make_char(mem_pool *mp, int v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tchar);
    typeset(p,gcused);

    p->val.ch=v;

    return p;
}

struct z *make_fixnum(mem_pool *mp, long int v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tfixnum);
    typeset(p,gcused);

    p->val.fixnum=v;

    return p;
}

struct z *make_flonum(mem_pool *mp, double v)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tflonum);
    typeset(p,gcused);

    p->val.flonum=v;

    return p;
}

struct z *make_string(mem_pool *mp, void *v, size_t s)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tnstring);
    typeset(p,gcused);

    p->val.str.p=v;
    p->val.str.l=s;

    return p;
}

struct z *make_string1(mem_pool *mp, char *s)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tnstring);
    typeset(p,gcused);

    p->val.str.p=(void *)s;
    p->val.str.l=strlen(s);

    return p;
}

//struct z *make_rstring(mem_pool *mp, char *s);
//struct z *make_symbol(mem_pool *mp, void *v, size_t s);
//struct z *make_regex(mem_pool *mp, void *v, size_t s);

struct z *make_list(mem_pool *mp, struct z *pcar, struct z *pcdr)
{
    struct z *p=NULL;
    p=make_obj(mp);
    typeinit(p,tlist);
    typeset(p,gcused);

    p->val.list.car=pcar;
    p->val.list.cdr=pcdr;

    return p;
}

//struct z *make_vector(mem_pool *mp, size_t s);

struct z *make_hashmap(mem_pool *mp, size_t s)
{
    struct z *p=NULL, **m=NULL, *x=NULL;
    int i;
    p=make_obj(mp);

    m= (struct z**) malloc(sizeof(struct z*) * default_hashsize);
    if(!m){fprintf(stderr,"<make_hashmap> problem: insufficient memory\n");exit(1);}

    p->val.map.p=m;
    p->val.map.l=0;

    for(i=0; i < default_hashsize ; i++)
    {
        x=make_list(mainpool, obj_null, obj_null);
        m[i]= (struct z*) &x;
    }

    typeinit(p,tmap);
    typeset(p,gcused);
    return p;
}

//struct z *make_iport(mem_pool *mp, FILE *in);
//struct z *make_oport(mem_pool *mp, FILE *out);
//struct z *make_fn(mem_pool *mp, struct z *scope, struct z *body);
//struct z *make_nfn(mem_pool *mp, z *(*pnfn)(struct z *scope));

/*operations*/

//struct z *bit_oper(struct z *obj,int oper);
//struct z *bit_oper2(struct z *obj,struct z *obj2,int oper);
//struct z *char_oper(struct z *obj,int oper);
//struct z *num_oper(struct z *obj,int oper);
//struct z *num_oper2(struct z *obj,struct z *obj2,int oper);
//struct z *string_oper(struct z *obj,int oper);
//struct z *string_oper2(struct z *obj,struct z *obj2,int oper);
//struct z *symbol_oper(struct z *obj,int oper);
//struct z *symbol_oper2(struct z *obj,struct z *obj2,int oper);
//struct z *regex_oper(struct z *obj,int oper);
//struct z *regex_oper2(struct z *obj,struct z *obj2,int oper);


/*list vector map methods*/



//struct z *list_front(struct z *l);
//struct z *list_back(struct z *l);
//struct z *list_at(struct z *l, struct z *obj);
//struct z *list_at2(struct z *l, size_t pos);

//struct z *list_pushfront(struct z *l, struct z *obj);
//struct z *list_popfront(struct z *l);
//struct z *list_pushback(struct z *l, struct z *obj);
//struct z *list_popback(struct z *l);

//void list_insert(struct z *l, size_t pos, struct z *obj);
//void list_insert2(struct z *l, size_t pos, size_t n, struct z *obj);
//void list_erase(struct z *l, size_t pos);
//void list_erase2(struct z *l, size_t pos1, size_t pos2);
//void list_swap(struct z *l1, struct z *l2);
//void list_clear(struct z *l);

//struct z *list_size(struct z *l);
//size_t list_size2(struct z *l);
//struct z *list_maxsize(struct z *l);
//size_t list_maxsize2(struct z *l);
//struct z *list_isempty(struct z *l);
//int list_isempty2(struct z *l);
//void list_reverse(struct z *l, size_t n);

//remove
//removeif
//splice
//sort
//merge
//unique

//struct z *vector_size(struct z *v);
//size_t vector_size2(struct z *v);
//struct z *vector_maxsize(struct z *v);
//size_t vector_maxsize2(struct z *v);
//struct z *vector_isempty(struct z *v);
//int vector_isempty2(struct z *v);
//void vector_reserve(struct z *v, size_t n);

//void vector_clear(struct z *v);
//struct z *vector_resize(struct z *v, struct z *obj);
//struct z *vector_resize2(struct z *v, size_t n);

//void vector_insert(struct z *v, size_t pos, struct z *obj);
//void vector_insert2(struct z *v, size_t pos, size_t n, struct z *obj);
//void vector_erase(struct z *v, size_t pos);
//void vector_erase2(struct z *v, size_t pos1, size_t pos2);
//void vector_swap(struct z *v1, struct z *v2);

//struct z *vector_at(struct z *v, struct z *obj);
//struct z *vector_at2(struct z *v, size_t n);
//struct z *vector_front(struct z *v);
//struct z *vector_back(struct z *v);
//struct z *vector_pushback(struct z *v, struct z *obj);
//struct z *vector_popback(struct z *v);

//void hashmap_resolve(struct z *hm);
//size_t hashmap_hash(struct z *str);

unsigned char rand8[256];

size_t hashmap_hash1(char *str, size_t hsize)
{
    unsigned char h1, h2;
    size_t h;

    if (*str == 0) return 0;
    h1 = *str; h2 = *str + 1;
    str++;

    while (*str)
    {
        h1 = rand8[h1 ^ *str];
        h2 = rand8[h2 ^ *str];
        str++;
    }

    h = ((size_t)h1 << 8)|(size_t)h2;
    return h % hsize;
}

//size_t hashmap_hash2(void *key);
//void hashmap_dump(struct z *hm, FILE *out);

/*
Normal hashmap size computation

Items   Hashsize

0       8
9       16
65      32
129     64
255     128
513     1024
4097    2048
65537   2048
*/

struct z *hashmap_size(struct z *hm)
{
    if(hm)
    {
        return make_fixnum(mainpool, (long int) hashmap_size1(hm));
    }
    else{fprintf(stderr,"<hashmap_size> error: no hashmap exists at %p.\n",hm);exit(1);}
    return NULL;
}


size_t hashmap_size1(struct z *hm)
{
    if(hm)
    {
        size_t s=hm->val.map.l;

        if(s<=8)                    return 8;
        else if(s>8 && s<=64)       return 16;
        else if(s>64 && s<=128)     return 32;
        else if(s>128 && s<=512)    return 64;
        else if(s>512 && s<=1024)   return 128;
        else if(s>1024 && s<=4096)  return 1024;
        else if(s>4096 && s<=65536) return 2048;
        else                        return 2046;
    }
    else{fprintf(stderr,"<hashmap_size1> error: no hashmap exists at %p.\n",hm);exit(1);}
    return 8;
}

//struct z *hashmap_maxsize(struct z *hm);
//size_t hashmap_maxsize2(struct z *hm);
//struct z *hashmap_isempty(struct z *hm);
//int hashmap_isempty2(struct z *hm);

//void hashmap_insert(struct z *hm, struct z *key, struct z *val);

void hashmap_insert1(struct z *hm, char *key, struct z *val)
{
    if(hm)
    {
        if(key[0]=='\0')
        {
            fprintf(stderr,"<hashmap_insert1> error: key is null.\n");
            return;
        }
        hashmap_resolve(hm,key,val);
    }
    else{fprintf(stderr,"<hashmap_insert1> error: no hashmap exists at %p.\n",hm);}
}



/*
hashmap is list of pairs(list of 2 elements) so:

hashmap[hash] is the list
car->element1
    car->pair1
        car->key1
        cdr->value1
    cdr->element2
        car->pair2
            car->key2
            cdr->value2
        cdr->element3
            car->pair3
                car->key3
                cdr->value3
            cdr->element4
                car->pair4
                    car->key4
                    cdr->value4
            cdr->null
*/

void hashmap_resolve(struct z *hm, char *key, struct z *val)
{
    size_t hash=hashmap_hash1(key,hashmap_size1(hm));
    struct z *list=hm->val.map.p[hash];
    struct z *p1, *p2, *pp;
    p1=car(list);//element1
    p2=cdr(list);//element2 - can be obj_null

    while(p2!= obj_null)
    {
        pp=car(p1);//pair1
        char *k=(char *) car(pp)->val.str.p; //key1
        //if key1 already exists then replace value
        if(strcmp(key,k) == 0)
        {
            pp->val.list.cdr=val; //value1
            return;
        }
        else
        {
            p1=p2;
            p2=cdr(p2);
        }
    }
    //key1 not found so insert new element to list
    p1=car(list);//element1
    p2=cdr(list);//element2 - can be obj_null

    while(p2!=obj_null)
    {
        p1=p2;
        p2=cdr(p2);
    }
    //car(p1) is the last element and cdr(p1)=p2=obj_null
    p1->val.list.cdr=make_list(mainpool,make_string1(mainpool, key),val);
}


//void hashmap_insert2(struct z *hm, struct z *list);
//void hashmap_erase(struct z *hm, struct z *key);



/*
hashmap is list of pairs(list of 2 elements) so:

hashmap[hash] is the list
car->element1
    car->pair1
        car->key1
        cdr->value1
    cdr->element2
        car->pair2
            car->key2
            cdr->value2
        cdr->element3
            car->pair3
                car->key3
                cdr->value3
            cdr->element4
                car->pair4
                    car->key4
                    cdr->value4
            cdr->null
*/

void hashmap_erase1(struct z *hm, char *key)
{
    if(hm)
    {
        if(key[0]=='\0')
        {
            fprintf(stderr,"<hashmap_erase1> error: key is null.\n");
            return;
        }
        else
        {
            size_t hash=hashmap_hash1(key,hashmap_size1(hm));
            struct z *list=hm->val.map.p[hash];
            struct z *p1, *p2, *pp, *parentcdrelem;
            p1=car(list);//element1
            p2=cdr(list);//element2 - can be obj_null

            if(p1==obj_null)
                return;

            while(p2!= obj_null)
            {
                pp=car(p1);//pair1
                char *k=(char *) car(pp)->val.str.p; //key1
                //key1 found
                if(strcmp(key,k) == 0)
                {

                    return;
                }
                else
                {
                    p1=p2;
                    p2=cdr(p2);
                }
            }
            //key1 not found - do nothing
        }
    }
    else{fprintf(stderr,"<hashmap_erase1> error: no hashmap exists at %p.\n",hm);}
}

//void hashmap_erase2(struct z *hm, size_t pos);
//void hashmap_swap(struct z *hm1, struct z *hm2);
//void hashmap_clear(struct z *hm);
//struct z *hashmap_find(struct z *hm, struct z *key);
//struct z *hashmap_at(struct z *hm, size_t pos);
//struct z *hashmap_count(struct z *hm, struct z *key);
//size_t hashmap_count2(struct z *hm, struct z *key);















/*
to do

mark and sweep
debug tracing
statistics
configurations

dll linkage
zeromq wrapper

*/








/*intrepreter stuff*/

char banner[]="    ___  ___ ___.__.________ \n    \\  \\/  /<   |  |\\___   / \n     >    <  \\___  | /    /  \n    /__/\\_ \\ / ____|/_____ \\ \n          \\/ \\/           \\/\n\n";


/*zread*/

char linebuff[LINESIZE];
char strbuff[256];



/*zprint*/






/*zeval*/


















/*main loop*/

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
