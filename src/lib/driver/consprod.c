#include "consprod.h"

void
consprod_init(consprod_t *cp, char *buf_start, char *buf_end)
{
    cp->buffer_start = buf_start;
    cp->buffer_end = buf_end;
    cp->prod = buf_start + 1;
    cp->cons = buf_start;
}


int
consprod_peek(consprod_t *cp)
{
    char *next_cons = (char*)cp->cons + 1;
    if (next_cons == cp->buffer_end)
	next_cons = cp->buffer_start;
    if (next_cons == cp->prod)
	return -1;
    else
	return *((unsigned char*)next_cons);
}


unsigned int
consprod_read(consprod_t *cp, char *buf, unsigned int len)
{
    unsigned int x;
    char *next_cons;

    next_cons = (char*)cp->cons + 1;
    if (next_cons == cp->buffer_end)
	next_cons = cp->buffer_start;

    x = len;
    while (x > 0) {
	if (next_cons == cp->prod) {
	    if (x != len)
		return len - x;
	    else
		while (next_cons == cp->prod)
		    ;
	}

	*buf = *next_cons;
	buf = buf + 1;
	cp->cons = next_cons;
	next_cons = next_cons + 1;
	
    	if (next_cons == cp->buffer_end)
	    next_cons = cp->buffer_start;

	x = x - 1;
    }

    return len;
}


unsigned int
consprod_write(consprod_t *cp, const char *buf, unsigned int len)
{
    unsigned int x;
    char *next_prod;
    char *prev_prod;

    next_prod = (char *)cp->prod + 1;
    if (next_prod == cp->buffer_end)
	next_prod = cp->buffer_start;

    if (cp->prod == cp->buffer_start)
    	prev_prod = (char *)cp->buffer_end - 1;
    else
	prev_prod = (char *)cp->prod - 1;

    x = len;
    while (x > 0) {
	while (next_prod == cp->cons)
    	    ;	

	*prev_prod = *buf;
	buf = buf + 1;
	prev_prod = (char *)cp->prod;
	cp->prod = next_prod;
	next_prod = next_prod + 1;
	
    	if (next_prod == cp->buffer_end)
	    next_prod = cp->buffer_start;

	x = x - 1;
    }

    return len;
}
