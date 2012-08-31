/*
Copyright (c) 2010-2012 Sriram Srinivasan.

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


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


#include <string>
#include <list>
#include <vector>
#include <map>


using namespace std;

struct z;
struct mem_pool;

struct z *obj_null;
struct z *obj_true;
struct z *obj_false;
struct z *obj_undefined;
struct z *obj_eof;

struct z *obj_ok;
struct z *obj_notok;

struct z *obj_def;
struct z *obj_undef;
struct z *obj_cond;
struct z *obj_if;
struct z *obj_else;
struct z *obj_do;
struct z *obj_fn;
struct z *obj_quote;
struct z *obj_quasiquote;
struct z *obj_unquote;
struct z *obj_unquotesplice;
struct z *obj_with;
struct z *obj_and;
struct z *obj_or;
struct z *obj_not;

inline void type_init(struct z *obj, int type);
inline int type_get(struct z *obj);
inline void type_set(struct z *obj, int t);
inline int istype(struct z *obj, int type);

struct mem_pool *make_mem_pool(size_t units);
void mem_pool_expand(struct mem_pool *mp, size_t units);
void mem_pool_break(struct mem_pool *mp);

struct z *obj_make(mem_pool *mp);
void obj_break(mem_pool *mp, struct z *o);

struct z *make_char(mem_pool *mp, int v);
struct z *make_fixnum(mem_pool *mp, long int v);
struct z *make_flonum(mem_pool *mp, double v);
struct z *make_string1(mem_pool *mp, void *v, size_t s);
struct z *make_string2(mem_pool *mp, char *s);
struct z *make_string3(mem_pool *mp, std::string s);
struct z *make_symbol1(mem_pool *mp, void *v, size_t s);
struct z *make_symbol2(mem_pool *mp, char *s);
struct z *make_symbol3(mem_pool *mp, size_t c);
struct z *make_regex1(mem_pool *mp, void *v, size_t s);
struct z *make_regex2(mem_pool *mp, char *s);
struct z *make_list(mem_pool *mp, struct z *pcar, struct z *pcdr);
struct z *make_vector(mem_pool *mp, size_t s);
struct z *make_hashmap(mem_pool *mp, size_t s);
struct z *make_iport(mem_pool *mp, FILE *in);
struct z *make_oport(mem_pool *mp, FILE *out);
struct z *make_fn(mem_pool *mp, struct z *scope, struct z *body);
struct z *make_nfn(mem_pool *mp, z *(*pnfn)(struct z *scope));

inline struct z *list_car(struct z *l);
inline struct z *list_cdr(struct z *l);

#define list_caar(obj)   list_car(list_car(obj))
#define list_cadr(obj)   list_car(list_cdr(obj))
#define list_cdar(obj)   list_cdr(list_car(obj))
#define list_cddr(obj)   list_cdr(list_cdr(obj))
#define list_caaar(obj)   list_car(list_caar(obj))
#define list_caadr(obj)   list_car(list_cadr(obj))
#define list_cadar(obj)   list_car(list_cdar(obj))
#define list_caddr(obj)   list_car(list_cddr(obj))
#define list_cdaar(obj)   list_cdr(list_caar(obj))
#define list_cdadr(obj)   list_cdr(list_cadr(obj))
#define list_cddar(obj)   list_cdr(list_cdar(obj))
#define list_cdddr(obj)   list_cdr(list_cddr(obj))


struct z *list_push_front(struct z *l, struct z *obj);
struct z *list_pop_front(struct z *l);
struct z *list_push_back(struct z *l, struct z *obj);
struct z *list_pop_back(struct z *l);
struct z *list_push_at(struct z *l, struct z *obj, size_t pos);
struct z *list_pop_at(struct z *l, size_t pos);
size_t list_size(struct z *l);
void list_reverse(struct z *l, size_t n);
void list_clear(struct z *l);

struct z *vector_push(struct z *v, struct z *obj);
struct z *vector_pop(struct z *v);
struct z *vector_pop_at(struct z *v, struct z *obj);
size_t vector_size(struct z *v);
void vector_reverse(struct z *v, size_t n);
void vector_clear(struct z *v);

struct z *hashmap_push(struct z *h, struct z *obj, char *key);
struct z *hashmap_pop(struct z *h, char *key);
size_t hashmap_hash(char *key);
size_t hashmap_size(struct z *h);
void hashmap_clear(struct z *h);


int is_delim(int ch);
int is_term_delim(int ch);
int is_oper(int ch);
int is_spl(int ch);
int is_hex(int ch);


struct z *zread(mem_pool *mp, FILE *in);
int zread_next_char(FILE *in);
void zread_skipws(FILE *in);
struct z *zread_list(mem_pool *mp, FILE *in);
struct z *zread_vector(mem_pool *mp, FILE *in);
struct z *zread_hashmap(mem_pool *mp, FILE *in);


struct z *zeval(struct z *exp, struct z *scope);


void zprint(FILE *out, struct z *exp);
void zprint_list(FILE *out, struct z *exp);
void zprint_vector(FILE *out, struct z *exp);
void zprint_hashmap(FILE *out, struct z *exp);


void repl(mem_pool *mp, FILE *in, FILE *out, struct z *scope);


int main(int agrc, char *agrv[]);
