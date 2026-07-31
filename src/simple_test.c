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

automata_t	*at_from_str(input_t const *str) {
	automata_t		*at = NULL;
	state_t			*lst = NULL, *st = NULL;
	transition_t	*t = NULL;
	at = automata_new();
	if (at == NULL)
		return (cleanup(at, lst, st, t));
	st = state_new();
	if (st == NULL)
		return (cleanup(at, lst, st, t));
	if (automata_add_state(at, st) == -1)
		return (cleanup(at, lst, st, t));
	at->start_state = 0;

	struct {
		bool	set;
		bool	esc;
	}	flags;
	flags.set = false;
	flags.esc = false;

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
				ssize_t	idx = automata_add_input(at, empty);
				if (idx < 0)
					return (cleanup(at, NULL, NULL, NULL));
				if (state_resize(lst, at->alphabet_size) == false)
					return (cleanup(at, NULL, NULL, NULL));
				t = transition_new();
				if (t == NULL)
					return (cleanup(at, NULL, NULL, t));
				t->state = lst_idx;
				if (state_add_transition(lst, t, idx) == false)
					return (cleanup(at, NULL, NULL, t));
				++pos;
				continue ;
			} else if (str[pos] == escape) {
				flags.esc = true;
				++pos;
				continue ;
			}
		}
		if (flags.set == false) {
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
		} else if (flags.set == true) {
			if (str[pos] == rsqbrack) {
				flags.set = false;
				++pos;
				continue ;
			}
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
		} else if (at->alphabet[i] == empty) {
			printf("%8s|", "empty");
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
					printf("%8ld|", at->states[i]->table[j]->state);
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
