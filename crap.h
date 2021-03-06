
#define TYPEGET(x,y) ((x->flags&y)==y)?1:0
#define TYPESET(x,y) x->flags=x->flags|y
#define GCGET(s) s->flags&0xf0000000
#define GCSET(s,t) s->flags=s->flags&(t|0x0fffffff)

/*hard objects*/

z *obj_null;
z *obj_true;
z *obj_false;
z *obj_quote;
z *obj_def;
z *obj_set;
z *obj_ok;
z *obj_notok;
z *obj_if;
z *obj_fn;
z *obj_do;
z *obj_cond;
z *obj_else;
z *obj_let;
z *obj_and;
z *obj_or;
z *obj_eof;
z *obj_quasiquote;
z *obj_unquote;
z *obj_unquotesplice;

z *symbols;
z *gtray;
z *empty_scope;
z *global_scope;

/*dynamic stack*/

struct stacknode
{
  void *p;
  struct stacknode *link;
};

struct dstack
{
  size_t size;
  struct stacknode *head;
  struct stacknode *list;
};

int dstack_push(struct dstack *st, void *data);
void *dstack_pop(struct dstack *st);
void dstack_pop2(struct dstack *st);
void dstack_init(struct dstack *st);

/*dynamic stack*/

/*dynamic hashtable*/

#define HASHSIZE 1009
#define KEYSIZE 1024
#define MULTIPLIER 107

typedef struct hashnode
{
    void *value;
    char *key;
    struct hashnode *next;
} hashnode;

hashnode *htable_create();
void htable_init(hashnode *ht);
void htable_put(hashnode *ht, char *str, void *v);
void *htable_get(hashnode *ht, char *str);
void htable_resolve(hashnode *ht, int loc, char *str, void *v);
int htable_remove(hashnode *ht, char *str);
int htable_hash(char *str);
int htable_hash1(char *str);
int htable_hash2(char *str,int multiplier,int items);
void htable_dump(hashnode *ht, FILE *out);

/*dynamic hashtable*/


/*gc stuff*/

typedef enum mem_pool_type {cell_pool,bin_pool} mem_pool_type;

typedef struct mem_pool
{
    mem_pool_type type;
    size_t size;
    void *p;
    struct mem_pool *prev;
    struct mem_pool *next;
} mem_pool;

mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units);
int break_mem_pool(mem_pool *mp);

void init_cell_pool(mem_pool *mp, size_t s);

#define cell_pool_size 65536
static size_t cells_per_pool = cell_pool_size / (sizeof(struct z)+sizeof(struct z*));

static int gc_on = 1;
static int gcmarktype = 1;
static size_t low_water_mark = 5000;

void gc_init();
void gc_shutdown();
void gc_gc();

static struct dstack *root_objects;

void push_root(struct z *o);
struct z *pop_root(struct z *o);

struct z *make_obj();
void break_obj(struct z *o);



/*max digits a number can have in input*/
#define NUMBUFFSIZE 33
/*max chararcters a string can have in input*/
#define STRBUFFSIZE 1025

/*scheme functions*/

inline struct z *car(struct z *o)
{
    if(o && TYPEGET(o,tpair))
    {
        return o->val.pair.car;
    }
    return 0;
}

inline struct z *cdr(struct z *o)
{
    if(o && TYPEGET(o,tpair))
    {
        return o->val.pair.cdr;
    }
    return 0;
}

/*inline struct z *index(struct z *o, size_t n)
{
    struct z *p1,*p2;
    if(o && TYPEGET(o,tpair))
    {
        for(p1=o->val.pair.car,p2=o->val.pair.cdr;((n>0) && (p2!=0));n--,p1=car(p2),p2=cdr(p2));
            return p1;
    }
    return 0;
}*/

#define caar(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define cddr(obj) cdr(cdr(obj))
#define caaar(obj) car(car(car(obj)))
#define caadr(obj) car(car(cdr(obj)))
#define cadar(obj) car(cdr(car(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cdaar(obj) cdr(car(car(obj)))
#define cdadr(obj) cdr(car(cdr(obj)))
#define cddar(obj) cdr(cdr(car(obj)))
#define cdddr(obj) cdr(cdr(cdr(obj)))
#define caaaar(obj) car(car(car(car(obj))))
#define caaadr(obj) car(car(car(cdr(obj))))
#define caadar(obj) car(car(cdr(car(obj))))
#define caaddr(obj) car(car(cdr(cdr(obj))))
#define cadaar(obj) car(cdr(car(car(obj))))
#define cadadr(obj) car(cdr(car(cdr(obj))))
#define caddar(obj) car(cdr(cdr(car(obj))))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))
#define cdaaar(obj) cdr(car(car(car(obj))))
#define cdaadr(obj) cdr(car(car(cdr(obj))))
#define cdadar(obj) cdr(car(cdr(car(obj))))
#define cdaddr(obj) cdr(car(cdr(cdr(obj))))
#define cddaar(obj) cdr(cdr(car(car(obj))))
#define cddadr(obj) cdr(cdr(car(cdr(obj))))
#define cdddar(obj) cdr(cdr(cdr(car(obj))))
#define cddddr(obj) cdr(cdr(cdr(cdr(obj))))


/*core function declarations*/

inline int is_null(z *o){return TYPEGET(o,tnull);}
inline int is_pair(z *o){return TYPEGET(o,tpair);}
inline int is_bool(z *o){return TYPEGET(o,tbool);}
inline int is_false(z *o){if(TYPEGET(o,tbool)){return (o->val.i)?1:0;}}
inline int is_true(z *o){return !is_false(o);}
inline int is_inum(z *o){return TYPEGET(o,tinum);}
inline int is_num(z *o){return TYPEGET(o,tnum);}
inline int is_string(z *o){return TYPEGET(o,tstring);}
inline int is_symbol(z *o){return TYPEGET(o,tsymbol);}
inline int is_nfn(z *o){return TYPEGET(o,tnfn);}
inline int is_fn(z *o){return TYPEGET(o,tfn);}
inline int is_iport(z *o){return TYPEGET(o,tiport);}
inline int is_oport(z *o){return TYPEGET(o,toport);}
inline int is_eof(z *o){return TYPEGET(o,teof);}

z *make_bool(int v);
z *make_string(char *v);
z *make_num(double v);
z *make_inum(long int v);
z *make_symbol(z *scope,char *v);

z *cons(z *pcar, z *pcdr);

z *make_nfn(z *(*pnfn)(struct z *scope, struct z *args));
z *make_fn(z *scope, z *args, z *body);

z *make_iport(FILE *in);
z *make_oport(FILE *out);

inline int is_delim(int ch)
{
    return isspace(ch) || ch==0 || ch=='(' ||
                ch==')' || ch=='"' || ch==';';
}

inline int is_init(int ch)
{
    return isalpha(ch) || ch=='*' || ch=='/' ||
                ch=='>' || ch=='<' || ch=='=' ||
                ch=='?' || ch=='!' || ch=='_' ||
                ch=='%' || ch=='.' || ch=='~' ||
                ch=='$' || ch=='^' || ch=='&' ||
                ch=='|' || ch==':';
}

inline int is_hex(int ch)
{
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}

int zread_next_char(FILE *in);
void zread_skipws(FILE *in);
void zread_skipstr(FILE *in, char *s);
z *zread(z *scope,FILE *in);
z *zread_pair(z *scope,FILE *in);

inline int is_self_evalable(z *o)
{
    return is_bool(o)||is_inum(o)||
           is_num(o)||is_string(o);
}

inline int is_tagged(z *o, z *tag)
{
    z *pcar;
    if(is_pair(o))
    {
        pcar=car(o);
        return is_symbol(pcar) && (pcar==tag);
    }
    return 0;
}

/*
environment
    packages
        modules
            symbols

__global__
    __main__
        tray


(import "stdlib->asdf" "stdlib->(ss@s bb cc d@dded e f)" "stdio" "math->")
(export "(func1 func2 func3)")
(def (codes name)
  (def codelist null)
  (let loop ((c (car name)))
    (append codelist (order c))
    (loop (set codelist (cadr name))))))


(import "stdlib.asdf" "stdlib.(ss@s bb cc d@dded e f)" "stdio" "math.*")
'(1 2 . (2 3))
'(1 2 : (2 3))



tray

scope = null null
        null



*/


z *make_env();
z *setup_scope();
inline z *get_vars(z *o){return car(o);}
inline z *get_vals(z *o){return cdr(o);}
z *var_bind(z *scope, z *pvar, z *pval);
z *var_val(z *scope, z *pvar);

z *parse_def_var(z *o);
z *parse_def_val(z *o);
z *parse_set_var(z *o);
z *parse_set_val(z *o);


z *zeval(z *scope, z *exp);

void zprint(FILE *out, z *exp);
void zprint_pair(FILE *out,z *exp);


void init();


/*builtin native function declarations*/

z *is_null_nfn(z *args);
z *is_pair_nfn(z *args);
z *is_bool_nfn(z *args);
z *is_false_nfn(z *args);
z *is_true_nfn(z *args);
z *is_inum_nfn(z *args);
z *is_num_nfn(z *args);
z *is_string_nfn(z *args);
z *is_symbol_nfn(z *args);
z *is_nfn_nfn(z *args);
z *is_fn_nfn(z *args);
z *is_iport_nfn(z *args);
z *is_oport_nfn(z *args);
z *is_eof_nfn(z *args);

z *cons_nfn(z *args);
z *car_nfn(z *args);

z *cdr_nfn(z *args);
z *set_car_nfn(z *args);
z *set_cdr_nfn(z *args);

z *list_nfn(z *args);
z *apply_nfn(z *args);

z *add_nfn(z *args);
z *subtract_nfn(z *args);
z *multiply_nfn(z *args);
z *quotient_nfn(z *args);
z *remainder_nfn(z *args);

z *equal_nfn(z *args);
z *less_nfn(z *args);
z *more_nfn(z *args);
z *less_equal_nfn(z *args);
z *more_equal_nfn(z *args);

z *interaction_scope_nfn(z *args);
z *null_scope_nfn(z *args);
z *scope_nfn(z *args);
z *error_nfn(z *args);

z *zread_char_nfn(z *args);
z *zread_next_char_nfn(z *args);
z *zread_nfn(z *args);
z *zeval_nfn(z *args);
z *zprint_char_nfn(z *args);
z *zprint_nfn(z *args);

z *load_nfn(z *args);

z *is_iport_nfn(z *args);
z *is_oport_nfn(z *args);
z *open_iport_nfn(z *args);
z *close_iport_nfn(z *args);
z *open_oport_nfn(z *args);
z *close_oport_nfn(z *args);

/*
set
=
eq?
==
===
abs
acos
align
asin
atan
ceil
close-all-ports
cos
count
empty?
null?
even?
exp
pow
filter
floor
round
gcd
int?
lcm
log
log10
logn
max
min
negative?
num->str
num?
odd?
positive?
random
read-line
read-string
sin
sqrt
tan
truncate
zero?
*/
