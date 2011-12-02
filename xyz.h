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

#ifndef XYZH

#define XYZH 1-03dec2011
#define LINUX

/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#ifdef LINUX

#include <sys/types.h>
#include <sys/socket.h>

#endif

#ifdef WINDOWS

#endif

/*
    flags
    xxxxxxxx    xxxxxxxx    xxxxxxxx    xxxxxxxx
    gc          other       subtype     type
*/

typedef enum types
{
    tnull = 1,
    tbool, tchar, tnum,
    tstring, tsymbol, tregex,
    tiport, toport,
    tlist, tvector, tmap, tlink,
    tfn, tnfn,
    tenv, tpromise, tcontinuation,
    tmacro, texcep,
    tblob,

    tsubtype = 1<<8,
    ttrue = (tsubtype + tbool),
    tfalse = (tsubtype + tbool + tsubtype),
    tfixnum = (tsubtype + tnum),
    tflonum = (tsubtype + tnum + tsubtype),
    tstring2 = (tsubtype + tstring),
    tstring3 = (tsubtype + tstring + tsubtype),

    gcmask = 0x00ffffff,
    gcprotect = 1<<30,
    gcmark = 1<<31,
    gcclear = 0xffffffff >> 1,
} types;

struct z;

typedef struct z
{
    int flags;
    union
    {
        int ch;
        double flonum;
        long int fixnum;
        struct
        {
            void *p;
            size_t l;
        } str; /*string,symbol,regex*/
        struct
        {
            struct z *car;
            struct z *cdr;
        } list;
        struct
        {
            struct z *p;
            size_t l;
        } vector;
        struct
        {
            struct z *p;
            size_t l;
        } map;
        struct
        {
            struct z *scope;
            struct z *body;
        } fn;
        struct z *(*nfn)(struct z *scope);
        struct z *link;
        FILE *in;
        FILE *out;
        struct
        {
            void *blob;
            size_t s;
        } blob;
    } val;
} z;


inline int typeget(z *obj)
{
    return obj->flags&0x0000ffff;
}
inline void typeset(z *obj, int type)
{
    obj->flags=((obj->flags)&0xffff0000)|type;
}
inline int istype(z *obj, int type)
{
    return ((obj->flags&0x0000ffff)==type)?1:0;
}


/*hard objects*/

struct z *obj_null;
struct z *obj_true;
struct z *obj_false;
struct z *obj_ok;
struct z *obj_notok;
struct z *obj_undefined;
struct z *obj_eof;

#define cell_pool_size 65536
//#define cell_pool_size 1024
#define cells_per_pool (unsigned int)(cell_pool_size / (sizeof(struct z)+sizeof(struct z*)))

typedef enum mem_pool_type {cell_pool,bin_pool} mem_pool_type;

typedef struct mem_pool
{
    mem_pool_type type;
    size_t size;
    void *p;
    struct z *freelist;
    struct mem_pool *prev;
    struct mem_pool *next;
} mem_pool;

mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units);
int break_mem_pool(mem_pool *mp);
void mem_pool_expand(mem_pool *mp,mem_pool_type type, size_t size, size_t units);

void gc_init(mem_pool *mp);
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


struct z *bit_oper(struct z *obj,int oper);
struct z *bit_oper2(struct z *obj,struct z *obj2,int oper);
struct z *char_oper(struct z *obj,int oper);
struct z *num_oper(struct z *obj,int oper);
struct z *num_oper2(struct z *obj,struct z *obj2,int oper);
struct z *string_oper(struct z *obj,int oper);
struct z *string_oper2(struct z *obj,struct z *obj2,int oper);
struct z *symbol_oper(struct z *obj,int oper);
struct z *symbol_oper2(struct z *obj,struct z *obj2,int oper);
struct z *regex_oper(struct z *obj,int oper);
struct z *regex_oper2(struct z *obj,struct z *obj2,int oper);

inline struct z *car(struct z *o)
{
    if(o && istype(o,tlist))
    {
        return o->val.list.car;
    }
    return NULL;
}

inline struct z *cdr(struct z *o)
{
    if(o && istype(o,tlist))
    {
        return o->val.list.cdr;
    }
    return NULL;
}

#define caar(obj) car(car(obj))
#define ff(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define fr(obj) car(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define rf(obj) cdr(car(obj))
#define cddr(obj) cdr(cdr(obj))
#define rr(obj) cdr(cdr(obj))
#define caaar(obj) car(car(car(obj)))
#define fff(obj) car(car(car(obj)))
#define caadr(obj) car(car(cdr(obj)))
#define ffr(obj) car(car(cdr(obj)))
#define cadar(obj) car(cdr(car(obj)))
#define frf(obj) car(cdr(car(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define frr(obj) car(cdr(cdr(obj)))
#define cdaar(obj) cdr(car(car(obj)))
#define rff(obj) cdr(car(car(obj)))
#define cdadr(obj) cdr(car(cdr(obj)))
#define rfr(obj) cdr(car(cdr(obj)))
#define cddar(obj) cdr(cdr(car(obj)))
#define rrf(obj) cdr(cdr(car(obj)))
#define cdddr(obj) cdr(cdr(cdr(obj)))
#define rrr(obj) cdr(cdr(cdr(obj)))
#define caaaar(obj) car(car(car(car(obj))))
#define ffff(obj) car(car(car(car(obj))))
#define caaadr(obj) car(car(car(cdr(obj))))
#define fffr(obj) car(car(car(cdr(obj))))
#define caadar(obj) car(car(cdr(car(obj))))
#define ffrf(obj) car(car(cdr(car(obj))))
#define caaddr(obj) car(car(cdr(cdr(obj))))
#define ffrr(obj) car(car(cdr(cdr(obj))))
#define cadaar(obj) car(cdr(car(car(obj))))
#define frff(obj) car(cdr(car(car(obj))))
#define cadadr(obj) car(cdr(car(cdr(obj))))
#define frfr(obj) car(cdr(car(cdr(obj))))
#define caddar(obj) car(cdr(cdr(car(obj))))
#define frrf(obj) car(cdr(cdr(car(obj))))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))
#define frrr(obj) car(cdr(cdr(cdr(obj))))
#define cdaaar(obj) cdr(car(car(car(obj))))
#define rfff(obj) cdr(car(car(car(obj))))
#define cdaadr(obj) cdr(car(car(cdr(obj))))
#define rffr(obj) cdr(car(car(cdr(obj))))
#define cdadar(obj) cdr(car(cdr(car(obj))))
#define rfrf(obj) cdr(car(cdr(car(obj))))
#define cdaddr(obj) cdr(car(cdr(cdr(obj))))
#define rfrr(obj) cdr(car(cdr(cdr(obj))))
#define cddaar(obj) cdr(cdr(car(car(obj))))
#define rrff(obj) cdr(cdr(car(car(obj))))
#define cddadr(obj) cdr(cdr(car(cdr(obj))))
#define rrfr(obj) cdr(cdr(car(cdr(obj))))
#define cdddar(obj) cdr(cdr(cdr(car(obj))))
#define rrrf(obj) cdr(cdr(cdr(car(obj))))
#define cddddr(obj) cdr(cdr(cdr(cdr(obj))))
#define rrrr(obj) cdr(cdr(cdr(cdr(obj))))

struct z *list_front(struct z *l);
struct z *list_back(struct z *l);
struct z *list_at(struct z *l, struct z *obj);
struct z *list_at2(struct z *l, size_t pos);

struct z *list_pushfront(struct z *l, struct z *obj);
struct z *list_popfront(struct z *l);
struct z *list_pushback(struct z *l, struct z *obj);
struct z *list_popback(struct z *l);

void list_insert(struct z *l, size_t pos, struct z *obj);
void list_insert2(struct z *l, size_t pos, size_t n, struct z *obj);
void list_erase(struct z *l, size_t pos);
void list_erase2(struct z *l, size_t pos1, size_t pos2);
void list_swap(struct z *l1, struct z *l2);
void list_clear(struct z *l);

struct z *list_size(struct z *l);
size_t list_size2(struct z *l);
struct z *list_maxsize(struct z *l);
size_t list_maxsize2(struct z *l);
struct z *list_isempty(struct z *l);
int list_isempty2(struct z *l);
void list_reverse(struct z *l, size_t n);

//remove
//removeif
//splice
//sort
//merge
//unique

struct z *vector_size(struct z *v);
size_t vector_size2(struct z *v);
struct z *vector_maxsize(struct z *v);
size_t vector_maxsize2(struct z *v);
struct z *vector_isempty(struct z *v);
int vector_isempty2(struct z *v);
void vector_reserve(struct z *v, size_t n);

void vector_clear(struct z *v);
struct z *vector_resize(struct z *v, struct z *obj);
struct z *vector_resize2(struct z *v, size_t n);

void vector_insert(struct z *v, size_t pos, struct z *obj);
void vector_insert2(struct z *v, size_t pos, size_t n, struct z *obj);
void vector_erase(struct z *v, size_t pos);
void vector_erase2(struct z *v, size_t pos1, size_t pos2);
void vector_swap(struct z *v1, struct z *v2);

struct z *vector_at(struct z *v, struct z *obj);
struct z *vector_at2(struct z *v, size_t n);
struct z *vector_front(struct z *v);
struct z *vector_back(struct z *v);
struct z *vector_pushback(struct z *v, struct z *obj);
struct z *vector_popback(struct z *v);


void hashtable_resolve(struct z *ht);
size_t hashtable_hash(struct z *str);
size_t hashtable_hash2(void *key);
void hashtable_dump(struct z *ht, FILE *out);

struct z *hashtable_size(struct z *ht);
size_t hashtable_size2(struct z *ht);
struct z *hashtable_maxsize(struct z *ht);
size_t hashtable_maxsize2(struct z *ht);
struct z *hashtable_isempty(struct z *ht);
int hashtable_isempty2(struct z *ht);

void hashtable_insert(struct z *ht, struct z *key, struct z *val);
void hashtable_insert2(struct z *ht, struct z *list);
void hashtable_erase(struct z *ht, struct z *key);
void hashtable_erase2(struct z *ht, size_t pos);
void hashtable_swap(struct z *ht1, struct z *ht2);

void hashtable_clear(struct z *ht);
struct z *hashtable_find(struct z *ht, struct z *key);
struct z *hashtable_at(struct z *ht, size_t pos);
struct z *hashtable_count(struct z *ht, struct z *key);
size_t hashtable_count2(struct z *ht, struct z *key);

#endif
