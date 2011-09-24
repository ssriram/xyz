/*

this is the crap.c file which i use to test crap and once i
feel it doesn't smell like real crap will add it into the core

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xyz.h"

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


mem_pool *make_mem_pool(mem_pool_type type, size_t size, size_t units)
{
    mem_pool *mp=NULL,*mp1=NULL,*mp2=NULL;
    void *vp;
    if(units && size)
    {
        mp=malloc(sizeof(mem_pool));
        vp=calloc(1,size);
        mp->type=type;
        mp->size=size;
        mp->p=vp;
        mp->prev=NULL;
        mp->next=NULL;
        mp1=mp;
        //printf("unit: %d\n",units);
        for(units--;units>0;units--)
        {
            mp2=malloc(sizeof(mem_pool));
            vp=calloc(1,size);
            mp2->type=type;
            mp2->size=size;
            mp2->p=vp;
            mp2->prev=mp1;
            mp2->next=NULL;
            mp1->next=mp2;
            mp1=mp2;  
            //printf("unit: %d\n",units);
        }
    }
    return mp;
}

int break_mem_pool(mem_pool *mp)
{
    mem_pool *mp1=NULL,*mp2=NULL;
    if(mp)
    {
        mp1=mp;
        while(mp1->next)
        {
            mp1=mp1->next;
        };
        while(mp1->prev)
        {
            mp2=mp1->prev;
            free(mp1->p);
            //printf("freed memory\n");
            free(mp1);
            //printf("freed unit\n");
            mp1=mp2;
        };
        free(mp1->p);
        //printf("freed memory\n");
        free(mp1);
        //printf("freed unit\n");
        return 0;
    }
    return -1;
}




int main()
{
    mem_pool *p=make_mem_pool(cell_pool,65536,5);
    break_mem_pool(p);
    return 0;
}


