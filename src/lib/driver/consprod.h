#ifndef __SUMERU_CONSPROD_H
#define __SUMERU_CONSPROD_H

struct consprod {
    char                        *buffer_start;
    char                        *buffer_end;
    char               		* volatile prod;
    char               		* volatile cons;
} __attribute__ ((packed));

typedef	struct consprod consprod_t;

void		consprod_init(
		    consprod_t *cp, 
		    char *buf_start, 
		    char *buf_end);

unsigned int	consprod_read(
		    consprod_t *cp, 
		    char *buf, 
		    unsigned int len);

unsigned int	consprod_write(
		    consprod_t *cp, 
		    const char *buf, 
		    unsigned int len);

int		consprod_peek(consprod_t *cp);


static inline
char*
consprod_get_prod(consprod_t *cp)
{
    return cp->prod;
}


static inline
char*
consprod_get_cons(consprod_t *cp)
{
    return cp->cons;
}


static inline
char*
consprod_get_buffer_end(consprod_t *cp)
{
    return cp->buffer_end;
}


static inline
char*
consprod_get_buffer_start(consprod_t *cp)
{
    return cp->buffer_start;
}


static inline
void
consprod_set_cons(consprod_t *cp, char *p)
{
    cp->cons = p;
}


static inline
void
consprod_set_prod(consprod_t *cp, char *p)
{
    cp->prod = p;
}


#endif

