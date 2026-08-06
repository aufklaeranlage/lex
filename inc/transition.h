#ifndef TRANSITION_H
# define TRANSITION_H

# include "state.h"

#include <unistd.h>
# include <stdbool.h>

typedef struct transition_s {
	ssize_t				state;
	struct transition_s	*next;
	ssize_t				min;
	ssize_t				max;
}	transition_t;

bool			transition_init(transition_t *t);
transition_t	*transition_new();
void			transition_del(transition_t *t);

void			transition_del_list(transition_t *head);

#endif
