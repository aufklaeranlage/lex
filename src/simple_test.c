#include "automata.h"
#include "state.h"
#include "transition.h"

static automata_t	*cleanup(automata_t *at, state_t *last_st, state_t *st, transition_t *t) {
	automata_del(at);
	state_del(last_st);
	state_del(st);
	transition_del(t);
	return (NULL);
}

#define STACK_SIZE 25

#define START_SUBAUTOMATA \
	lst = st;\
	lst_idx = st_idx;\
	if (state_resize(lst, at->alphabet_size) == false) {\
		return (cleanup(at, NULL, NULL, NULL));\
	}\
	st = state_new();\
	t = transition_new();\
	if (st == NULL || t == NULL) {\
		return (cleanup(at, NULL, st, t));\
	}\
	st_idx = automata_add_state(at, st);\
	if (st_idx == -1) {\
		return (cleanup(at, NULL, st, t));\
	}\
	t->state = st_idx;\
	if (state_add_transition(lst, t, epsi_idx) == false) {\
		return (cleanup(at, NULL, NULL, t));\
	}

#define EPSILON_TO_LST \
	if (state_resize(st, at->alphabet_size) == false) {\
		return (cleanup(at, NULL, NULL, NULL));\
	}\
	t = transition_new();\
	if (t == NULL) {\
		return (cleanup(at, NULL, NULL, t));\
	}\
	t->state = lst_idx;\
	if (state_add_transition(st, t, epsi_idx) == false){\
		return (cleanup(at, NULL, NULL, t));\
	}

automata_t	*at_from_str(input_t const *str) {
	automata_t		*at = NULL;
	state_t			*lst = NULL, *st = NULL;
	transition_t	*t = NULL;

	ssize_t			stidx_stack[STACK_SIZE];
	ssize_t			stidx_pos = 0;

	at = automata_new();
	if (at == NULL)
		return (cleanup(at, lst, st, t));
	st = state_new();
	if (st == NULL)
		return (cleanup(at, lst, st, t));
	if (automata_add_state(at, st) == -1)
		return (cleanup(at, lst, st, t));
	at->start_state = 0;

	ssize_t	epsi_idx = automata_add_input(at, epsilon);
	if (epsi_idx < 0)
		return (cleanup(at, NULL, NULL, NULL));

	struct {
		bool	set;
		bool	esc;
		bool	or;
	}	flags;
	flags.set = false;
	flags.esc = false;
	flags.or = false;

	ssize_t	lst_idx = 0, st_idx = 0;
	ssize_t	pos = 0;
	while (str[pos] != '\0') {
		if (flags.esc == false) {
			if (str[pos] == lsqbrack && flags.set == false) {
				lst = st;
				lst_idx = st_idx;
				flags.set = true;
				st = state_new();
				st_idx = automata_add_state(at, st);
				if (st_idx == -1)
					return (cleanup(at, NULL, st, NULL));
				++pos;
				continue ;
			} else if (str[pos] == asterisk) {
				/* Asterisk, reconnect last state to itself on idx of 'asterisk'
				 * input */
				EPSILON_TO_LST
				++pos;
				continue ;
			} else if (str[pos] == lbrack && stidx_pos < STACK_SIZE) {
				// Start new sub-automata with epsilon connection
				START_SUBAUTOMATA
				// Save lst_idx
				stidx_stack[stidx_pos++] = lst_idx;
				++pos;
				continue ;
			} else if (str[pos] == rbrack && stidx_pos != 0) {
				// Connect end of sub-automata through epsilon and reset lst_idx
				// Start new sub-automata with epsilon connection
				lst = st;
				lst_idx = st_idx;
				if (state_resize(lst, at->alphabet_size) == false)
					return (cleanup(at, NULL, NULL, NULL));
				st = state_new();
				t = transition_new();
				if (st == NULL || t == NULL)
					return (cleanup(at, NULL, st, t));
				st_idx = automata_add_state(at, st);
				if (st_idx == -1)
					return (cleanup(at, NULL, st, t)); // transition is alredy in lst 
				t->state = st_idx;
				if (state_add_transition(lst, t, epsi_idx) == false)
					return (cleanup(at, NULL, NULL, t));
				lst_idx = stidx_stack[--stidx_pos];
				lst = at->states[lst_idx];
				++pos;
				continue ;
			} else if (str[pos] == pipeor) {
				++pos;
				continue ;
			} else if (str[pos] == escape) {
				flags.esc = true;
				++pos;
				continue ;
			}
		}
		if (flags.set == false && flags.or == false) {
			// Start new sub-automata with epsilon connection
			START_SUBAUTOMATA
			// Add new literal node
			lst = st;
			lst_idx = st_idx;
			ssize_t	idx = automata_add_input(at, str[pos]);
			if (idx < 0)
				return (cleanup(at, NULL, NULL, NULL));
			if (state_resize(lst, at->alphabet_size) == false)
				return (cleanup(at, NULL, NULL, NULL));
			st = state_new();
			t = transition_new();
			if (st == NULL || t == NULL)
				return (cleanup(at, NULL, st, t));
			st_idx = automata_add_state(at, st);
			if (st_idx == -1)
				return (cleanup(at, NULL, st, t)); // transition is alredy in lst 
			t->state = st_idx;
			if (state_add_transition(lst, t, idx) == false)
				return (cleanup(at, NULL, NULL, t));
		} else if (flags.set == true || flags.or == true) {
			// Add to state without creating new one
			if (str[pos] == rsqbrack) {
				flags.set = false;
				++pos;
				continue ;
			}
			// Reset or flag always
			flags.or = false;
			ssize_t	idx = automata_add_input(at, str[pos]);
			if (idx < 0)
				return (cleanup(at, NULL, NULL, t));
			if (state_resize(lst, at->alphabet_size) == false)
				return (cleanup(at, NULL, NULL, t));
			t = transition_new();
			if (t == NULL)
				return (cleanup(at, NULL, NULL, t));
			t->state = st_idx;
			if (state_add_transition(lst, t, idx) == false)
				return (cleanup(at, NULL, NULL, t));
		}
		if (flags.esc == true)
			flags.esc = false;
		++pos;
	}
	st->accepting = true;
	for (ssize_t i = 0; i < at->nstates; i++) {
		if (at->states[i]->table_size < at->alphabet_size) {
			state_resize(at->states[i], at->alphabet_size);
		}
	}
	return (at);
}

ssize_t __ltostr_impl(ssize_t num, char *str, ssize_t len, ssize_t depth) {
	str[depth] = (num % 10) + 0x30;
	num = num / 10;
	if (num == 0 || --len == 0)
		return (depth);
	return (__ltostr_impl(num, str, len, depth + 1));
}

void ltostr(ssize_t num, char *str, char **endptr, ssize_t len) {
	ssize_t	end = __ltostr_impl(num, str, len, 0);
	*endptr = str + end;
}

#include <string.h>

const char	*transition_string(transition_t *t) {
	static char		str[20];
	char			*endptr;
	ssize_t			pos = 0;

	memset(str, '\0', 20);
	while (pos < 18 && t != NULL) {
		if (pos != 0 )
			str[pos++] = ',';
		ltostr(t->state, str + pos, &endptr, 20 - pos);
		pos = endptr - str + 1;
		t = t->next;
	}
	return (str);
}

#include <stdio.h>
#include <ctype.h>

void print_automata(automata_t *at) {
	ssize_t	cols = at->alphabet_size, rows = at->nstates;

	printf("---------");
	for (int i = 0; i < cols; i++) {
		printf("---------");
	}
	printf("-\n");
	printf("|%8s|", "states");
	for (int i = 0; i < cols; i++) {
		if (isprint(at->alphabet[i])) {
			printf("%8c|", at->alphabet[i]);
		} else if (at->alphabet[i] == epsilon) {
			printf("%8s|", "epsilon");
		} else {
			printf("%8d|", (unsigned int)(at->alphabet[i]));
		}
	}
	printf("\n");
	for (int i = 0; i < rows; i++) {
		if (i == at->start_state) {
			printf("|->%6d|", i);
		} else if (at->states[i]->accepting == true) {
			printf("|*%7d|", i);
		} else {
			printf("|%8d|", i);
		}
		if (at->states[i]->table == NULL) {
			for (int i2 = 0; i2 < cols; i2++) {
				printf("--------|");
			}
		} else {
			for (int j = 0; j < cols; j++) {
				if (at->states[i]->table[j] == NULL) {
					printf("%8s|", "xxxxxxxx");
				} else {
					printf("%8s|", transition_string(at->states[i]->table[j]));
				}
			}
		}
		printf("\n");
	}
	printf("---------");
	for (int i = 0; i < cols; i++) {
		printf("---------");
	}
	printf("-\n");
}

int main(int ac, char const *av[]) {
	if (ac <= 1)
		return (-1);
	automata_t	*at = at_from_str((input_t const *)av[1]);
	if (at == NULL)
		return (-1);
	print_automata(at);
	automata_del(at);
	return (0);
}
