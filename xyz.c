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

#define XYZCVERSION 1.0
#define XYZCUPDATED june-13-2011

/*
*/

#include "xyz.h"

/*gc stuff*/

/*core function definitions*/

z *make_obj()
{
    z *p;
    p=malloc(sizeof(struct z));
    if(!p){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    p->val.i=0;
    p->val.s=0;
    p->val.pair.car=0;
    p->val.pair.cdr=0;
    p->val.nfn=0;
    p->val.fn.scope=0;
    p->val.fn.args=0;
    p->val.fn.body=0;
    return p;
}

z *make_bool(int b)
{
    z *p=make_obj();
    TYPESET(p,tbool);
    p->val.i=(long int)b;
    return p;
}

z *make_inum(long int i)
{
    z *p=make_obj();
    TYPESET(p,tinum);
    p->val.i=i;
    return p;
}

z *make_num(double d)
{
    z *p=make_obj();
    TYPESET(p,tnum);
    p->val.d=d;
    return p;
}

z *make_string(char *str)
{
    z *p=make_obj();
    TYPESET(p,tstring);
    p->val.s=malloc(strlen(str)+1);
    if(!p->val.s){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
    strcpy(p->val.s,str);
    return p;
}

z *cons(z *pcar, z *pcdr) //cons a.k.a make_pair
{
    z *p=make_obj();
    TYPESET(p,tpair);
    p->val.pair.car=pcar;
    p->val.pair.cdr=pcdr;
    return p;
}

void splitmodobj(char *name, char schar, char *modname, char *objname)
{
    int i = 0;
    int j = 0;
    int flag = 0;

    while (name[i] != '\0')
    {
        if (flag == 0)
        {
            if (name[i] == schar)
            {
                flag = 1;
                modname[i] = '\0';
            }
            else
            {
                modname[i] = name[i];
            }
        }
        else
        {
            objname[j++] = name[i];
            objname[j]='\0';
        }
    i++;
    }
}


z *findobj(z *objs, char *name)
{
    z *p, *sym;
    sym=objs;
	int idx=0;
    while(!is_null(sym))
    {
        if(strcmp(car(sym)->val.s,name)==0)
        {
            //obj found
			printf("%u %s at %d\n",(unsigned)car(sym),car(sym)->val.s,idx);
            return car(sym);
        }
        sym=cdr(sym);
		idx++;
    }
    
    //obj not found
    return obj_notok;
}

z *createobj(z *objs, char *name)
{
    z *obj;
    if(objs && name)
    {
        obj=make_obj();
        TYPESET(obj,tsymbol);
        obj->val.s=malloc(strlen(name)+1);
        if(!obj->val.s){fprintf(stderr,"problem: insufficient memory\n");exit(1);}
        strcpy(obj->val.s,name);
        objs=cons(objs,obj);
        return obj;
    }
    fprintf(stderr,"error: invalid objname / memory access\n");
    return obj_notok;
}

z *make_symbol(z *scope, char *name)
{

/*
if modname present in name
    if module present
        if obj present return obj
        else throw error
    else throw error
else
    if main module present
        if obj present return obj
        else create obj
    else throw error    
*/

    char modname[30],objname[30];
    z *obj, *tray, *currmod, *currobjs;

    if(strchr(name,':')!=NULL)
    {
        splitmodobj(name,':',modname,objname);
        
        if(modname)
        {
            //modname present
            tray=gtray;
            while(!is_null(tray))
            {
                currmod=car(tray);
                if(strcmp(car(currmod)->val.s,modname)==0)
                {
                    //module present
                    currobjs=cdr(currmod);
                    obj= findobj(currobjs,objname);
                    if(obj!=obj_notok)
                    {
                        //obj present
                        return obj;
                    }
                    else
                    {
                        fprintf(stderr,"error: undefined object\n");                        
                        return obj_notok;
                    }
                }
                tray=cdr(tray);
            }
            fprintf(stderr,"error: undefined module\n");
            return obj_notok;
        }
        else
        {
            fprintf(stderr,"error: undefined module or object\n");
            obj= obj_notok;
        }        
    }
    else
    {
        //no module name present
        tray=gtray;
        while(!is_null(tray))
        {
            currmod=car(tray);
            if(strcmp(car(currmod)->val.s,"main")==0)
            {
                //main module present
                currobjs=cdr(currmod);
                obj= findobj(currobjs,name);
                if(obj!=obj_notok)
                {
                    //obj present
                    return obj;
                }
                else
                {
                    //obj not present so create it.
                    obj= createobj(currobjs,name);
                    return obj;
                }
            }
            tray=cdr(tray);
        }        
        fprintf(stderr,"error: main module not found\n");
        return obj_notok;
    }
}




////////////



z *make_nfn(z *(*pnfn)(struct z *scope, struct z *args))
{
    z *p=make_obj();
    TYPESET(p,tnfn);
    p->val.nfn=pnfn;
    return p;
}

z *make_fn(z *scope, z *args, z *body)
{
    z *p=make_obj();
    TYPESET(p,tfn);
    p->val.fn.args=args;
    p->val.fn.body=body;
    p->val.fn.scope=scope;
    return  p;
}

z *make_iport(FILE *in)
{
    z *p=make_obj();
    TYPESET(p,tiport);
    p->val.in=in;
    return p;
}

z *make_oport(FILE *out)
{
    z *p=make_obj();
    TYPESET(p,toport);
    p->val.out=out;
    return p;
}

void init()
{
    obj_null=make_obj();
    TYPESET(obj_null,tnull);
    obj_true=make_obj();
    TYPESET(obj_true,tbool);
    obj_true->val.i=1;
    obj_false=make_obj();
    TYPESET(obj_false,tbool);
    obj_false->val.i=0;

    symbols=obj_null;
    empty_scope=obj_null;
}

void load_default_symbols(z *scope)
{
    obj_quote=make_symbol(scope,"quote");
    obj_def=make_symbol(scope,"def");
    obj_set=make_symbol(scope,"set");
    obj_ok=make_symbol(scope,"ok");
    obj_notok=make_symbol(scope,"not-ok");
    obj_if=make_symbol(scope,"if");
    obj_fn=make_symbol(scope,"fn");
    obj_do=make_symbol(scope,"do");
    obj_cond=make_symbol(scope,"cond");
    obj_else=make_symbol(scope,"else");
    obj_let=make_symbol(scope,"let");
    obj_and=make_symbol(scope,"and");
    obj_or=make_symbol(scope,"or");
    obj_quasiquote=make_symbol(scope,"quasiquote");
    obj_unquote=make_symbol(scope,"unquote");
    obj_unquotesplice=make_symbol(scope,"unquotesplice");
}


/*lib functions*/

/*scope functions*/

/*read*/

int zread_next_char(FILE *in)
{
    int ch=getc(in);
    ungetc(ch,in);
    return ch;
}

void zread_skipws(FILE *in)
{
    int ch;
    while((ch=getc(in))!=0)
    {
        /*skip whitespaces*/
        if(isspace(ch)){continue;}
        /*skip multiline comments*/
        else if(ch==';' && (zread_next_char(in)==':'))
        {
            ch=getc(in);
            while((ch=getc(in)) !=0 && (ch!=':' || zread_next_char(in)!=';')){}
            continue;
        }
        /*skip single line comments*/
        else if(ch==';' && (zread_next_char(in)!=':'))
        {
            while((ch=getc(in)) !=0 && ch!='\n'){}
            continue;
        }
        ungetc(ch,in);
        break;
    }
}

void skip_str(FILE *in,char *s)
{
    int ch;
    while(*s!=0)
    {
        ch=getc(in);
        if(ch!=*s)
        {
            fprintf(stderr,"error: unexpected string\n");
            exit(1);
        }
        s++;
    }
}

z *zread(z *scope, FILE *in)
{
    int ch;
    char numbuff[NUMBUFFSIZE];
    char strbuff[STRBUFFSIZE];
    int l=0;
    int sl=0;
    int hex=0;
    int id=0;

#define NBUFF(x) if(l < NUMBUFFSIZE - 1){numbuff[l] = x; l++; numbuff[l] = 0;}
#define SBUFF(x) if(sl < STRBUFFSIZE - 1){strbuff[sl] = x; sl++; strbuff[sl] = 0;} else {fprintf(stderr,"error: string/symbol too long max:%d\n", STRBUFFSIZE);exit(1);}

    /*skip whitespaces*/
    zread_skipws(in);
    ch=getc(in);

    if(ch=='#')
    {
        ch=getc(in);
        switch(ch)
        {
            /*boolean*/
            case '#':
            {
                ch=getc(in);
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
                ch=getc(in);

                switch(ch)
                {
                    case 'r':
                        return make_string("\r");
                    case 'n':
                        return make_string("\n");
                    case 't':
                        return make_string("\t");
                    case 's':
                        return make_string(" ");
                    case '\\':
                        return make_string("\\");
                    case '#':
                        return make_string("#");
                    default:
                        fprintf(stderr,"error: invalid special character\n");
                        //exit(1);
                        return obj_notok;
                }
            }

            /*characters = string[0]*/
            default:
            {
                if(is_delim(zread_next_char(in)))
                {
                    char s[]={'\0','\0'};
                    s[0]=ch;
                    return make_string(s);
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
    else if(isdigit(ch) || ((ch=='-' || ch=='+')&& isdigit(zread_next_char(in))))
    {
        hex=0;
        if(ch=='-' || ch=='+')
        {
            NBUFF(ch)
            ch=getc(in);
        }
        if(ch=='0') {ch=getc(in); NBUFF('0')}
        if(ch=='x') {ch=getc(in); hex=1; NBUFF('x')}
        id=0;
        while(isdigit(ch)||(hex && is_hex(ch)))
        {
            NBUFF(ch)
            ch=getc(in);
        }
        if(!hex && ch=='.')
        {
            id=1;
            NBUFF(ch)
            ch=getc(in);
            while(isdigit(ch))
            {
                NBUFF(ch)
                ch=getc(in);
            }
        }
        if(!hex && (ch=='e' || ch=='E'))
        {
            id=1;
            NBUFF(ch)
            ch=getc(in);
            if(ch=='-' || ch=='+')
            {
                NBUFF(ch)
                ch=getc(in);
            }
            while(isdigit(ch))
            {
                NBUFF(ch)
                ch=getc(in);
            }
            if(ch=='.' && isdigit(zread_next_char(in)))
            {
                NBUFF(ch)
                ch=getc(in);
                while(isdigit(ch))
                {
                    NBUFF(ch)
                    ch=getc(in);
                }
            }
        }
        if(is_delim(ch))
        {
            ungetc(ch,in);
            /*debug*/
            /*printf("%s\n",numbuff);*/
            switch(id)
            {
                case 0:
                {
                    long int li;
                    int i;
                    if(hex)
                    {
                        sscanf(numbuff,"%x",&i);
                        li=(long)i;
                    }
                    else sscanf(numbuff,"%ld",&li);
                    return make_inum(li);
                    break;
                }
                case 1:
                {
                    char *ep;
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
    else if(((ch=='+' || ch=='-') && !isdigit(zread_next_char(in))) || is_init(ch))
    {
        sl=0;
        while(is_init(ch) || isdigit(ch) || ch=='+' || ch=='-')
        {
            SBUFF(ch);
            //printf("%c\n",ch);
            ch=getc(in);
        }
        if(is_delim(ch))
        {
            ungetc(ch,in);
            //printf("%s\n",strbuff);
            return make_symbol(scope,strbuff);
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
        while((ch=getc(in))!='"')
        {
            if(ch=='\\')
            {
                ch=getc(in);
                switch(ch)
                {
                    case 'n':
                        SBUFF('\n')
                        continue;
                    case 't':
                        SBUFF('\t')
                        continue;
                    case 's':
                        SBUFF(' ')
                        continue;
                    case 'r':
                        SBUFF('\r')
                        continue;
                    case '"':
                        SBUFF(ch)
                        continue;
                    case '\\':
                        SBUFF('\\')
                        continue;
                    case '0':
                        //SBUFF(0)
                        continue;
                    //hex string
                    case 'x':
                    {
                        l=0;
                        long int li;
                        int i;
                        NBUFF('0')
                        NBUFF('x')
                        if(is_hex((ch=getc(in)))){NBUFF(ch)}
                        else
                        {
                            fprintf(stderr,"error: invalid hex string\n");
                            //exit(1);
                            return obj_notok;
                        }
                        if(is_hex((ch=getc(in)))){NBUFF(ch)}
                        else
                        {
                            fprintf(stderr,"error: invalid hex string\n");
                            //exit(1);
                            return obj_notok;
                        }
                        ch=getc(in);
                        if(!ch)
                        {
                            fprintf(stderr,"error: illegal string\n");
                            //exit(1);
                            return obj_notok;
                        }
                        ungetc(ch,in);
                        /*printf("%s",numbuff);*/
                        sscanf(numbuff,"%x",&i);
                        /*printf("%d %c",inum,inum);*/
                        SBUFF((int)i)
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
                SBUFF((char)ch)
            }
        }
        return make_string(strbuff);
    }

    else if(ch=='(')
    {
        return zread_pair(scope,in);
    }

    else if(ch=='\'')
    {
        return cons(obj_quote,cons(zread(scope,in),obj_null));
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

z *zread_pair(z *scope,FILE *in)
{
    int ch;
    z *pcar, *pcdr;

    zread_skipws(in);
    ch=getc(in);

    /*empty list*/
    if(ch==')')
    {
        return obj_null;
    }
    ungetc(ch,in);

    pcar=zread(scope,in);
    zread_skipws(in);
    ch=getc(in);

    /*pair*/
    if(ch=='.')
    {
        ch=zread_next_char(in);
        if(!is_delim(ch))
        {
            fprintf(stderr,"error: incorrect pair syntax\n");
            //exit(1);
            return obj_notok;
        }
        pcdr=zread(scope,in);
        zread_skipws(in);
        ch=getc(in);
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
        ungetc(ch,in);
        pcdr=zread_pair(scope,in);
        return cons(pcar,pcdr);
    }
}

/*eval*/

z *var_bind(z *scope, z *pvar, z *pval)
{
    z *scope1, *pvars, *pvals;
    scope1=car(scope);
    pvars=get_vars(scope1);
    pvals=get_vals(scope1);
    while(!is_null(pvars))
    {
        if(pvar==car(pvars))
        {
            z *p=car(pvals);p=pval;
            return obj_ok;
        }
        pvars=cdr(pvars);
        pvals=cdr(pvals);
    }
    pvars=car(scope1);pvars=cons(pvar,car(scope1));
    pvals=cdr(scope1);pvals=cons(pval,cdr(scope1));
    return obj_ok;
}

z *var_val(z *scope, z *pvar)
{
    z *scope1, *pvars, *pvals;
    while(!is_null(scope))
    {
        scope1=car(scope);
        pvars=get_vars(scope1);
        pvals=get_vals(scope1);
        while(!is_null(pvars))
        {
            if(pvar==car(pvars))
            {
                return car(pvals);
            }
            pvars = cdr(pvars);
            pvals = cdr(pvals);
        }
        scope=cdr(scope);
    }
    fprintf(stderr,"error: undefined variable\n");
    //exit(1);
    return obj_notok;
}

z *parse_def_var(z *o)
{
    if(is_symbol(cadr(o))) return cadr(o);
    else return caadr(o);
}

z *parse_def_val(z *o)
{
    if(is_symbol(cadr(o))) return caddr(o);
    //else return parse_fn_body(cdadr(o),cddr(o));
}

z *parse_set_var(z *o)
{
    if(is_symbol(cadr(o))) return cadr(o);
    else
    {
        fprintf(stderr,"error: (set <var> <val>) expected\n");
        return obj_notok;
    }
}

z *parse_set_val(z *o)
{
    if(is_symbol(cadr(o))) return caddr(o);
    else
    {
        fprintf(stderr,"error: (set <var> <val>) expected\n");
        return obj_notok;
    }
}

z *zeval(z *scope, z *exp)
{
    tailcalll:
    if(is_self_evalable(exp)){return exp;}
    else if(is_symbol(exp)){return var_val(scope,exp);}
    else if(is_tagged(exp,obj_quote)){return cadr(exp);}
    else if(is_tagged(exp,obj_def)){return var_bind(scope,parse_def_var(exp),parse_def_val(exp));}
    else
    {
        fprintf(stderr,"error: cannot eval, invalid symbolic expression\n");
        //exit(1);
        return obj_notok;
    }
    fprintf(stderr,"error: cannot eval, invalid state\n");
    return exp;
}

/*print*/

void zprint_pair(FILE *out,z *exp)
{
    z *pcar, *pcdr;
    pcar=car(exp);
    pcdr=cdr(exp);
    zprint(out,pcar);
    if(TYPEGET(pcdr,tpair))
    {
        fprintf(out," ");
        zprint_pair(out,pcdr);
    }
    else if(TYPEGET(pcdr,tnull))
    {
        return;
    }
    else
    {
        fprintf(out," . ");
        zprint(out,pcdr);
    }
}

void zprint(FILE *out,z *exp)
{
    if(TYPEGET(exp,tnull))
    {
        fprintf(out,"()");
        return;
    }
    else if(TYPEGET(exp,tbool))
    {
        fprintf(out,"##%c",((int)exp->val.i)?'t':'f');
        return;
    }
    else if(TYPEGET(exp,tstring))
    {
        if(strlen(exp->val.s)==1)
        {
            switch(exp->val.s[0])
            {
                case '\n':
                    fprintf(out,"%s","#\\n");
                    break;
                case '\t':
                    fprintf(out,"%s","#\\t");
                    break;
                case ' ':
                    fprintf(out,"%s","#\\s");
                    break;
                case '\\':
                    fprintf(out,"%s","#\\\\");
                    break;
                case '#':
                    fprintf(out,"%s","#\\#");
                    break;
                case 'r':
                    fprintf(out,"%s","#\\r");
                    break;
                default:
                    fprintf(out,"#%c",exp->val.s[0]);
            }
            return;
        }
        else
        {
            fprintf(out,"\"%s\"",exp->val.s);
            return;
        }
    }
    else if(TYPEGET(exp,tinum))
    {
        fprintf(out,"%li",exp->val.i);
        return;
    }
    else if(TYPEGET(exp,tnum))
    {
        fprintf(out,"%f",exp->val.d);
        return;
    }
    else if(TYPEGET(exp,tsymbol))
    {
        fprintf(out,"%s",exp->val.s);
        return;
    }
    else if(TYPEGET(exp,tpair))
    {
        fprintf(out,"(");
        zprint_pair(out,exp);
        fprintf(out,")");
        return;
    }
    else if(TYPEGET(exp,tfn) || TYPEGET(exp,tnfn))
    {
        fprintf(out,"##<function>:%u",((unsigned int)&exp));
    }
    else
    {
        fprintf(stderr,"error: invalid data type\n");
        exit(1);
    }
    return;
}

/*read eval print loop*/

int main()
{
    //char b[]="___  ___ ___ __ ________\n\\  \\/  /<   |  |\\___   /\n >    <  \\___  | /    / \n/__/\\_ \\ / ____|/_____ \\\n      \\/ \\/           \\/ \n";
    //printf("%s",b);
    //printf("%s","*xyz* - a small scheme like implementation\n\n");

    init();
    gtray=cons(
            cons(obj_null,
                    cons(obj_null,obj_null)),obj_null);
    z *s=cons(cons(obj_null,obj_null),empty_scope);
	load_default_symbols(s);
    while(1)
    {
        printf("xyz> ");
        zprint(stdout,zeval(s,zread(s,stdin)));
        printf("\n");
    }

    return 0;
}

/*========================================================*/

/*dynamic stack*/

int dstack_push(struct dstack *st, void *p)
{
    if(st)
    {
        st->list = malloc(sizeof(struct stacknode));
        if(st->list == 0)
        {
            fprintf(stderr,"dstack error: no memory\n");
            exit(1);
        }
        st->list->p = p;
        st->list->link = st->head;
        st->head= st->list;
        st->size++;

        return 0;
    }
    else
    {
        return 1;
    }
}

void *dstack_pop(struct dstack *st)
{
    if(st)
    {
        if(st->head == 0)
        {
            /*dstack is empty*/
            return 0;
        }
        void *tmp = st->head->p;
        st->list = st->head;
        st->head = st->head->link;
        free(st->list);
        st->size--;
        return tmp;
    }
    else
    {
        return 0;
    }

}

void dstack_pop2(struct dstack *st)
{
    void *p=dstack_pop(st);
    p=0;
}

void dstack_init(struct dstack *st)
{
    if(st)
    {
        st->head = 0;
        st->list = 0;
        st->size=0;
    }
    else
    {}
}

/*dynamic stack*/

/*dynamic hashtable*/

hashnode *htable_create()
{
    hashnode *p=malloc((sizeof(struct hashnode))*HASHSIZE);
    if(!p){fprintf(stderr,"hashmap error: no memory\n");exit(1);}
    return p;
}

void htable_init(hashnode *ht)
{
    int i;
    for(i==0;i<HASHSIZE;i++)
    {
        ht[i].value=0;
        ht[i].next=0;
        ht[i].key=0;
    }
}

int htable_hash(char *str)
{
    int index = 0;
    char *tmp = 0;
    tmp = calloc(strlen(str) + 1, sizeof(char));
    strcpy(tmp, str);
    while(*tmp)
    {
        index += *tmp;
        tmp++;
    }
    index = index % HASHSIZE;
    return index;
}

int htable_hash1(char *str)
{
    int index = 0;
    for(;*str;str++) index= MULTIPLIER * index + *str;
    return index % HASHSIZE;
}

int htable_hash2(char *str,int multi, int items)
{
    unsigned int h_=0;
    for(;*str;str++) h_= multi * h_ + *str;
    return h_ % items;
}

void htable_put(hashnode *ht, char *str, void *v)
{
    int index = 0;

    index = htable_hash1(str);
    if(ht[index].value != 0)
    {
        htable_resolve(ht, index, str, v);
    }
    else
    {
        ht[index].key = calloc(strlen(str) + 1, sizeof(char));
        strcpy(ht[index].key, str);
        ht[index].value=v;
    }
}

void htable_resolve(hashnode *ht, int loc, char *str, void *v)
{
    hashnode *tmp;
    tmp = ht + loc;

    while(tmp->next != 0)
        tmp = tmp->next;
    tmp->next = (hashnode *)malloc(sizeof(hashnode));
    tmp->next->key = calloc(strlen(str) + 1, sizeof(char));
    strcpy(tmp->next->key, str);
    tmp->next->value=v;
    tmp->next->next = 0;
}

void htable_dump(hashnode *ht, FILE *out)
{
    int i = 0;
    hashnode *target;

    for(i = 0; i < HASHSIZE; i++)
    {
        if(ht[i].value != 0)
        {
            target = ht + i;
            while(target)
            fprintf(out,"index[%d] key[%s] value[%u]",i,target->key,(unsigned)target->value);
            target = target->next;
        }
    }
}

int htable_remove(hashnode *ht, char *str)
{
    hashnode *h1;
    hashnode *h2;
    char *tmp = 0;
    int index = 0;

    index = htable_hash1(str);

    /*no element*/
    if(ht[index].value == 0)
        return 0;

    /*single element*/
    if(ht[index].next == 0)
    {
        if(strcmp(ht[index].key, str) == 0)
        {
            /*value found*/
            tmp = ht[index].key;
            free(ht[index].key);
            ht[index].key = 0;
            ht[index].value = 0;
            free(tmp);
        }
    }
    else
    {
        /*chain of elements*/
        h1 = ht + index;
        /*traverse the chain*/
        while(h1->next != 0)
        {
            if(strcmp(h1->next->key, str) == 0)
            {
                h2 = h1->next;
                if(h1->next->next)
                    h1->next = h1->next->next;
                else
                    h1->next = 0;

                h2->key = 0;
                h2->value = 0;
                free(h2);
            }
        }
    }

 return 0;
}

/*dynamic hashtable*/
