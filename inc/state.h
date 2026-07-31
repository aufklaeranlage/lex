#ifndef STATE_H
# define STATE_H

# include <unistd.h>
# include <stdbool.h>

typedef struct transition_s	transition_t;

typedef struct state_s {
	transition_t	**table;
	ssize_t			table_size;
	bool			accepting;
}	state_t;

bool	state_init(state_t *st);
state_t	*state_new();
void	state_clean(state_t *st);
bool	state_re(state_t *st);
void	state_del(state_t *st);

bool	state_resize(state_t *st, ssize_t size);
bool	state_add_transition(state_t *st, transition_t *t, ssize_t idx);

#endif
