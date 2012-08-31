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

#include "new.h"

using namespace std;


/*
    flags
    xxxxxxxx    xxxxxxxx    xxxxxxxx    xxxxxxxx
    gc          other       subtype     type
*/

enum z_type
{
    tnum=1,
    tstring, tsymbol, tregex,
    tlist, tvector, thashmap, tlink,
    tfn, tnfn,
    tmodule,
    tiport, toport,
    tmacro, tpromise,
    tblob,

    tsubtype = 1<<8,
    tfixnum = (tnum + tsubtype),
    tflonum = (tnum + tsubtype + tsubtype),
    tbignum = (tnum + tsubtype + tsubtype + tsubtype),

    gcmask = 0x00ffffff,
    gcused = 1<<31,
    gcmarked = 1<<30,
    gcflag = gcused | gcmarked,
    gcclear = 0xffffffff >> 2,
};

struct z
{
    int flags;
    union
    {
        int ch;
        size_t sym;
        double flonum;
        long int fixnum;
        string *s;
        struct
        {
            struct z *car;
            struct z *cdr;
        } list;
        vector<struct z*> *v;
        map<string, struct z*> *m;
        struct
        {
            struct map<struct z*, struct z*> *scope;
            struct z *body;
        } fn;
        struct z *(*nfn)(struct map<struct z*, struct z*> *scope);
        FILE *in;
        FILE *out;
        struct
        {
            void *p;
            size_t s;
        } blob;
        struct
        {   
            map<string, struct z*> *elements;
            map<string, struct z*> *symbols;
        } module;
    } val;
};

struct mem_pool
{
    vector<struct z*> *mem;
    vector<struct z*> *free_cells;
};

vector<string> error_stack;
vector<string> symbols;

inline void type_init(struct z *obj, int type)
{
    obj->flags=(0x00000000) | type;
}

inline int type_get(struct z *obj)
{
    return obj->flags&0x0000ffff;
}

inline void type_set(struct z *obj, int type)
{
    obj->flags=(obj->flags & 0xffff0000) | type;
}

inline int istype(struct z *obj, int type)
{
    return ((obj->flags&0x0000ffff)==type)?1:0;
}


struct mem_pool *make_mem_pool(size_t units)
{
    mem_pool *mp = new mem_pool();
    mp->mem = new vector<struct z*>();
    return mp;
}

void mem_pool_expand(mem_pool *mp, size_t units)
{
}

void mem_pool_break(struct mem_pool *mp)
{
    mp->mem->clear();
    delete mp->mem;
    delete mp;
}

struct z *obj_make(mem_pool *mp)
{
    struct z *o= new z();
    mp->mem->push_back(o);
    return o;
}

void obj_break(mem_pool *mp, struct z *o)
{
    
}


struct z *make_fixnum(mem_pool *mp, long int v)
{
    struct z *p = obj_make(mp);
    type_init(p,tfixnum);
    p->val.fixnum=v;
    return p;
}

struct z *make_flonum(mem_pool *mp, double v)
{
    struct z *p = obj_make(mp);
    type_init(p,tflonum);
    p->val.flonum=v;
    return p;
}


/*
struct z *make_string1(mem_pool *mp, void *v, size_t s);
struct z *make_string2(mem_pool *mp, char *s);
*/
struct z *make_string3(mem_pool *mp, string s)
{
    struct z *p = obj_make(mp);
    type_init(p,tstring);
    p->val.s= new string(s);
    return p;
}


/*
struct z *make_symbol1(mem_pool *mp, void *v, size_t s);
struct z *make_symbol2(mem_pool *mp, char *s);
*/

struct z *make_symbol3(mem_pool *mp, size_t s)
{
    struct z *p = obj_make(mp);
    type_init(p,tsymbol);
    p->val.sym=s;
    return p;
}

/*
struct z *make_regex1(mem_pool *mp, void *v, size_t s);
struct z *make_regex2(mem_pool *mp, char *s);
*/
struct z *make_list(mem_pool *mp, struct z *pcar, struct z *pcdr)
{
    struct z *p = obj_make(mp);
    type_init(p,tlist);
    p->val.list.car=pcar;
    p->val.list.cdr=pcdr;

    return p;
}

struct z *make_vector(mem_pool *mp, size_t s)
{
    struct z *p = obj_make(mp);
    type_init(p,tvector);
    p->val.v= new vector<struct z*>;

    return p;
}

struct z *make_hashmap(mem_pool *mp, size_t s)
{
    struct z *p = obj_make(mp);
    type_init(p,thashmap);
    p->val.m = new map<string,struct z*>;

    return p;
}

/*
struct z *make_iport(mem_pool *mp, FILE *in);
struct z *make_oport(mem_pool *mp, FILE *out);
struct z *make_fn(mem_pool *mp, struct z *scope, struct z *body);
struct z *make_nfn(mem_pool *mp, z *(*pnfn)(struct z *scope));
*/

inline struct z *list_car(struct z *l)
{
    if(type_get(l) == tlist)
    {
        return l->val.list.car;
    }
    else
    {
        return obj_null;
    }
}

inline struct z *list_cdr(struct z *l)
{
    if(type_get(l) == tlist)
    {
        return l->val.list.cdr;
    }
    else
    {
        return obj_null;
    }
}


/*
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
*/


int is_delim(int ch)
{
    return isspace(ch) || ch==0 || ch==',' ||
            ch=='(' || ch==')' ||
            ch=='[' || ch==']' ||
            ch=='{' || ch=='}' ||
            ch=='\'' || ch=='"' ||
            ch=='#';
}

int is_term_delim(int ch)
{
    return  ch==0 || 
            ch==')' ||
            ch==']' || 
            ch=='}';
}

int is_oper(int ch)
{
    return ch=='`'||ch=='~'||ch=='!'||ch=='@'||ch=='#'||ch=='$';
}

int is_spl(int ch)
{
    return ch=='%'||ch=='^'||ch=='&'||ch=='*'||ch=='_'||ch=='-'||ch=='+'||ch=='|'||ch=='\\'||
        ch=='<'||ch=='>'||ch=='.'||ch=='/'||ch=='?'||ch==';'||ch==':'||ch=='=';      
}

int is_hex(int ch)
{
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}

int zread_next_char(FILE *in)
{
    int ch=getc(in);
    ungetc(ch, in);
    return ch;
}

void zread_skipws(FILE *in)
{
    int ch;
    while((ch=getc(in))!=0)
    {
        /*skip whitespaces*/
        if(isspace(ch)){continue;}
        /*skip comma*/
        else if (ch==','){continue;}
        
        else if(ch=='#')
        {
            while((ch=getc(in)) !=0 && ch!='\n'){}
            continue;
        }
        
        ungetc(ch,in);
        break;
    }
}

struct z *zread_list(mem_pool *mp, FILE *in)
{
    int ch;
    struct z *pcar, *pcdr;

    pcar=obj_null;
    pcdr=obj_null;
    
    /*skip whitespaces and comments*/
    zread_skipws(in);
    ch=getc(in);

    /*got empty list*/
    if(ch==')')
    {
        return obj_null;
    }
    
    else if(ch==':'||is_term_delim(ch))
    {
        error_stack.push_back("error: invaild list pair");
        return obj_notok;
    }
    
    ungetc(ch,in);
    
    /*get the car*/
    pcar=zread(mp,in);    
    if(pcar == obj_notok) return obj_notok;
    zread_skipws(in);
    ch=getc(in);

    /*pairing*/
    if(ch==':')
    {
        zread_skipws(in);
        ch=zread_next_char(in);
        if(is_term_delim(ch))
        {
            error_stack.push_back("error: invaild list pair");
            return obj_notok;
        }
        
        /*get the cdr*/
        pcdr=zread(mp,in);
        if(pcdr == obj_notok) return obj_notok;
        zread_skipws(in);
        ch=getc(in);
        
        if(ch!=')')
        {
            error_stack.push_back("error: invaild list pair");
            return obj_notok;
        }
        
        return make_list(mp, pcar, pcdr);
    }
    else
    {
        ungetc(ch,in);
        pcdr=zread_list(mp,in);
        if(pcdr == obj_notok) return obj_notok;
        return make_list(mp, pcar, pcdr);
    }
}

struct z *zread_vector(mem_pool *mp, FILE *in)
{
    int ch;
    struct z *vec;
    struct z *pelem;

    /*skip whitespaces and comments*/
    zread_skipws(in);
    ch=getc(in);

    /*got empty vector*/
    if(ch==']')
    {
        return obj_null;
    }

    vec=make_vector(mp, 4);
    ungetc(ch,in);
    
    zread_skipws(in);
    ch=getc(in);
            
    while(ch != ']')
    {
        ungetc(ch,in);
        
        pelem=zread(mp,in);
        if(pelem == obj_notok) return obj_notok;
        vec->val.v->push_back(pelem);
        
        zread_skipws(in);
        ch=getc(in);
        
        if(ch==']') break;        
    }
    
    return vec;
}


struct z *zread_hashmap(mem_pool *mp, FILE *in)
{
    int ch,quote;
    struct z *map;
    struct z *value;
    string key="";

    /*skip whitespaces and comments*/
    zread_skipws(in);
    ch=getc(in);

    map=make_hashmap(mp, 4);
    
    /*got empty hashmap*/
    if(ch=='}')
    {
        return obj_null;
    }
    ungetc(ch,in);    
    
    zread_skipws(in);
    ch=getc(in);
    
    while(ch != '}')
    {
        ungetc(ch,in);
    
        zread_skipws(in);
        ch=getc(in);
        
        if(ch=='"' || ch=='\'')
        {
            quote=ch;

            while((ch=getc(in))!=quote)
            {
                if(ch==0)
                {
                    
                    error_stack.push_back("error: invaild hashmap key");
                    return obj_notok;
                }
                else
                {
                    key.append(1,(char)ch);
                }
            }
            
            zread_skipws(in);
            ch=getc(in);
    
             /*pairing*/
            if(ch==':')
            {
                zread_skipws(in);
                ch=zread_next_char(in);
                if(is_term_delim(ch))
                {
                    
                    error_stack.push_back("error: invaild hashmap");
                    return obj_notok;
                }
                
                value=zread(mp,in);
                if(value == obj_notok) return obj_notok;
                map->val.m->insert( std::pair<string,struct z*>(key,value) );
                  
                zread_skipws(in);
                ch=getc(in);
        
                if(ch=='}')
                {
                    break;
                }
                else
                {
                    key="";
                    continue;
                }
            }
        }
        else
        {
            
            error_stack.push_back("error: invaild hashmap");
            return obj_notok;
        }
    }   
    return map;
}

struct z *zread(mem_pool *mp, FILE *in)
{
    int ch;
    bool hex=false;
    bool id=false;
    string numbuff="";
    string str="";

    zread_skipws(in);
    ch=getc(in); 

    /*number*/
    if(isdigit(ch) || ((ch=='-' || ch=='+') && isdigit(zread_next_char(in))))
    {
        hex=false;
        if(ch=='-' || ch=='+')
        {
            numbuff.append(1,ch);
            ch=getc(in);
        }
        
        if(ch=='0') 
        {
            numbuff.append(1,ch);
            ch=getc(in); 
        }
        
        if(ch=='x') 
        {
            ch=getc(in); hex=true;
            numbuff.append(1,ch);
        }
        
        id=false;
        
        while(isdigit(ch)||(hex && is_hex(ch)))
        {
            numbuff.append(1,ch);
            ch=getc(in);
        }

        if(!hex && ch=='.')
        {
            id=true;
            numbuff.append(1,ch);
            ch=getc(in);
            while(isdigit(ch))
            {
                numbuff.append(1,ch);
                ch=getc(in);
            }
        }
        
        if(!hex && (ch=='e' || ch=='E'))
        {
            id=true;
            numbuff.append(1,ch);
            ch=getc(in);
            if(ch=='-' || ch=='+')
            {
                numbuff.append(1,ch);
                ch=getc(in);
            }
            while(isdigit(ch))
            {
                numbuff.append(1,ch);
                ch=getc(in);
            }
            if(ch=='.' && isdigit(zread_next_char(in)))
            {
                numbuff.append(1,ch);
                ch=getc(in);
                while(isdigit(ch))
                {
                    numbuff.append(1,ch);   
                    ch=getc(in);
                }
            }
        }

        if(ch!='.' || is_delim(ch) || is_oper(ch) || is_spl(ch))
        {
            ungetc(ch,in);
            if(id == false)
            {
                long int inum=0;
                unsigned int hnum=0;
                int num=0;
                if(hex)
                {
                    sscanf(numbuff.c_str(),"%x",&num);
                    inum=(long int) num;
                }
                else
                {
                    sscanf(numbuff.c_str(),"%ld",&inum);
                    inum=(long int) inum;
                }
                return make_fixnum(mp,inum);
            }
            else
            {
                double d;
                sscanf(numbuff.c_str(),"%lf",&d);
                return make_flonum(mp,d);
            }
        }
        
        else
        {
            
            error_stack.push_back("error: invaild number syntax");
            return obj_notok;
        }
    }
    
    /*symbol or identifier*/
    else if(isalpha(ch) || is_spl(ch))
    {
        str="";
        while(isalpha(ch) || is_spl(ch))
        {
            str.append(1,ch);
            ch=getc(in);
        }
        
        symbols.push_back(str);
        
        return make_symbol3(mp, symbols.size()-1);
    }
    
    /*string*/
    else if(ch=='"'||ch=='\'')
    {
        char quote=ch;
        while((ch=getc(in))!=quote)
        {
            if(ch=='\\')
            {
                ch=getc(in);
                switch(ch)
                {
                    case '\n':
                        continue;
                    case 'n':
                        str.append(1,'\n');
                        continue;
                    case 't':
                        str.append(1,'\t');
                        continue;
                    case 's':
                        str.append(1,' ');
                        continue;
                    case 'r':
                        str.append(1,'\r');
                        continue;
                    case '"':
                        str.append(1,ch);
                        continue;
                    case '\'':
                        str.append(1,ch);
                        continue;
                    case '\\':
                        str.append(1,'\\');
                        continue;
                    case '0':
                        //str.append(1,'\0');
                        continue;
                    case 'x':
                    {
                        int inum;
                        numbuff.append(1,'0');
                        numbuff.append(1,'x');
                        if(is_hex((ch=getc(in))))
                        {
                            numbuff.append(1,ch);
                        }
                        else
                        {   
                            
                            error_stack.push_back("error: invalid hex string");
                            return obj_notok;
                        }
                        if(is_hex((ch=getc(in))))
                        {
                            numbuff.append(1,ch);
                        }
                        else
                        {
                            
                            error_stack.push_back("error: invalid hex string");
                            return obj_notok;
                        }
                        ch=getc(in);
                        if(!ch)
                        {
                            
                            error_stack.push_back("error: invalid string");
                            return obj_notok;
                        }
                        ungetc(ch,in);
                        sscanf(numbuff.c_str(),"%x",&inum);
                        str.append(1,char(inum));
                        continue;
                    }
                    default:
                        
                        error_stack.push_back("error: invaild string termination");
                        return obj_notok;
                }
            }
            else if(ch==0)
            {
                
                error_stack.push_back("error: invaild string");
                return obj_notok;
            }
            else
            {
                str.append(1,(char)ch);
            }
        }
        return make_string3(mp,str);
    }
    
    else if(ch=='(')
    {
        return zread_list(mp,in);
    }
    
    else if(ch=='[')
    {
        return zread_vector(mp,in);
    }
    
    else if(ch=='{')
    {
        return zread_hashmap(mp,in);
    }
    
    else if(is_oper(ch))
    {
        return obj_null;
    }
    
    else if(ch==EOF)
    {
        return obj_eof;
    }
    
    else
    {
        return obj_notok;
    }
}



struct z *zeval(mem_pool *mp, struct z *exp, struct z *scope)
{
    return exp;
}


void zprint(FILE *out, struct z *exp)
{
    if(exp == obj_eof)
    {
        //fprintf(out,"()");
        return;
    }
    
    else if(exp == obj_null)
    {
        fprintf(out,"()");
        return;
    }
    else if(exp == obj_true)
    {
        fprintf(out,"true");
        return;
    }
    else if(exp == obj_false)
    {
        fprintf(out,"false");
        return;
    }
    else if(exp == obj_notok)
    {
        vector<string>::iterator i;
        for(i=error_stack.begin();i!=error_stack.end();++i)
        {
            fprintf(out,"%s\n",(*i).c_str());
        }
        error_stack.clear();
        fprintf(out,"not-ok\n");
        exit(1);
    }
    else 
    {
        int t=type_get(exp);
        
        if(t == tfixnum)
        { 
            fprintf(out,"%ld",exp->val.fixnum);
            return;
        }
        else if(t == tflonum)
        { 
            fprintf(out,"%lf",exp->val.flonum);
            return;
        }
        else if(t == tstring)
        { 
            fprintf(out,"\'%s\'",exp->val.s->c_str());
            return;
        }    
        else if(t == tlist)
        { 
            fprintf(out,"(");
            zprint_list(out,exp);
            fprintf(out,")");
            return;
        }
        else if(t == tvector)
        { 
            fprintf(out,"[");
            zprint_vector(out,exp);
            fprintf(out,"]");
            return;
        }
        else if(t == thashmap)
        { 
            fprintf(out,"{ ");
            zprint_hashmap(out,exp);
            fprintf(out," }");
            return;
        }
        else if(t == tsymbol)
        { 
            fprintf(out,"%s",symbols[exp->val.sym].c_str());
            return;
        }
        
        return;
    }
}

void zprint_list(FILE *out, struct z *exp)
{
    struct z *car = list_car(exp);
    struct z *cdr = list_cdr(exp);
    
    zprint(out,car);
    
    if(type_get(cdr) == tlist)
    {
        fprintf(out," ");
        zprint_list(out,cdr);
    }
    
    else if(cdr == obj_null)
        return;
    
    else
    {
        fprintf(out," : ");
        zprint(out,cdr);
    }
}

void zprint_vector(FILE *out, struct z *exp)
{
    struct z *elem;
    vector<struct z*> *vec;
    vector<struct z*>::iterator i;

    vec=exp->val.v;
    
    for(i = vec->begin(); i < vec->end(); ++i)
    {
        elem=*i;
        
        zprint(out,elem);
        
        if(i < vec->end()-1)
        {
            fprintf(out,", ");
        }
    }    
}

void zprint_hashmap(FILE *out, struct z *exp)
{
    string key;
    struct z *value;
    map<string,struct z*> *hm;
    map<string,struct z*>::iterator i;
    size_t j;

    hm=exp->val.m;
    
    for(i = hm->begin(),j=0; i != hm->end(); ++i,++j)
    {
        key=(*i).first;
        value=(*i).second;
        
        fprintf(out,"\"%s\"",key.c_str());
        fprintf(out," : ");
        zprint(out,value);
        
        if(j < hm->size()-1)
        {
            fprintf(out,", ");
        }
    }
}


void repl(mem_pool *mp, FILE *in, FILE *out, struct z *scope)
{
    struct z *sexp;
    while(1)
    {
        fprintf(out,"> ");
        sexp=zread(mp,in);
        zprint(out,zeval(mp,sexp,scope));
        fprintf(out,"\n");
    }
}

int main(int agrc, char *agrv[])
{
    struct z *scope;
    
    mem_pool *mp=make_mem_pool(1);
    
    obj_null = new z();
    obj_notok = new z();
    obj_eof = new z();
    
    repl(mp,stdin,stdout,scope);
    
    return 0;
}

