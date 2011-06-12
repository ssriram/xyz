/*

dev interpreter without much features
mostly crappy one.

*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

/*max digits a number can have in input*/
#define numbuffmax 33
/*max chararcters a string can have in input*/
#define strbuffmax 1025
/*normal string can have MAX(size_t) as length*/

typedef enum
{
    teof,tnull,tbool,
    tchar,tnum,tstring,
    tpair,tsymbol,
    tnfn,tfn,
    tiport,toport,
    tsyntax,tclosure,
    tcontinuation,tmacro,
    tpromise
} types;

/*core data type*/

typedef struct s
{
    types type;
    struct
    {
        char flag;
        struct s *prev;
        struct s *next;
    } gc;
    union
    {
        char c;
        int b;
        double num;
        char *sym;
        char *str;
        struct s *(*nfn)(struct s *args);
        struct s *link;
        FILE *istream;
        FILE *ostream;
        struct
        {
            struct s *car;
            struct s *cdr;
        } pair;
        struct
        {
            struct s *scope;
            struct s *args;
            struct s *body;
        } fn;
    } val;
} s;


/*scheme functions*/

#define car(obj) (obj->val.pair.car)
#define cdr(obj) (obj->val.pair.cdr)

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


/*hard objects*/

s *obj_null;
s *obj_true;
s *obj_false;
s *obj_quote;
s *obj_def;
s *obj_set;
s *obj_ok;
s *obj_notok;
s *obj_if;
s *obj_fn;
s *obj_do;
s *obj_cond;
s *obj_else;
s *obj_let;
s *obj_and;
s *obj_or;
s *obj_eof;
s *obj_quasiquote;
s *obj_unquote;
s *obj_unquotesplice;

s *symbols;
s *null_scope;
s *global_scope;

s *make_bool(int v);
s *make_char(char v);
s *make_string(char *v);
s *make_num(double v);
s *make_symbol(char *v);
s *make_pair(s *o);

s *cons(s *pcar, s *pcdr);

int is_bool(s *o){return o->type==tbool;}
int is_false(s *o){return o==obj_false;}
int is_true(s *o){return !(o==obj_false);}
int is_char(s *o){return o->type==tchar;}
int is_num(s *o){return o->type==tnum;}
int is_string(s *o){return o->type==tstring;}
int is_pair(s *o){return o->type==tpair;}
int is_symbol(s *o){return o->type==tsymbol;}
int is_nfn(s *o){return o->type==tnfn;}
int is_null(s *o){return o->type==tnull;}
int is_fn(s *o){return o->type==tfn;}
int is_iport(s *o){return o->type==tiport;}
int is_oport(s *o){return o->type==toport;}
int is_eof(s *o){return o==obj_eof;}

s *make_obj()
{
    s *p;
    p=malloc(sizeof(struct s));
    if(!p){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    p->val.c=0;
    p->val.link=0;
    p->val.pair.car=0;
    p->val.pair.cdr=0;
    p->val.nfn=0;
    p->val.fn.scope=0;
    p->val.fn.args=0;
    p->val.fn.body=0;
    p->val.num=0.0;
    return p;
}

s *make_bool(int b)
{
    s *p=make_obj();
    p->type=tbool;
    p->val.b=b;
    return p;
}

s *make_char(char c)
{
    s *p=make_obj();
    p->type=tchar;
    p->val.c=c;
    return p;
}

s *make_num(double d)
{
    s *p=make_obj();
    p->type=tnum;
    p->val.num=d;
    return p;
}

s *make_string(char *str)
{
    s *p=make_obj();
    p->type=tstring;
    p->val.str=malloc(strlen(str)+1);
    if(!p->val.str){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    strcpy(p->val.str,str);
    return p;
}

s *cons(s *pcar, s *pcdr) //cons a.k.a make_pair
{
    s *p=make_obj();
    p->type=tpair;
    p->val.pair.car=pcar;
    p->val.pair.cdr=pcdr;
    return p;
}

s *make_symbol(char *name)
{
    s *p, *sym;
    sym=symbols;
    while(!is_null(sym))
    {
        //printf("%u %s",sym,sym->val.sym);
        if(strcmp(car(sym)->val.sym,name)==0)
        {
            return car(sym);
        }
        sym=cdr(sym);
    }
    p=make_obj();
    p->type=tsymbol;
    p->val.sym=malloc(strlen(name)+1);
    if(!p->val.sym){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    strcpy(p->val.sym,name);
    symbols=cons(p,symbols);
    return p;
}

s *make_nfn(s *(*pnfn)(s *args))
{
    s *p=make_obj();
    p->type=tnfn;
    p->val.nfn=pnfn;
    return p;
}

s *make_fn(s *args, s *body, s *scope)
{
    s *p=make_obj();
    p->type=tfn;
    p->val.fn.args=args;
    p->val.fn.body=body;
    p->val.fn.scope=scope;
    return  p;
}

s *make_env();
s *setup_scope();
s *spark_read(FILE *in);
s *spark_eval(s *exp, s *env);
void print(FILE *out, s *obj);

s *make_iport(FILE *in)
{
    s *p=make_obj();
    p->type=tiport;
    p->val.istream=in;
    return p;
}

int next_char(FILE *in);

s *make_oport(FILE *out)
{
    s *p=make_obj();
    p->type=toport;
    p->val.ostream=out;
    return p;
}


/*lib functions*/

s *plus_nfn(s *args)
{
    double ans=0.0;
    while(!is_null(args))
    {
        ans+=(car(args)->val.num);
        args=cdr(args);
    }
    return make_num(ans);
}

s *minus_nfn(s *args)
{
    double ans=(car(args)->val.num);
    args=cdr(args);
    while(!is_null(args))
    {
        ans-=(car(args)->val.num);
        args=cdr(args);
    }
    return make_num(ans);
}

s *multiply_nfn(s *args)
{
    double ans=1.0;
    while(!is_null(args))
    {
        ans*=(car(args)->val.num);
        args=cdr(args);
    }
    return make_num(ans);
}

s *quotient_nfn(s *args)
{
    if((cadr(args)->val.num !=0.0))
        return make_num(car(args)->val.num / cadr(args)->val.num);
    else fprintf(stderr,"error: divide by zero\n");
    return obj_notok;
    //exit(1);
}

s *remainder_nfn(s *args)
{
    if((cadr(args)->val.num !=0.0))
        return make_num(((int)(cadr(args)->val.num) % (int)(car(args)->val.num)));
    else fprintf(stderr,"error: modulo by zero\n");
    return obj_notok;
    //exit(1);
}

s *is_null_nfn(s *args)
{
    return is_null(car(args))?obj_true:obj_false;
}

s *is_bool_nfn(s *args)
{
    return is_bool(car(args))?obj_true:obj_false;
}

s *is_symbol_nfn(s *args)
{
    return is_symbol(car(args))?obj_true:obj_false;
}

s *is_num_nfn(s *args)
{
    return is_num(car(args))?obj_true:obj_false;
}

s *is_char_nfn(s *args)
{
    return is_char(car(args))?obj_true:obj_false;
}

s *is_string_nfn(s *args)
{
    return is_string(car(args))?obj_true:obj_false;
}

s *is_pair_nfn(s *args)
{
    return is_pair(car(args))?obj_true:obj_false;
}

s *is_fn_nfn(s *args)
{
    return (is_nfn(car(args))||is_fn(car(args)))?obj_true:obj_false;
}

s *char_num_nfn(s *args)
{
    return make_num((car(args)->val.c));
}

s *num_char_nfn(s *args)
{
    return make_char((char)((int)(car(args)->val.num)));
}

s *num_string_nfn(s *args)
{
    char numbuff[numbuffmax];
    sprintf(numbuff,"%f",(car(args))->val.num);
    return make_string(numbuff);
}

s *string_num_nfn(s *args)
{
    char *ep;
    return make_num(strtod((car(args)->val.str),&ep));
}

s *symbol_string_nfn(s *args)
{
    return make_string((car(args)->val.sym));
}

s *string_symbol_nfn(s *args)
{
    return make_symbol((car(args)->val.str));
}

s *equal_num_nfn(s *args)
{
    double v;

    v=(car(args)->val.num);
    while(!is_null(args=cdr(args)))
    {
        if(v!=(car(args)->val.num))
            return obj_false;
    }
    return obj_true;
}

s *equal_nfn(s *args)
{
    s *o1,*o2;
    o1=car(args);
    o2=cadr(args);

    if(o1->type!=o2->type)
    {
        return obj_false;
    }
    switch(o1->type)
    {
        case tnum:
            return (o1->val.num==o2->val.num)?obj_true:obj_false;
            break;
        case tchar:
            return (o1->val.c==o2->val.c)?obj_true:obj_false;
            break;
        case tstring:
            return (strcmp(o1->val.str,o2->val.str)==0)?obj_true:obj_false;
            break;
        default:
            return (o1==o2)?obj_true:obj_false;
    }
}

s *lessthan_nfn(s *args)
{
    double next,prev;

    prev=(car(args)->val.num);
    while(!is_null(args=cdr(args)))
    {
        next=car(args)->val.num;
        if(prev<next){prev=next;}
        else return obj_false;
    }
    return obj_true;
}

s *morethan_nfn(s *args)
{
    double next,prev;

    prev=(car(args)->val.num);
    while(!is_null(args=cdr(args)))
    {
        next=car(args)->val.num;
        if(prev>next){prev=next;}
        else return obj_false;
    }
    return obj_true;
}

s *cons_nfn(s *args)
{
    return cons(car(args),cadr(args));
}

s *car_nfn(s *args)
{
    return caar(args);
}

s *cdr_nfn(s *args)
{
    return cdar(args);
}

s *set_car_nfn(s *args)
{
    car(car(args))=cadr(args);
    return obj_ok;
}

s *set_cdr_nfn(s *args)
{
    cdr(car(args))=cadr(args);
    return obj_ok;
}

s *list_nfn(s *args)
{
    return args;
}

s *apply_nfn(s *args)
{
    fprintf(stderr,"error: illegal state in native function 'apply'\n");
    //return obj_notok;
    exit(1);
}

s *interaction_scope_nfn(s *o)
{
    return global_scope;
}

s *null_scope_nfn(s *o)
{
    return setup_scope();
}

s *scope_nfn(s *o)
{
    return make_env();
}

s *eval_nfn(s *args)
{
    fprintf(stderr,"error: illegal state in native function 'eval'\n");
    //return obj_notok;
    exit(1);
}

//file io

s *load_nfn(s *o)
{
    char *filename;
    FILE *in;
    s *exp;
    s *result=0;

    filename = car(o)->val.str;
    in = fopen(filename, "r");
    if (!in)
    {
        fprintf(stderr, "error: cannot open to load file \"%s\"", filename);
        return obj_notok;
        //exit(1);
    }
    while ((exp = spark_read(in))!=0)
    {
        printf("*> ");
        print(stdout,exp);
        printf("\n");
        result = spark_eval(exp, global_scope);
        print(stdout,result);
        printf("\n");

    }
    fclose(in);
    return result;
}


s *open_iport_nfn(s *o)
{
    char *filename;
    FILE *in;

    filename = car(o)->val.str;
    in = fopen(filename, "r");
    if (!in) {
        fprintf(stderr, "error: could not open iport on \"%s\"\n", filename);
        //exit(1);
        return obj_notok;
    }
    return make_iport(in);
}


s *is_iport_nfn(s *o){return is_iport(car(o))?obj_true:obj_false;}

s *close_iport_nfn(s *o)
{
    int result;
    result = fclose(car(o)->val.istream);
    if (!result)
    {
        fprintf(stderr, "error: could not close iport\n");
        //exit(1);
        return obj_notok;
    }
    return obj_ok;
}

s *read_nfn(s *o)
{
    FILE *in;
    s *result;

    in=is_null(o)?stdin:car(o)->val.istream;
    result=spark_read(in);
    return (!result)?obj_eof:result;
}

s *read_char_nfn(s *o)
{
    FILE *in;
    int result;

    in = is_null(o)?stdin:car(o)->val.istream;
    result = getc(in);
    return (!result)?obj_eof:make_char(result);
}

s *next_char_nfn(s *o)
{
    FILE *in;
    int result;

    in = is_null(o)?stdin:car(o)->val.istream;
    result = next_char(in);
    return (!result)?obj_eof:make_char(result);
}

s *is_eof_nfn(s *o)
{
    return is_eof(car(o))?obj_true:obj_false;
}


s *open_oport_nfn(s *o)
{
    char *filename;
    FILE *out;

    filename = car(o)->val.str;
    out = fopen(filename, "w");
    if (!out)
    {
        fprintf(stderr, "error: could not open oport for file \"%s\"\n", filename);
        //exit(1);
        return obj_notok;
    }
    return make_oport(out);
}


s *is_oport_nfn(s *o){return is_oport(car(o))?obj_true:obj_false;}

s *close_oport_nfn(s *o)
{
    int result;

    result = fclose(car(o)->val.ostream);
    if (!result)
    {
        fprintf(stderr, "error: could not close oport\n");
        //exit(1);
        return obj_notok;
    }
    return obj_ok;
}

s *print_char_nfn(s *o)
{
    s *character;
    FILE *out;

    character = car(o);
    o = cdr(o);
    out = is_null(o)?stdout:car(o)->val.ostream;
    putc(character->val.c, out);
    fflush(out);
    return obj_ok;
}

s *print_nfn(s *o)
{
    s *exp;
    FILE *out;

    exp = car(o);
    o = cdr(o);
    out = is_null(o) ?stdout :car(o)->val.ostream;
    print(out, exp);
    fflush(out);
    return obj_ok;
}

s *error_nfn(s *o)
{
    printf("*spark error*: ");
    while (!is_null(o)) {
        print(stderr, car(o));
        fprintf(stderr, " ");
        o = cdr(o);
    };
    //exit(1);
    fprintf(stderr,"\n");
    return obj_notok;
}




/*scope functions*/

s *outer_scope(s *o){return cdr(o);}
s *first_scope(s *o){return car(o);}

s *make_scope(s *pvars, s *pvals)
{
    return cons(pvars,pvals);
}

s *vars(s *o){return car(o);}
s *vals(s *o){return cdr(o);}

void var_bind(s *pvars, s *pvals, s *scope)
{
    car(scope)=cons(pvars,car(scope));
    cdr(scope)=cons(pvals,cdr(scope));
}

s *extend(s *pvars, s *pvals, s *scope)
{
    return cons(make_scope(pvars,pvals),scope);
}

s *varval(s *pvar, s *scope)
{
    s *scope1, *pvars, *pvals;
    while(!is_null(scope))
    {
        scope1=first_scope(scope);
        pvars=vars(scope1);
        pvals=vals(scope1);
        while(!is_null(pvars))
        {
            if(pvar==car(pvars))
            {
                return car(pvals);
            }
            pvars = cdr(pvars);
            pvals = cdr(pvals);
        }
        scope=outer_scope(scope);
    }
    fprintf(stderr,"error: undefined variable\n");
    //exit(1);
    return obj_notok;

}

s *setvar(s *pvar, s *pval, s *scope)
{
    s *scope1, *pvars, *pvals;
    while(!is_null(scope))
    {
        scope1=first_scope(scope);
        pvars=vars(scope1);
        pvals=vals(scope1);
        while(!is_null(pvars))
        {
            if(pvar==car(pvars))
            {
                car(pvals)=pval;
                return obj_ok;
            }
            pvars=cdr(pvars);
            pvals=cdr(pvals);
        }
        scope=outer_scope(scope);
    }
    fprintf(stderr,"error: undefined variable\n");
    //exit(1);
    return obj_notok;
}

s *defvar(s *pvar, s *pval, s *scope)
{
    s *scope1, *pvars, *pvals;
    scope1=first_scope(scope);
    pvars=vars(scope1);
    pvals=vals(scope1);
    while(!is_null(pvars))
    {
        if(pvar==car(pvars))
        {
            car(pvals)=pval;
            return obj_ok;
        }
        pvars=cdr(pvars);
        pvals=cdr(pvals);
    }
    var_bind(pvar,pval,scope1);
    return obj_ok;
}

s *setup_scope()
{
    s *global;
    global=extend(obj_null,obj_null,null_scope);
    return global;
}

void populate_env(s *scope)
{
#define LOAD_NFN(NAME, FNAME)\
        defvar(make_symbol(NAME),make_nfn(FNAME),scope);

    LOAD_NFN("null?",is_null_nfn);
    LOAD_NFN("bool?",is_bool_nfn);
    LOAD_NFN("num?",is_num_nfn);
    LOAD_NFN("char?",is_char_nfn);
    LOAD_NFN("str?",is_string_nfn);
    LOAD_NFN("sym?",is_symbol_nfn);
    LOAD_NFN("pair?",is_pair_nfn);
    LOAD_NFN("func?",is_fn_nfn);
    LOAD_NFN("char->num",char_num_nfn);
    LOAD_NFN("num->char",num_char_nfn);
    LOAD_NFN("str->num",string_num_nfn);
    LOAD_NFN("num->str",num_string_nfn);
    LOAD_NFN("sym->str",symbol_string_nfn);
    LOAD_NFN("str->sym",string_symbol_nfn);
    LOAD_NFN("+",plus_nfn);
    LOAD_NFN("-",minus_nfn);
    LOAD_NFN("*",multiply_nfn);
    LOAD_NFN("/",quotient_nfn);
    LOAD_NFN("%",remainder_nfn);
    LOAD_NFN("=",equal_num_nfn);
    LOAD_NFN("<",lessthan_nfn);
    LOAD_NFN(">",morethan_nfn);
    LOAD_NFN("cons",cons_nfn);
    LOAD_NFN("car",car_nfn);
    LOAD_NFN("cdr",cdr_nfn);
    LOAD_NFN("car!",set_car_nfn);
    LOAD_NFN("cdr!",set_cdr_nfn);
    LOAD_NFN("list",list_nfn);
    LOAD_NFN("eq?",equal_nfn);
    LOAD_NFN("apply",apply_nfn);

    LOAD_NFN("interaction-env",interaction_scope_nfn);
    LOAD_NFN("null-env",null_scope_nfn);
    LOAD_NFN("new-env",scope_nfn);
    LOAD_NFN("eval",eval_nfn);

    LOAD_NFN("load",load_nfn);
    LOAD_NFN("open-iport",open_iport_nfn);
    LOAD_NFN("close-iport",close_iport_nfn);
    LOAD_NFN("iport?",is_iport_nfn);
    LOAD_NFN("read",read_nfn);
    LOAD_NFN("read-char",read_char_nfn);
    LOAD_NFN("next-char",next_char_nfn);
    LOAD_NFN("eof?",is_eof_nfn);
    LOAD_NFN("open-oport",open_oport_nfn);
    LOAD_NFN("close-oport",close_oport_nfn);
    LOAD_NFN("oport?",is_oport_nfn);
    LOAD_NFN("print",print_nfn);
    LOAD_NFN("print-char",print_char_nfn);

    LOAD_NFN("error",error_nfn);
}

s *make_env()
{
    s *env;
    env=setup_scope();
    populate_env(env);
    return env;
}

void init()
{
    obj_null=make_obj();
    obj_null->type=tnull;
    obj_true=make_obj();
    obj_true->type=tbool;
    obj_true->val.b=1;
    obj_false=make_obj();
    obj_false->type=tbool;
    obj_false->val.b=0;

    symbols=obj_null;
    obj_quote=make_symbol("quote");
    obj_def=make_symbol("def");
    obj_set=make_symbol("set");
    obj_ok=make_symbol("ok");
    obj_notok=make_symbol("not-ok");
    obj_if=make_symbol("if");
    obj_fn=make_symbol("fn");
    obj_do=make_symbol("do");
    obj_cond=make_symbol("cond");
    obj_else=make_symbol("else");
    obj_let=make_symbol("let");
    obj_and=make_symbol("and");
    obj_or=make_symbol("or");
    obj_quasiquote=make_symbol("quasiquote");
    obj_unquote=make_symbol("unquote");
    obj_unquotesplice=make_symbol("unquotesplice");

    null_scope=obj_null;
    global_scope=make_env();
}


/*read*/

s *spark_read(FILE *i);
s *read_pair(FILE *i);

int is_delim(int ch)
{
    return isspace(ch) || ch==0 || ch=='(' ||
                ch==')' || ch=='"' || ch==';';
}

int is_init(int ch)
{
    return isalpha(ch) || ch=='*' || ch=='/' ||
                ch=='>' || ch=='<' || ch=='=' ||
                ch=='?' || ch=='!' || ch=='_' ||
                ch=='%' || ch=='@' || ch=='~' ||
                ch=='$' || ch=='^' || ch=='&' ||
                ch=='|' || ch==':';
}

int next_char(FILE *i)
{
    int ch=getc(i);
    ungetc(ch,i);
    return ch;
}

int is_hex(int ch)
{
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}

void skip_ws(FILE *i)
{
    int ch;
    while((ch=getc(i))!=0)
    {
        /*skip whitespaces*/
        if(isspace(ch)){continue;}
        /*skip multiline comments*/
        else if(ch==';' && (next_char(i)==':'))
        {
            ch=getc(i);
            while((ch=getc(i)) !=0 && (ch!=':' || next_char(i)!=';')){}
            continue;
        }
        /*skip single line comments*/
        else if(ch==';' && (next_char(i)!=':'))
        {
            while((ch=getc(i)) !=0 && ch!='\n'){}
            continue;
        }
        ungetc(ch,i);
        break;
    }
}

void skip_string(FILE *i,char *str)
{
    int ch;
    while(*str!=0)
    {
        ch=getc(i);
        if(ch!=*str)
        {
            fprintf(stderr,"error: unexpected string\n");
            exit(1);
        }
        str++;
    }
}

s *read_pair(FILE *i)
{
    int ch;
    s *pcar, *pcdr;

    skip_ws(i);
    ch=getc(i);

    /*empty list*/
    if(ch==')')
    {
        return obj_null;
    }
    ungetc(ch,i);

    pcar=spark_read(i);
    skip_ws(i);
    ch=getc(i);

    /*pair*/
    if(ch=='.')
    {
        ch=next_char(i);
        if(!is_delim(ch))
        {
            fprintf(stderr,"error: incorrect pair syntax\n");
            //exit(1);
            return obj_notok;
        }
        pcdr=spark_read(i);
        skip_ws(i);
        ch=getc(i);
        if(ch!=')')
        {
            fprintf(stderr,"error: incorrect pair syntax\n");
            //exit(1);
            return obj_notok;
        }
        return cons(pcar,pcdr);
    }
    else
    {
        ungetc(ch,i);
        pcdr=read_pair(i);
        return cons(pcar,pcdr);
    }
}

s *spark_read(FILE *i)
{
    int ch;
    char numbuff[numbuffmax];
    char strbuff[strbuffmax];
    int l=0;
    int sl=0;
    int hex=0;
    int id=0;
    int inum=0;
    char *ep;

#define NUMBUFF(x) if(l < numbuffmax - 1){numbuff[l] = x; l++; numbuff[l] = 0;}
#define STRBUFF(x) if(sl < strbuffmax - 1){strbuff[sl] = x; sl++; strbuff[sl] = 0;} else {fprintf(stderr,"error: string/symbol too long max:%d\n",strbuffmax);exit(1);}

    /*skip whitespaces*/
    skip_ws(i);
    ch=getc(i);

    if(ch=='#')
    {
        ch=getc(i);
        switch(ch)
        {
            /*boolean*/
            case '#':
            {
                ch=getc(i);
                switch(ch)
                {
                    case 't':
                        return obj_true;
                    case 'f':
                        return obj_false;
                    case 'T':
                        return obj_true;
                    case 'F':
                        return obj_false;
                    default:
                        fprintf(stderr,"error: unknown literal\n");
                        //exit(1);
                        return obj_notok;
                }
            }

            /*special characters*/
            case '\\':
            {
                ch=getc(i);
                switch(ch)
                {
                    case 'r':
                        return make_char('\r');
                    case 'n':
                        return make_char('\n');
                    case 't':
                        return make_char('\t');
                    case 's':
                        return make_char(' ');
                    case '\\':
                        return make_char('\\');
                    case '#':
                        return make_char('#');
                    default:
                        fprintf(stderr,"error: invalid spl character\n");
                        //exit(1);
                        return obj_notok;
                }
            }

            /*characters*/
            default:
            {
                if(is_delim(next_char(i)))
                {
                    return make_char(ch);
                }
                else
                {
                    fprintf(stderr,"error: invalid character\n");
                    //exit(1);
                    return obj_notok;
                }
            }
        }
    }

    /*number*/
    else if(isdigit(ch) || ((ch=='-' || ch=='+')&& isdigit(next_char(i))))
    {
        hex=0;
        if(ch=='-' || ch=='+')
        {
            NUMBUFF(ch)
            ch=getc(i);
        }
        if(ch=='0') {ch=getc(i); NUMBUFF('0')}
        if(ch=='x') {ch=getc(i); hex=1; NUMBUFF('x')}
        id=0;
        while(isdigit(ch)||(hex && is_hex(ch)))
        {
            NUMBUFF(ch)
            ch=getc(i);
        }
        if(!hex && ch=='.')
        {
            id=1;
            NUMBUFF(ch)
            ch=getc(i);
            while(isdigit(ch))
            {
                NUMBUFF(ch)
                ch=getc(i);
            }
        }
        if(!hex && (ch=='e' || ch=='E'))
        {
            id=1;
            NUMBUFF(ch)
            ch=getc(i);
            if(ch=='-' || ch=='+')
            {
                NUMBUFF(ch)
                ch=getc(i);
            }
            while(isdigit(ch))
            {
                NUMBUFF(ch)
                ch=getc(i);
            }
            if(ch=='.' && isdigit(next_char(i)))
            {
                NUMBUFF(ch)
                ch=getc(i);
                while(isdigit(ch))
                {
                    NUMBUFF(ch)
                    ch=getc(i);
                }
            }
        }
        if(is_delim(ch))
        {
            ungetc(ch,i);
            /*debug*/
            /*printf("%s\n",numbuff);*/
            switch(id)
            {
                case 0:
                {
                    if(hex) sscanf(numbuff,"%x",&inum);
                    else sscanf(numbuff,"%d",&inum);
                    return make_num((double)inum);
                    break;
                }
                case 1:
                {
                    return make_num(strtod(numbuff,&ep));
                    break;
                }
            }
        }
        else
        {
            fprintf(stderr,"error: invalid number/delimiter\n");
            //exit(1);
            return obj_notok;
        }
    }

    /*symbol a.k.a variable*/
    else if(((ch=='+' || ch=='-') && !isdigit(next_char(i))) || is_init(ch))
    {
        sl=0;
        while(is_init(ch) || isdigit(ch) || ch=='+' || ch=='-')
        {
            STRBUFF(ch)
            //printf("%c\n",ch);
            ch=getc(i);
        }
        if(is_delim(ch))
        {
            ungetc(ch,i);
            //printf("%s\n",strbuff);
            return make_symbol(strbuff);
        }
        else
        {
            fprintf(stderr,"error: invalid symbol\n");
            //exit(1);
            return obj_notok;
        }
    }

    /*string*/
    else if(ch=='"')
    {
        sl=0;
        while((ch=getc(i))!='"')
        {
            if(ch=='\\')
            {
                ch=getc(i);
                switch(ch)
                {
                    case 'n':
                        STRBUFF('\n')
                        continue;
                    case 't':
                        STRBUFF('\t')
                        continue;
                    case 's':
                        STRBUFF(' ')
                        continue;
                    case 'r':
                        STRBUFF('\r')
                        continue;
                    case '"':
                        STRBUFF(ch)
                        continue;
                    case '\\':
                        STRBUFF('\\')
                        continue;
                    case '0':
                        //STRBUFF(0)
                        continue;
                    case 'x':
                    {
                        l=0;inum=0;
                        NUMBUFF('0')
                        NUMBUFF('x')
                        if(is_hex((ch=getc(i)))){NUMBUFF(ch)}
                        else
                        {
                            fprintf(stderr,"error: invalid hex string\n");
                            //exit(1);
                            return obj_notok;
                        }
                        if(is_hex((ch=getc(i)))){NUMBUFF(ch)}
                        else
                        {
                            fprintf(stderr,"error: invalid hex string\n");
                            //exit(1);
                            return obj_notok;
                        }
                        ch=getc(i);
                        if(!ch)
                        {
                            fprintf(stderr,"error: illegal string\n");
                            //exit(1);
                            return obj_notok;
                        }
                        ungetc(ch,i);
                        /*printf("%s",numbuff);*/
                        sscanf(numbuff,"%x",&inum);
                        /*printf("%d %c",inum,inum);*/
                        STRBUFF(inum)
                        continue;
                    }
                    default:
                        fprintf(stderr,"error: improper string termination\n");
                        //exit(1);
                        return obj_notok;
                }

            }
            else if(ch==0)
            {
                fprintf(stderr,"error: improper string termination\n");
                //exit(1);
                return obj_notok;
            }
            else
            {
                STRBUFF((char)ch)
            }
        }
        return make_string(strbuff);
    }

    else if(ch=='(')
    {
        return read_pair(i);
    }

    else if(ch=='\'')
    {
        return cons(obj_quote,cons(spark_read(i),obj_null));
    }
    
    else if(ch=='`')
    {
        return cons(obj_quasiquote,cons(spark_read(i),obj_null));
    }
    
    else if(ch==',')
    {
        struct s *p=obj_unquote;
        ch=next_char(i);
        if(ch=='@')
        {
            ch=getc(i);
            p=obj_unquotesplice;
        }
        else
        {
        }
        return cons(p,cons(spark_read(i),obj_null));
    }

    else if(ch==EOF)
    {
        return 0;
    }

    else
    {
        printf("%d",ch);
        fprintf(stderr,"error: invalid input %c\n",ch);
        //exit(1);
        return obj_notok;
    }
    fprintf(stderr,"error: invalid state\n");
    exit(1);
}

/*eval*/

s *spark_eval(s *o, s *scope);
int is_tagged(s *o, s *t);
s *quotedata(s *o){return cadr(o);}
int is_quoted(s *o){return is_tagged(o,obj_quote);}
int is_selfspark_eval(s *o){return is_bool(o) || is_num(o) || is_char(o) || is_string(o);}

int is_var(s *o){return is_symbol(o);}
int is_set(s *o){return is_tagged(o,obj_set);}
int is_def(s *o){return is_tagged(o,obj_def);}

int is_tagged(s *o, s *t)
{
    s *pcar;
    if(is_pair(o))
    {
        pcar=car(o);
        return is_symbol(pcar) && (pcar==t);
    }
    return 0;
}

s *set_var(s *o){return cadr(o);}
s *set_val(s *o){return caddr(o);}
s *def_var(s *o)
{
    if(is_symbol(cadr(o)))
    {
        return cadr(o);
    }
    else
    {
        return caadr(o);
    }
}

s *make_fndef(s *args,s *body);

s *def_val(s *o)
{
    if(is_symbol(cadr(o)))
    {
        return caddr(o);
    }
    else
    {
        return make_fndef(cdadr(o),cddr(o));
    }
}


s *make_fncall(s *fn, s *args){return cons(fn,args);}
s *fncall_fn(s *o){return car(o);}
s *fncall_args(s *o){return cdr(o);}
s *fncall_farg(s *o){return car(o);}
s *fncall_rarg(s *o){return cdr(o);}
int is_fncall(s *o){return is_pair(o);}
int is_fncall_noargs(s *o){return is_null(o);}

s *fncall_arglist(s *o, s *scope)
{
    if(is_null(o))
    {
        return obj_null;
    }
    else
    {
        return cons(spark_eval(fncall_farg(o),scope),fncall_arglist(fncall_rarg(o),scope));
    }
}

s *if_cond(s *o){return cadr(o);}
s *if_texp(s *o){return caddr(o);}
int is_if(s *o){return is_tagged(o,obj_if);}

s *if_fexp(s *o)
{
    if(is_null(cadddr(o)))
    {
        return obj_false;
    }
    else
    {
        return cadddr(o);
    }
}

s *make_fndef(s *args,s *body)
{
    return cons(obj_fn,cons(args,body));
}
int is_fndef(s *o){return is_tagged(o,obj_fn);}

s *fndef_args(s *o){return cadr(o);}
s *fndef_body(s *o){return cddr(o);}
int is_lexp(s *o){return is_null(cdr(o));}

s *fexp(s *o){return car(o);}
s *rexp(s *o){return cdr(o);}

s *doexp(s *o){return cons(obj_do,o);}
int is_doexp(s *o){return is_tagged(o,obj_do);}
s *doexp_list(s *o){return cdr(o);}

int is_cond(s *o){return is_tagged(o,obj_cond);}
s *cond_clauses(s *o){return cdr(o);}
s *cond_predicate(s *o){return car(o);}
s *cond_action(s *o){return cdr(o);}
int is_cond_else(s *o){return cond_predicate(o)==obj_else;}

s *cond_if(s *c, s *t, s *f)
{
    return cons(obj_if,
                    cons(c,
                            cons(t,
                                    cons(f,obj_null))));
}

s *seq_map(s *o)
{
    if(is_null(o))
    {
        return o;
    }
    else if(is_lexp(o))
    {
        return fexp(o);
    }
    else
    {
        return doexp(o);
    }
}

s *expand_clauses(s *o)
{
    s *f,*r;

    if(is_null(o))
    {
        return obj_false;
    }
    else
    {
        f=car(o);
        r=cdr(o);
        if(is_cond_else(f))
        {
            if(is_null(r))
            {
                return seq_map(cond_action(f));
            }
            else
            {
                fprintf(stderr,"error: else must be last in cond->if\n");
                //exit(1);
                return obj_notok;
            }
        }
        else
        {
            return cond_if(cond_predicate(f),seq_map(cond_action(f)),expand_clauses(r));
        }
    }
}

s *cond_to_if(s *o){return expand_clauses(cond_clauses(o));}

int is_let(s *o){return is_tagged(o,obj_let);}
s *let_all(s *o){return cadr(o);}
s *let_body(s *o){return cddr(o);}
s *let_var(s *o){return car(o);}
s *let_val(s *o){return cadr(o);}
s *let_vars(s *o)
{
    return is_null(o)?obj_null:
                    cons(let_var(car(o)),let_vars(cdr(o)));
}
s *let_vals(s *o)
{
    return is_null(o)?obj_null:
                    cons(let_val(car(o)),let_vals(cdr(o)));
}
s *let_vars_all(s *o){return let_vars(let_all(o));}
s *let_vals_all(s *o){return let_vals(let_all(o));}

s *let_to_fncall(s *o)
{
    return make_fncall(make_fndef(let_vars_all(o),let_body(o)),let_vals_all(o));
}

int is_and(s *o){return is_tagged(o,obj_and);}
int is_or(s *o){return is_tagged(o,obj_or);}

s *and_cond(s *o){return cdr(o);}
s *or_cond(s *o){return cdr(o);}

s *apply_fn(s *o){return car(o);}
s *apply_prep(s *args)
{
    if (is_null(cdr(args)))
    {
        return car(args);
    }
    else
    {
        return cons(car(args),
                    apply_prep(cdr(args)));
    }
}
s *apply_args(s *o)
{
    return apply_prep(cdr(o));
}

/*eval stuff*/

s *eval_set(s *o, s *scope)
{
    return setvar(set_var(o),spark_eval(set_val(o),scope),scope);
}

s *eval_def(s *o, s *scope)
{
    return defvar(def_var(o),spark_eval(def_val(o),scope),scope);
}

s *eval_exp(s *o){return car(o);}
s *eval_env(s *o){return cadr(o);}

s *spark_eval(s *o, s *scope)
{
    s *fn;
    s *args;
    s *result;
tailcall:
    if(is_selfspark_eval(o))
    {
        return o;
    }
    else if(is_var(o))
    {
        return varval(o,scope);
    }
    else if(is_quoted(o))
    {
        return quotedata(o);
    }
    else if(is_def(o))
    {
        return eval_def(o,scope);
    }
    else if(is_set(o))
    {
        return eval_set(o,scope);
    }
    else if(is_if(o))
    {
        o = is_true(spark_eval(if_cond(o),scope))?spark_eval(if_texp(o),scope):spark_eval(if_fexp(o),scope);
        goto tailcall;
    }
    else if(is_fndef(o)) //lambda
    {
        return make_fn(fndef_args(o),fndef_body(o),scope);
    }
    else if(is_doexp(o)) //begin (do ...)
    {
        o=doexp_list(o);
        while(!is_lexp(o))
        {
            spark_eval(fexp(o),scope);
            o=rexp(o);
        }
        o=fexp(o);
        goto tailcall;
    }
    else if (is_cond(o))
    {
        o=cond_to_if(o);
        goto tailcall;
    }
    else if(is_let(o))
    {
        o=let_to_fncall(o);
        goto tailcall;
    }
    else if(is_and(o))
    {
        o=and_cond(o);
        if(is_null(o))
        {
            return obj_true;
        }
        while(!is_lexp(o))
        {
            result=spark_eval(fexp(o),scope);
            if(is_false(result))
            {
                return result;
            }
            o=rexp(o);
        }
        o=fexp(o);
        goto tailcall;
    }
    else if(is_or(o))
    {
        o=or_cond(o);
        if(is_null(o))
        {
            return obj_false;
        }
        while(!is_lexp(o))
        {
            result=spark_eval(fexp(o),scope);
            if(is_false(result))
            {
                return result;
            }
            o=rexp(o);
        }
        o=fexp(o);
        goto tailcall;
    }
    else if(is_fncall(o))
    {
        fn=spark_eval(fncall_fn(o),scope);
        args=fncall_arglist(fncall_args(o),scope);

        //eval_nfn must not be called
        if(is_nfn(fn) && fn->val.nfn==eval_nfn)
        {
            o=eval_exp(args);
            scope=eval_env(args);
            goto tailcall;
        }

        //apply_nfn must not be called
        if(is_nfn(fn) && fn->val.nfn==apply_nfn)
        {
            fn=apply_fn(args);
            args=apply_args(args);
        }

        if(is_nfn(fn))
        {
            return (fn->val.nfn)(args);
        }
        else if(is_fn(fn))
        {
            scope=extend(fn->val.fn.args,args,fn->val.fn.scope);
            o=doexp(fn->val.fn.body);
            goto tailcall;
        }
        else
        {
            fprintf(stderr,"error: invalid function call\n");
            //exit(1);
            return obj_notok;
        }
    }
    else
    {
        fprintf(stderr,"error: cannot eval, invalid symbolic expression\n");
        //exit(1);
        return obj_notok;
    }
    fprintf(stderr,"error: cannot eval, invalid state\n");
    return o;
}

/*print*/

void print(FILE *f,s *obj);
void print_pair(FILE *f,s *obj);

void print_pair(FILE *f,s *obj)
{
    s *pcar, *pcdr;
    pcar=car(obj);
    pcdr=cdr(obj);
    print(f,pcar);
    if(pcdr->type==tpair)
    {
        fprintf(f," ");
        print_pair(f,pcdr);
    }
    else if(pcdr->type==tnull)
    {
        return;
    }
    else
    {
        fprintf(f," . ");
        print(f,pcdr);
    }
}

void print(FILE *f,s *obj)
{
    switch(obj->type)
    {
        case tnull:
            fprintf(f,"()");
            break;
        case tbool:
            fprintf(f,"##%c",obj->val.b?'t':'f');
            break;
        case tchar:
        {
            switch(obj->val.c)
            {
                case '\n':
                    fprintf(f,"%s","#\\n");
                    break;
                case '\t':
                    fprintf(f,"%s","#\\t");
                    break;
                case ' ':
                    fprintf(f,"%s","#\\s");
                    break;
                case '\\':
                    fprintf(f,"%s","#\\\\");
                    break;
                case '#':
                    fprintf(f,"%s","#\\#");
                    break;
                case 'r':
                    fprintf(f,"%s","#\\r");
                    break;
                default:
                    fprintf(f,"#%c",obj->val.c);
            }
            return;
        }
        case tnum:
            fprintf(f,"%f",obj->val.num);
            break;
        case tstring:
            fprintf(f,"\"%s\"",obj->val.str);
            break;
        case tsymbol:
            fprintf(f,"%s",obj->val.sym);
            break;
        case tpair:
            fprintf(f,"(");
            print_pair(f,obj);
            fprintf(f,")");
            break;
        case tnfn:
        case tfn:
            fprintf(f,"##<function>");
            break;
        default:
            fprintf(stderr,"error: invalid data type\n");
            exit(1);
    }
    return;
}

/*read eval print loop*/

int main()
{
    char banner[]="                               __\n     _________________ _______|  | __\n    /  ___/\\____ \\__  \\\\_  __ \\  |/ /\n    \\___ \\ |  |_> > __ \\|  | \\/    <\n   /____  >|   __(____  /__|  |__|_ \\\n        \\/ |__|       \\/           \\/\n\n";

    //fprintf(stderr,banner);
    //fprintf(stderr,"*Spark*\n");
    init();

    s *sexp;

    while(1)
    {
        fprintf(stderr,"*> ");
        sexp=spark_read(stdin);
        print(stdout,spark_eval(sexp,global_scope));
        printf("\n");
    }
    return 0;
}

/*
;Test Cases

123
-123
007
007.007
-007
-007.007
1.234e1
1.234e+1
-1.234e-1
1.234E1
1.234E+1
-1.234E-1
-1.234E-1.234
;0b11010101 ;binary not yet
;0x3de34f ;hex not yet
;0x02337 ;octal not yet



;single line ;;comment
;:multi
line
comment:;

##t ;true
##T
##f ;false
##F
#a ;characters
#z
#A
#Z
#0
#9
#\n ;newline
#\s ;space
#\t ;tab
#\# ;# character
#\\ ;\ character

"single line string"

"multi
    line string \n\n\n "

"akgre\t\s\n\rhhRfT" ;\t\s\n\r as in C string

"\x02\xff\xae\x3a\x34" ;hex string as in C

;"\u2343\u3ea4\u24f0" ;no unicode strings yet

'() ;the ' operator
(quote ()) ; alternative quote fn

'(0 . 1)
'(0 1 2 3)

(def a -234.34e-3.4)
a
(set a -23)
a

(if ##t 1 2)
(if ##t 'a 'b)
(if ##f 1 2)
(if 0 1 2)
;(if ##f 1) ;segfaulting test case - undefined result
(if ##f 1 ()) ;works fine
;(if ##f 1 (quote  (1 2 3))) ;invalid expression

(+ 12 3 40)
(+ 23.4 45.34 -45.45e-3.4)
(+)
+

(bool? ##t)
(bool? -7.232)
(num->char 99)
(char->num #\#)
(< 1 2 3)
(> 3 2 1)
(cons 'a 'b)

;lambda the ultimate
(def fact (fn (n) (if (= n 1) 1 (* n (fact (- n 1))))))
(fact 5)

;the Y combinator
(def Y
    (fn (f)
      ((fn (x) (f (fn (y) ((x x) y))))
       (fn (x) (f (fn (y) ((x x) y)))))))

(def factorial
    (Y (fn (fact)
         (fn (n)
           (if (= n 0)
               1
               (* n (fact (- n 1))))))))

(fact 23)
(factorial 23)

(def outer
    (fn (a)
        (def inner
        (fn (b)
            (+ a b)))
        (inner 3)))
(outer 4)

(do 1 2 3)

(cond (##f 1)
        ((eq? 'a 'a) 2)
        (else 3))

(let ((x (+ 1 1)) (y (- 5 2))) (+ x y))

(def (map proc items)
    (if (null? items)
        '()
        (cons (proc (car items))
              (map proc (cdr items)))))


(and 1 2 3)
(and)
(or ##f 2 ##t)
(or)
(def a 1)
(and ##f (set a 2))
a
(and ##t (set a 3))
a
(or ##f (set a 4))
a
(or ##t (set a 5))
a

(apply + '(1 2 3))
(apply + 1 2 '(3))
(apply + 1 2 3 '())


(def env (new-env))
(eval '(def z 25) env)
(eval 'z env)

*/
/*

======scheme vs dylan========

#t    #t
#f    #f
23    23
#b1011   #b1011
#o644   #o644
#x2A5F   #x2A5F
-4/5   -4/5
6.02E23   6.02E23
#\a    'a'
#\newline  '\n'
                (Dylan supports \a, \b, \e, \f, \n, \r, \t, and \0)
"Hello"   "Hello"
N/A    "Hello\n"
'apple   #"apple" or apple:
N/A    #"two words" (Think "hashed string"...)
'(1 #\a dog) #(1, 'a', #"dog")
'#(5 10 15)  #[5, 10, 15]
`(1 2 ,x ,@y) N/A

*/


/*
simply scheme functions
+
-
/
<
<=
=
>
>=
abs
accumulate
acos
align
appearances
asin
atan
before?
bf
bl
butfirst
butlast
ceiling
char->word
char-rank
children
close-all-ports
close-input-port
close-output-port
cos
count
datum
empty?
equal?
even?
every
exp
expt
filter
first
floor
gcd
integer?
item
keep
last
lcm
list-ref
log
logoize
logoize-1
logoize-2
make-node
make-vector
max
maybe-num
member?
min
modulo
negative?
number->string
number?
odd?
open-input-file
open-output-file
positive?
quotient
random
read-line
read-string
reduce
remainder
remove
repeated
round
se
sentence
sentence?
show
show-line
sin
sqrt
string->word
strings-are-numbers
tan
truncate
vector-ref
vector-set!
whoops
word
word->string
word?
zero?
*/
