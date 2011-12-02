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

/*    init();
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
*/


    mem_pool *p=make_mem_pool(cell_pool,65536,4);
    init_cell_pool(p,sizeof(struct z));
    break_mem_pool(p);

    return 0;
}
