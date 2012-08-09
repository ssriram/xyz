
#include <stdio.h>
#include <stdlib.h>

typedef enum types
{
    eof=1, error, number, string, symbol, list, vector, map
} types;


int nextch(FILE *in)
{
    int ch=getc(in);
    ungetc(ch,in);
    return ch;
}

void skipws(FILE *in)
{
    int ch;
    while(in)
    {
        ch=getc(in);
        if(ch==' '||ch=='\t'||ch=='\n'||ch=='\r')
        {   
        }
        else
        {
            ungetc(ch,in);
            break;
        }
    }
}

int ishex(int ch)
{
    return ((ch>='0') && (ch<='9')) ||
           ((ch>='a') && (ch<='f')) ||
           ((ch>='A') && (ch<='F'));
}

int isoper(int ch)
{
    return ch=='`'||ch=='~'||ch=='!'||ch=='@'||ch=='#'||ch=='$';
}

int isspl(int ch)
{
    return ch=='%'||ch=='^'||ch=='&'||ch=='*'||ch=='_'||ch=='-'||ch=='+'||ch=='|'||ch=='\\'||
        ch=='<'||ch=='>'||ch=='.'||ch=='/'||ch=='?'||ch==';'||ch==':'||ch=='=';      
}

int isdelim(int ch)
{
    return isspace(ch)||ch==0||
    ch=='{'||ch=='['||ch=='('||
    ch=='}'||ch==']'||ch==')'||
    ch=='\''||ch=='"'||isoper(ch);
}


int parse(FILE *in)
{
    int ch;

    skipws(in);
    ch=getc(in);
    
    if(ch=='{')
    {
        int ch;
        int key,value;
                    
        while(in)
        {
            skipws(in);
            ch=getc(in);
            
            if(ch=='}')
            {
                break;
            }
            
            skipws(in);
            
            key = parse(in);
            
            skipws(in);
            ch=getc(in);
            
            if(ch==':')
            {
                value = parse(in);
            }

            printf("map key:%d value:%d\n",key,value);
            
            skipws(in);
            ch=getc(in);
            
            if(ch==',')
            {
                skipws(in);
            }
            
            else ungetc(ch,in);

        }

        return map;
    }
    
    else if(ch=='[')
    {
        int ch;
        int elem;
        
        while(in)
        {
            skipws(in);
            ch=getc(in);
            
            if(ch==']')
            {
                break;
            }
            
            skipws(in);
            
            elem = parse(in);
            
            printf("vector elem:%d\n",elem);
            
            skipws(in);
            ch=getc(in);
            
            if(ch==',')
            {
                skipws(in);
            }
            
            else ungetc(ch,in);
    
        }

        return vector;
    }
    
    else if(ch=='(')
    {
        int ch;
        int elem;
        
        while(in)
        {
            skipws(in);
            ch=getc(in);
            
            if(ch==')')
            {
                break;
            }
            
            skipws(in);
            
            elem = parse(in);
            
            printf("list elem:%d\n",elem);
            
            skipws(in);
            ch=getc(in);
            
            if(ch==',')
            {
                skipws(in);
            }
            
            else ungetc(ch,in);
    
        }

        return list;
    }
    
    else if(isdigit(ch) || ((ch=='+'||ch=='-') && isdigit(nextch(in))))
    {
        int ch;
        int hex=0;
        int flo=0;
        char *numstr;
        double dval;
        int ival;
        unsigned int hval;
        
        *numstr='0';numstr[1]='\0';
        
        skipws(in);
        ch=getc(in);
        
        if(ch=='+'||ch=='-')
        {
            *numstr=ch;numstr++;
            ch=getc(in);
        }    

        if(ch=='0' && 'x'==nextch(in))
        {
            hex=1;
            *numstr=ch;numstr++;
            *numstr=getc(in);numstr++;
            ch=getc(in);
        }
        
        while(isdigit(ch) || (hex && ishex(ch)))
        {
            *numstr=ch;numstr++;
            ch=getc(in);
        }
        
        if(!hex && ch=='.')
        {
            flo=1;
            *numstr=ch;numstr++;
            ch=getc(in);
            
            while(isdigit(ch))
            {
                *numstr=ch;numstr++;
                ch=getc(in);
            }
        }
        
        if(!hex && (ch=='e' || ch=='E'))
        {
            flo=1;
            *numstr=ch;numstr++;
            ch=getc(in);
            
            if(ch=='-' || ch=='+')
            {
                *numstr=ch;numstr++;
                ch=getc(in);
            }
            
            while(isdigit(ch))
            {
                *numstr=ch;numstr++;
                ch=getc(in);
            }
            
            if(ch=='.' && isdigit(nextch(in)))
            {
                *numstr=ch;numstr++;
                ch=getc(in);
                
                while(isdigit(ch))
                {
                    *numstr=ch;numstr++;
                    ch=getc(in);
                }    
            }
        }
        
        if(isdelim(ch))
        {
            ungetc(ch,in);
            *numstr='\0';
            
            if(flo)
            {
                sscanf(numstr,"%lf",&dval);
                printf("number:%lf\n",dval);
            }
            else if(hex)
            {
                sscanf(numstr,"%x",&hval);
                printf("number:%d\n",hval);            
            }
            else
            {
                sscanf(numstr,"%d",&ival);
                printf("number:%d\n",ival);
            }
            return number;
        }
        
        fprintf(stderr,"number syntax error");    
        return error;

    }
     
    else if(((ch=='+'||ch=='-') && !isdigit(nextch(in))) || ((isalpha(ch) || isspl(ch)) && !isoper(ch)))
    {
        ungetc(ch,in);
        {
            int ch;
            char *symstr;
            
            skipws(in);
            ch=getc(in);
            
            if ((ch=='+'||ch=='-') && !isdigit(nextch(in)))
            {
                *symstr=ch;symstr++;
                ch=getc(in);
            }

            while(in)
            {
                if((isalpha(ch) || isspl(ch) || isdigit(ch)) && !isoper(ch))
                {
                    *symstr=ch;symstr++;
                    ch=getc(in);
                }
                else
                {
                    symstr='\0';
                    printf("symbol:%s",symstr);
                    return symbol;
                }
            }
            
            fprintf(stderr,"symbol syntax error");    
            return error;
        }

    }
    
    else if(ch=='"' || ch=='\'')
    {
        ungetc(ch,in);
        {
            int ch;
            char *str;
            int quote;
            
            skipws(in);
            quote=getc(in);
            
            while(in)
            {
                ch=getc(in);
                
                switch(ch)
                {
                    case '\\':
                    {
                        ch=getc(in);
                        
                        switch(ch)
                        {
                            case '"':
                            case '\'':
                            case '\\':
                            case '/':
                            {
                                *str=ch;str++;
                            }
                            case 'b':
                            case 'f':
                            case 'n':
                            {
                                *str='\n';str++;
                            }
                            case 'r':
                            {
                                *str='\r';str++;
                            }
                            case 't':
                            {
                                *str='\t';str++;
                            }
                            case 'u':
                            {
                            }
                        }
                    }
                    case '"':
                    case '\'':
                    {
                        if(ch==quote)
                        {
                            *str='\0';
                            printf("string: %s",str);
                            return string;
                        }
                        else
                        {
                            *str=ch;str++;
                        }
                    }
                    default:
                    {
                        *str=ch;str++;
                    }
                }
            }
            
            fprintf(stderr,"syntax error");
            return error;
        }

    }
    
    else if(ch==EOF)
    {
        return eof;
    }
            
    else
    {
        fprintf(stderr,"syntax error\n");
        return error;
    }
}

//int parse_map(FILE *in)
//int parse_vector(FILE *in)
//int parse_list(FILE *in)
//int parse_string(FILE *in)
//int parse_number(FILE *in)
//int parse_symbol(FILE *in)

int eval(int t)
{
    return t;
}

void print(int t)
{
    switch(t)
    {
        case number:
            printf("got number\n");
            break;
        case string:
            printf("got string\n");
            break;
        case symbol:
            printf("got symbol\n");
            break;
        case list:
            printf("got list\n");
            break;
        case vector:
            printf("got vector\n");
            break;
        case map:
            printf("got map\n");
            break;
        case error:
            printf("got error\n");
            break;
        default:
            printf("got invalid\n");
            break;
    }
}

int main()
{
    int t;

    while(1)
    {
        printf("> ");
        t=parse(stdin);
        print(eval(t));
        printf("\n");
    }
    return 0;
}
