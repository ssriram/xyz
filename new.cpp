#include <cstdio>
#include <cstdlib>

#include <string>
#include <list>
#include <vector>
#include <map>

#include "new.h"

using namespace std;

enum z_type
{
    tchar=1, 
    tfixnum, tflonum,
    tstring, tsymbol, tregex,
    tlist, tvector, thashmap,
    tiport, toport,
    tfn, tnfn,
    tblob
};

struct z
{
    int flags;
    int type;
    union
    {
        int ch;
        double flonum;
        long int fixnum;
        string *s;
        struct
        {
            struct z *car;
            struct z *cdr;
        } list;
        vector<struct z*> *v;
        map<string,struct z*> *m;
        struct
        {
            struct z *scope;
            struct z *body;
        } fn;
        struct z *(*nfn)(struct z *scope);
        FILE *in;
        FILE *out;
        struct
        {
            void *p;
            size_t s;
        } blob;
    } val;
};

struct mem_pool
{
    vector<struct z*>* p;
};

enum gc_flags
{
    gc_unused = 0,
    gc_used = 1,
    gc_marked = 2
};


struct mem_pool *global_mem_pool;




inline int type_get(struct z *obj)
{
    return obj->type;
}

inline void type_set(struct z *obj, int t)
{
    obj->type=t;
}

inline int istype(struct z *obj, int type)
{
    return (obj->type)==type;
}

struct mem_pool *make_mem_pool(size_t units)
{
    mem_pool *mp = new mem_pool();
    mp->p = new vector<struct z*>();
    return mp;
}

void mem_pool_expand(mem_pool *mp, size_t units)
{
    
}

void mem_pool_break(struct mem_pool *mp)
{
    mp->p->clear();
    delete mp->p;
    delete mp;
}

struct z *obj_make(mem_pool *mp)
{
    struct z *o= new z();
    mp->p->push_back(o);
    return o;
}

void obj_break(mem_pool *mp, struct z *o)
{
    
}


struct z *make_char(mem_pool *mp, int v)
{
    struct z *p = obj_make(mp);
    p->type=tchar;
    p->flags=gc_used;
    p->val.ch=v;    
    return p;
}

struct z *make_fixnum(mem_pool *mp, long int v)
{
    struct z *p = obj_make(mp);
    p->type=tfixnum;
    p->flags=gc_used;
    p->val.fixnum=v;    
    return p;
}

struct z *make_flonum(mem_pool *mp, double v)
{
    struct z *p = obj_make(mp);
    p->type=tflonum;
    p->flags=gc_used;
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
    p->type=tstring;
    p->flags=gc_used;
    p->val.s= new string(s);    
    return p;
}

/*
struct z *make_symbol1(mem_pool *mp, void *v, size_t s);
struct z *make_symbol2(mem_pool *mp, char *s);
struct z *make_regex1(mem_pool *mp, void *v, size_t s);
struct z *make_regex2(mem_pool *mp, char *s);
*/
struct z *make_list(mem_pool *mp, struct z *pcar, struct z *pcdr)
{
    struct z *p = obj_make(mp);
    p->type=tlist;
    p->flags=gc_used;

    p->val.list.car=pcar;
    p->val.list.cdr=pcdr;

    return p;
}

struct z *make_vector(mem_pool *mp, size_t s)
{
    struct z *p = obj_make(mp);
    p->type=tvector;
    p->flags=gc_used;
    
    p->val.v= new vector<struct z*>;
    
    return p;
}

struct z *make_hashmap(mem_pool *mp, size_t s)
{
    struct z *p = obj_make(mp);
    p->type=thashmap;
    p->flags=gc_used;
    
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

int read_next_char(FILE *in)
{
    int ch=getc(in);
    ungetc(ch, in);
    return ch;
}

void read_skipws(FILE *in)
{
    int ch;
    while((ch=getc(in))!=0)
    {
        /*skip whitespaces*/
        if(isspace(ch)){continue;}
        /*skip comma*/
        else if (ch==','){continue;}
        /*skip multiline comments*/
        else if(ch=='#' && (read_next_char(in)=='#'))
        {
            ch=getc(in);
            while((ch=getc(in)) !=0 && (ch!='#' && read_next_char(in)!='#')){}
            continue;
        }
        /*skip single line comments*/
        else if(ch=='#' && (read_next_char(in)!='#'))
        {
            while((ch=getc(in)) !=0 && ch!='\n'){}
            continue;
        }
        
        ungetc(ch,in);
        break;
    }
}

struct z *read_list(FILE *in)
{
    int ch;
    struct z *pcar, *pcdr;

    /*skip whitespaces and comments*/
    read_skipws(in);
    ch=getc(in);

    /*got empty list*/
    if(ch==')')
    {
        return obj_null;
    }
    ungetc(ch,in);
    
    /*get the car*/
    pcar=read(in); 
    
    read_skipws(in);
    ch=getc(in);

    /*pairing*/
    if(ch==':')
    {
        read_skipws(in);
        ch=read_next_char(in);
        if(is_term_delim(ch))
        {
            fprintf(stderr,"error: incorrect pair syntax\n");
            return obj_notok;
        }
        
        /*get the cdr*/
        pcdr=read(in);
        
        read_skipws(in);
        ch=getc(in);
        
        if(ch!=')')
        {
            fprintf(stderr,"error: incorrect pair syntax\n");
            return obj_notok;
        }
        
        return make_list(global_mem_pool, pcar, pcdr);
    }
    else
    {
        ungetc(ch,in);
        pcdr=read_list(in);
        return make_list(global_mem_pool, pcar, pcdr);
    }
}

struct z *read_vector(FILE *in)
{
    int ch;
    struct z *vec;
    struct z *pelem;

    /*skip whitespaces and comments*/
    read_skipws(in);
    ch=getc(in);

    vec=make_vector(global_mem_pool, 4);

    /*got empty vector*/
    if(ch==']')
    {
        return vec;
    }

    ungetc(ch,in);
    
    read_skipws(in);
    ch=getc(in);
            
    while(ch != ']')
    {
        ungetc(ch,in);
        
        pelem=read(in);
        vec->val.v->push_back(pelem);
        
        read_skipws(in);
        ch=getc(in);
        
        if(ch==']') break;        
    }
    
    return vec;
}


struct z *read_hashmap(FILE *in)
{
    int ch,quote;
    struct z *map;
    struct z *value;
    string key="";

    read_skipws(in);
    ch=getc(in);

    map=make_hashmap(global_mem_pool, 4);
    
    if(ch=='}')
    {
        return map;
    }
    ungetc(ch,in);    
    
    read_skipws(in);
    ch=getc(in);
    
    while(ch != '}')
    {
        ungetc(ch,in);
    
        read_skipws(in);
        ch=getc(in);
        
        if(ch=='"' || ch=='\'')
        {
            quote=ch;

            while((ch=getc(in))!=quote)
            {
	            if(ch==0)
	            {
		            fprintf(stderr,"error: improper hashmap key\n");
		            return obj_notok;
	            }
	            else
	            {
		            key.append(1,(char)ch);
	            }
            }
            
            read_skipws(in);
            ch=getc(in);
    
             /*pairing*/
            if(ch==':')
            {
                read_skipws(in);
                ch=read_next_char(in);
                if(is_term_delim(ch))
                {
                    fprintf(stderr,"error: incorrect hashmap syntax\n");
                    return obj_notok;
                }
                
                /*get the cdr*/
                value=read(in);
                
                map->val.m->insert( std::pair<string,struct z*>(key,value) );
                  
                read_skipws(in);
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
            else
            {
                fprintf(stderr,"error: incorrect hashmap syntax\n");
                return obj_notok;
            }
        }
        else
        {
            fprintf(stderr,"error: incorrect hashmap syntax\n");
            return obj_notok;
        }
    }   
    return map;
}

struct z *read(FILE *in)
{
    int ch;
    bool hex=false;
    bool id=false;
    string numbuff="";
    string str="";

    read_skipws(in);
    ch=getc(in); 

    /*number*/
    if(isdigit(ch) || ((ch=='-' || ch=='+') && isdigit(read_next_char(in))))
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
            if(ch=='.' && isdigit(read_next_char(in)))
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

        if(is_delim(ch))
        {
            ungetc(ch,in);
            switch(id)
            {
                case false:
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
                    return make_fixnum(global_mem_pool,inum);
                    break;
                }
                case true:
                {
                    double d;
                    sscanf(numbuff.c_str(),"%lf",&d);
                    return make_flonum(global_mem_pool,d);
                    break;
                }
            }
        }
        
        else
        {
            fprintf(stderr,"error: invalid number/delimiter\n");
            return obj_notok;
        }
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
						    fprintf(stderr,"error: invalid hex string\n");
						    return obj_notok;
					    }
					    if(is_hex((ch=getc(in))))
					    {
						    numbuff.append(1,ch);
					    }
					    else
					    {
						    fprintf(stderr,"error: invalid hex string\n");
						    return obj_notok;
					    }
					    ch=getc(in);
					    if(!ch)
					    {
						    fprintf(stderr,"error: illegal string\n");
						    return obj_notok;
					    }
					    ungetc(ch,in);
					    sscanf(numbuff.c_str(),"%x",&inum);
					    str.append(1,char(inum));
					    continue;
				    }
				    default:
					    fprintf(stderr,"error: improper string termination\n");
					    return obj_notok;
			    }
		    }
		    else if(ch==0)
		    {
			    fprintf(stderr,"error: improper string termination\n");
			    return obj_notok;
		    }
		    else
		    {
			    str.append(1,(char)ch);
		    }
	    }
	    return make_string3(global_mem_pool,str);
    }
    
    else if(ch=='(')
    {
        return read_list(in);
    }
    
    else if(ch=='[')
    {
        return read_vector(in);
    }
    
    else if(ch=='{')
    {
        return read_hashmap(in);
    }
}



struct z *eval(struct z *exp, struct z *scope)
{
    return exp;
}


void print(FILE *out, struct z *exp)
{
    if(exp==obj_null)
    {
        fprintf(out,"()");
    }
    else if(exp==obj_true)
    {
        fprintf(out,"true");
    }
    else if(exp==obj_false)
    {
        fprintf(out,"false");
    }
    else
    {
        int t=type_get(exp);
        switch(t)
        {
            case tchar:
                fprintf(out,"char");
                break;
            case tfixnum:
                fprintf(out,"%ld",exp->val.fixnum);
                break;
            case tflonum:
                fprintf(out,"%lf",exp->val.flonum);
                break;
            case tstring:
                fprintf(out,"\'%s\'",exp->val.s->c_str());
                break;
            case tlist:
                fprintf(out,"(");
                print_list(out,exp);
                fprintf(out,")");
                break;
            case tvector:
                fprintf(out,"[");
                print_vector(out,exp);
                fprintf(out,"]");
                break;
            case thashmap:
                fprintf(out,"{ ");
                print_hashmap(out,exp);
                fprintf(out," }");
                break;                
            default:
                fprintf(stderr,"error: invalid data type\n");
                break;
        }
    }
}

void print_list(FILE *out, struct z *exp)
{
    struct z *car = list_car(exp);
    struct z *cdr = list_cdr(exp);
    
    print(out,car);
    
    if(type_get(cdr) == tlist)
    {
        fprintf(out," ");
        print_list(out,cdr);
    }
    
    else if(cdr == obj_null)
        return;
    
    else
    {
        fprintf(out," : ");
        print(out,cdr);
    }
}

void print_vector(FILE *out, struct z *exp)
{
    struct z *elem;
    vector<struct z*> *vec;
    vector<struct z*>::iterator i;

    vec=exp->val.v;
    
    for(i = vec->begin(); i < vec->end(); ++i)
    {
        elem=*i;
        
        print(out,elem);
        
        if(i < vec->end()-1)
        {
            fprintf(out,", ");
        }
    }    
}

void print_hashmap(FILE *out, struct z *exp)
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
        print(out,value);
        
        if(j < hm->size()-1)
        {
            fprintf(out,", ");
        }
    }
}


void repl(FILE *in, FILE *out, struct z *scope)
{
    struct z *sexp;
    while(1)
    {
        fprintf(out,"> ");
        sexp=read(in);
        print(out,eval(sexp,scope));
        printf("\n");
    }
}

int main(int agrc, char *agrv[])
{
    struct z *scope;
    
    global_mem_pool=make_mem_pool(1);
    
    obj_null = new z();
    obj_notok = new z();
    
    repl(stdin,stdout,scope);
    
    return 0;
}

