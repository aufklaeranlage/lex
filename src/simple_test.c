#include "automata.h"
#include "state.h"
#include "transition.h"

#include <stdlib.h>

static automata_t	*cleanup(automata_t *at, state_t *st, transition_t *t) {
	automata_del(at);
	state_del(st);
	transition_del(t);
	return (NULL);
}

#define STACK_SIZE 25

ssize_t __ltostr_impl(ssize_t num, char *str, ssize_t len, ssize_t depth) {
	char c = (num % 10) + 0x30;
	if (num == 0 || --len == 0)
		return (depth);
	ssize_t	ret = __ltostr_impl(num / 10, str, len, depth + 1);
	str[ret - 1 - depth] = c;
	return (ret);
}

void ltostr(ssize_t num, char *str, char **endptr, ssize_t len) {
	if (num == 0 && len > 0) {
		str[0] = '0';
		*endptr = str;
	} else {
		ssize_t	end = __ltostr_impl(num, str, len, 0);
		*endptr = str + end;
	}
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
		pos = endptr - str;
		if (t->min != 0 || t->max != 0) {
			str[pos++] = '{';
			if (t->min != 0) {
				ltostr(t->min, str + pos, &endptr, 20 - pos);
				pos = endptr - str;
			}
			if (t->min != t->max) {
				str[pos++] = ',';
			}
			if (t->max != t->min && t->max != 0) {
				ltostr(t->max, str + pos, &endptr, 20 - pos);
				pos = endptr - str;
			}
			str[pos++] = '}';
		}
		t = t->next;
	}
	return (str);
}

#include <stdio.h>
#include <ctype.h>

void mermaid_automata(automata_t *at, char const str[]) {
	ssize_t	states = at->nstates;

	printf("%s\n", str);
	for (ssize_t i = 0; i < states; i++) {
		state_t	*state = at->states[i];

		if (at->start_state == i) {
			printf("\t[*] --> %ld : epsilon\n", i);
		}
		if (state->accepting == true) {
			printf("\t%ld --> [*] : epsilon\n", i);
		}

		for (ssize_t j = 0; j < state->table_size; j++) {
			transition_t *t = state->table[j];
			while (t != NULL) {
				printf("\t%ld --> %ld", i, t->state);
				if (at->alphabet[j] != epsilon) {
					printf(" : %c\n", at->alphabet[j]);
				} else {
					printf(" : epsilon\n");
				}
				t = t->next;
			}
		}
	}
}

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
		if (i == at->start_state && at->states[i] != NULL && at->states[i]->accepting == true) {
			printf("|->*%5d|", i);
		} else if (i == at->start_state) {
			printf("|->%6d|", i);
		} else if (at->states[i] != NULL && at->states[i]->accepting == true) {
			printf("|*%7d|", i);
		} else {
			printf("|%8d|", i);
		}
		if (at->states[i] == NULL || at->states[i]->table == NULL) {
			for (int i2 = 0; i2 < cols; i2++) {
				printf("--------|");
			}
		} else {
			for (int j = 0; j < cols; j++) {
				if (at->states[i]->table_size <= j || at->states[i]->table[j] == NULL) {
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

ssize_t	automata_get_stateidx(automata_t *at, state_t *st) {
	for (ssize_t i = 0; i < at->nstates; i++) {
		if (at->states[i] == st)
			return (i);
	}
	return (-1);
}

#define NEW_STATE(state) \
	(state) = state_new();\
	if ((state) == NULL) {\
		return (cleanup(at, NULL, NULL));\
	}\
	if (automata_add_state(at, (state)) == -1) {\
		return (cleanup(at, (state), NULL));\
	}\

#define CONNECT(from, to, input_idx) \
	if (state_resize((from), at->alphabet_size) == false) {\
		return (cleanup(at, NULL, NULL));\
	}\
	t = transition_new();\
	if (t == NULL) {\
		return (cleanup(at, NULL, NULL));\
	}\
	t->state = automata_get_stateidx(at, (to));\
	if (state_add_transition((from), t, (input_idx)) == false){\
		cleanup(NULL, NULL, t);\
	}\

#define CUR_SCOPE scope[scope_pos]

#define INSERT_PRE_START \
	state_t	*tmp;\
	NEW_STATE(tmp);\
	ssize_t	start_idx = automata_get_stateidx(at, start);\
	ssize_t	tmp_idx = automata_get_stateidx(at, tmp);\
	at->states[start_idx] = tmp;\
	at->states[tmp_idx] = start;\
	CONNECT(tmp, start, epsi_idx);\
	if (CUR_SCOPE.start == start) {\
		CUR_SCOPE.start = tmp;\
	}\


#define MOVE_SCOPE \
	start = end;\
	NEW_STATE(end);\

automata_t *enfa_from_str(input_t const *str) {
	automata_t	*at = automata_new();
	if (at == NULL) { return (NULL); }
	state_t		*start = NULL, *end = NULL;
	transition_t	*t = NULL;

	struct {
		bool	or;
		bool	set;
	}	flags;
	flags.or = false;
	flags.set = false;

	at->start_state = 0;
	ssize_t		epsi_idx = automata_add_input(at, epsilon);
	if (epsi_idx == -1) { return (cleanup(at, NULL, NULL)); }

	struct {
		state_t	*start;
		state_t	*end;

		state_t	*threads[STACK_SIZE];
		ssize_t	thread_pos;
	}	scope[STACK_SIZE];
	ssize_t	scope_pos = 0;
	CUR_SCOPE.thread_pos = 0;

	NEW_STATE(start);
	end = start;
	CUR_SCOPE.start = start;
	CUR_SCOPE.end = start;

	ssize_t	pos = 0;
	ssize_t	alpha_idx;
	while (str[pos] != '\0') {
		if (str[pos] == asterisk) {
			// Insert artifical start state before start
			++scope_pos;
			NEW_STATE(CUR_SCOPE.start);
			NEW_STATE(CUR_SCOPE.end);
			ssize_t	start_idx = automata_get_stateidx(at, start);\
			ssize_t	tmp_idx = automata_get_stateidx(at, CUR_SCOPE.start);\
			at->states[start_idx] = CUR_SCOPE.start;\
			at->states[tmp_idx] = start;\
			// Before start modifications
			CONNECT(CUR_SCOPE.start, start, epsi_idx);
			CONNECT(end, start, epsi_idx);
			CONNECT(end, CUR_SCOPE.end, epsi_idx);
			CONNECT(CUR_SCOPE.start, CUR_SCOPE.end, epsi_idx);
			end = CUR_SCOPE.end;
			start = CUR_SCOPE.start;
			--scope_pos;
			CUR_SCOPE.start = start;
			CUR_SCOPE.end = end;

			++pos;
			continue ;
		} else if (str[pos] == lbrack) {
			CUR_SCOPE.start = end;
			NEW_STATE(start);
			CONNECT(end, start, epsi_idx);
			NEW_STATE(CUR_SCOPE.end);
			++scope_pos;
			end = start;
			CUR_SCOPE.start = start;
			CUR_SCOPE.end = end;
			// Reset threadpos for new scope
			CUR_SCOPE.thread_pos = 0;

			++pos;
			continue ;
		} else if (str[pos] == rbrack) {
			--scope_pos;
			CONNECT(end, CUR_SCOPE.end, epsi_idx);
			start = CUR_SCOPE.start;
			end = CUR_SCOPE.end;

			++pos;
			continue ;
		} else if (str[pos] == pipeor) {
			if (start == CUR_SCOPE.start) {
				// Insert artifical start state before start
				INSERT_PRE_START;
			}
			CUR_SCOPE.threads[CUR_SCOPE.thread_pos++] = end;
			NEW_STATE(start);
			CONNECT(CUR_SCOPE.start, start, epsi_idx);
			end = start;
			CUR_SCOPE.end = end;
			flags.or = true;

			++pos;
			continue ;
		} else if (str[pos] == lsqbrack && flags.set != true) {
			// Move scope once for new input set
			flags.set = true;
			MOVE_SCOPE;

			++pos;
			continue ;
		} else if (str[pos] == rsqbrack && flags.set == true) {
			flags.set = false;

			++pos;
			continue ;
		}

		// Connect loose threads if we didn't just add one
		if (flags.or == false && CUR_SCOPE.thread_pos != 0) {
			MOVE_SCOPE;
			CONNECT(start, end, epsi_idx);
			for (ssize_t i = 0; i < CUR_SCOPE.thread_pos; i++) {
				CONNECT(CUR_SCOPE.threads[i], end, epsi_idx);
			}
			CUR_SCOPE.thread_pos = 0;
		}
		// Reset or flag so we connect threads on the next operation
		flags.or = false;

		if (flags.set == false) {
			// Literal input
			alpha_idx = automata_add_input(at, str[pos]);
			if (alpha_idx == -1) { return (cleanup(at, NULL, NULL)); }
			// No set == move scope
			MOVE_SCOPE;
			CONNECT(start, end, alpha_idx);
		} else if (flags.set == true) {
			if (str[pos] == setconnect) {
				char	start_c = str[pos - 1];
				char	end_c = str[pos + 1];
				char	dir = start_c > end_c ? -1 : 1;
				for (; start_c != end_c; start_c += dir) {
					// Literal input
					alpha_idx = automata_add_input(at, start_c);
					if (alpha_idx == -1) { return (cleanup(at, NULL, NULL)); }
					CONNECT(start, end, alpha_idx);
				}
				// Literal input
				alpha_idx = automata_add_input(at, end_c);
				if (alpha_idx == -1) { return (cleanup(at, NULL, NULL)); }
				CONNECT(start, end, alpha_idx);
			} else {
				// Literal input
				alpha_idx = automata_add_input(at, str[pos]);
				if (alpha_idx == -1) { return (cleanup(at, NULL, NULL)); }
				// Set == don't move scope
				CONNECT(start, end, alpha_idx);
			}
		}
		++pos;
	}

	// Connect loose threads
	if (CUR_SCOPE.thread_pos != 0) {
		MOVE_SCOPE;
		CONNECT(start, end, epsi_idx);
		for (ssize_t i = 0; i < CUR_SCOPE.thread_pos; i++) {
			CONNECT(CUR_SCOPE.threads[i], end, epsi_idx);
		}
		CUR_SCOPE.thread_pos = 0;
	}
	
	// Needs to be before table size adjusting
	if (start->table_size == 0) { CONNECT(start, end, epsi_idx); }
	// Adjust all tables to appropriate size
	for (ssize_t i = 0; i < at->nstates; i++) {
		if (state_resize(at->states[i], at->alphabet_size) == false) {
			return (cleanup(at, NULL, NULL));
		}
	}

	end->accepting = true;
	return (at);
}

void print_cmap(cmap_t *cm) {
	for (ssize_t i = 0; i < cm->size; i++) {
		cnode_t *cn = cm->nodes[i];
		printf("%ld: ", cn->idx);
		for (ssize_t j = 0; j < cn->size; j++) {
			if (j != 0) {
				printf(", ");
			}
			printf("%ld", cn->mapped[j]);
		}
		printf("\n");
	}
}

bool automata_has_ambiguity(automata_t const *at) {
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_t	*state = at->states[i];
		for (ssize_t j = 0; j < state->table_size; j++) {
			transition_t	*t = state->table[j];
			ssize_t			num_t = 0;
			// Count number of transition_s
			for (; t != NULL; t = t->next) {
				++num_t;
			}
			if (num_t > 1) { return (true); }
		}
	}
	return (false);
}

automata_t *combine_states(automata_t const *nfa) {
	automata_t	*at = automata_new();
	if (at == NULL) { return (NULL); }
	cmap_t		*cm = cmap_new();
	if (cm == NULL) { return (cleanup(at, NULL, NULL)); }

	for (ssize_t i = 0; i < nfa->alphabet_size; i++) {
		if (automata_add_input(at, nfa->alphabet[i]) == -1) {
			return (cmap_del(cm), cleanup(at, NULL, NULL));
	  	}
	}

	// Create mapping for start node 
	cnode_t			*cn = cnode_new(at->nstates, 1);
	cn->mapped[0] = nfa->start_state;
	cmap_add_node(cm, cn);
	state_t	*tmp;
	NEW_STATE(tmp);
	if (state_resize(tmp, at->alphabet_size) == false) {
		return (cleanup(at, NULL, NULL));
	}
	at->start_state = 0;

	// Go through enfa and collect all possible transitions
	for (ssize_t i = 0; i < nfa->nstates; i++) {
		state_t	*state = nfa->states[i];
		// Create combined state from possible targets
		for (ssize_t j = 0; j < state->table_size; j++) {
			transition_t	*t = state->table[j];
			ssize_t			num_t = 0;
			// Move active state
			for (; t != NULL; t = t->next) {
				++num_t;
			}
			if (num_t == 0) { continue ; }

			cn = cnode_new(at->nstates, num_t);
			if (cn == NULL) { return (cmap_del(cm), cleanup(at, NULL, NULL)); }

			// Collect all state transitions
			num_t = 0;
			for (t = state->table[j]; t != NULL; t = t->next) {
				cn->mapped[num_t] = t->state;
				// Make new combined state also accpeting if target is
				if (nfa->states[t->state]->accepting == true) { cn->accepting = true; }
				++num_t;
			}

			ssize_t	cn_idx = cmap_contains(cm, cn);
			if (cn_idx == -1) {
				// Add new combination and create new node in at
				NEW_STATE(tmp);
				if (state_resize(tmp, at->alphabet_size) == false) { return (cleanup(at, NULL, NULL)); }
				if (cn->accepting == true) { tmp->accepting = true; }
				if (cmap_add_node(cm, cn) == false) { return (cnode_del(cn), cmap_del(cm), cleanup(at, NULL, NULL)); }
			} else {
				cnode_del(cn);
			}
		}
	}

	for (ssize_t i = 0; i < cm->size; i++) {
		state_t	*state = at->states[i];
		cnode_t	*node = cm->nodes[i];

		for (ssize_t j = 0; j < node->size; j++) {
			state_t	*cpy_st = nfa->states[node->mapped[j]];

			for (ssize_t k = 0; k < nfa->alphabet_size; k++) {
				transition_t	*t = cpy_st->table[k];
				ssize_t			num_t = 0;
				// Move active state
				for (; t != NULL; t = t->next) {
					++num_t;
				}
				if (num_t == 0) { continue ; }

				cn = cnode_new(at->nstates, num_t);
				if (cn == NULL) { return (cmap_del(cm), cleanup(at, NULL, NULL)); }

				// Collect all state transitions
				num_t = 0;
				for (t = cpy_st->table[k]; t != NULL; t = t->next) {
					cn->mapped[num_t] = t->state;
					++num_t;
				}

				ssize_t	cn_idx = cmap_contains(cm, cn);
				t = transition_new();
				if (t == NULL) {
					return (cleanup(at, NULL, NULL));
				}
				t->state = cn_idx;
				if (state_add_transition(state, t, k) == false){
					cleanup(NULL, NULL, t);
				}
				cnode_del(cn);
			}
		}
	}

	cmap_del(cm);

	return (at);
}

/**	@brief Replaces all transitions to `src` with transitions to `dst` and
 * 	removes `src` from `at`.
 */ 
void automata_replace(automata_t *at, ssize_t dst, ssize_t src) {
	state_t	*st_dst = at->states[src];
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_t	*st = at->states[i];
		if (st == NULL) { continue ; }
		for (ssize_t j = 0; j < st->table_size; j++) {
			transition_t	*t = st->table[j];
			for (; t != NULL; t = t->next) {
				if (t->state == src) {
					if (dst == -1) {
						transition_del_list(t);
						st->table[j] = NULL;
					} else {
						t->state = dst;
					}
					if (st_dst->accepting == true)
						st->accepting = true;
				}
			}
		}
	}

	state_del(at->states[src]);
	at->states[src] = NULL;
}

/**	@brief Replaces all transitions to `src` with transitions to `dst` and
 * 	removes `src` from `at`.
 */ 
void automata_mov(automata_t *at, ssize_t dst, ssize_t src) {
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_t	*st = at->states[i];
		if (st == NULL) { continue ; }
		for (ssize_t j = 0; j < st->table_size; j++) {
			transition_t	*t = st->table[j];
			for (; t != NULL; t = t->next) {
				if (t->state == src) {
					t->state = dst;
				}
			}
		}
	}
	if (at->start_state == src) { at->start_state = dst; }
	state_t	*tmp = at->states[dst];
	at->states[dst] = at->states[src];
	at->states[src] = tmp;
}

ssize_t	automata_get_first_null_stateidx(automata_t *at) {
	for (ssize_t i = 0; i < at->nstates; i++) {
		if (at->states[i] == NULL)
			return (i);
	}
	return (-1);
}

bool state_is_pure_epsilon(state_t *st, ssize_t epsi_idx) {
	for (ssize_t i = 0; i < st->table_size; i++) {
		if (i == epsi_idx) {
			if (st->table[i] == NULL ) {
				return (false);
			}
		} else if (st->table[i] != NULL) {
			return (false);
		}
	}
	return (true);
}

bool state_has_epsilon(state_t *st, ssize_t epsi_idx) {
	return (st->table[epsi_idx] != NULL);
}

bool automata_has_epsilon(automata_t *at) {
	ssize_t	epsi_idx = automata_get_input_idx(at, epsilon);

	for (ssize_t i = 0; i < at->nstates; i++) {
		if (state_has_epsilon(at->states[i], epsi_idx) == true)
			return (true);
	}
	return (false);
}

automata_t	*remove_null_states(automata_t *at) {
	ssize_t	null_idx = automata_get_first_null_stateidx(at);
	if (null_idx == -1)
		return (at);
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_t	*st = at->states[i];
		if (st == NULL) { continue ; }
		if (i > null_idx) {
			automata_mov(at, null_idx, i);
			null_idx = automata_get_first_null_stateidx(at);
		}
	}

	state_t	**tmp = realloc(at->states, (null_idx) * sizeof(state_t *));
	if (tmp == NULL) { return (at); };

	at->states = tmp;
	at->nstates = null_idx;

	return (at);
}

bool append_state(state_t *dst, state_t *src) {
	if (dst->table_size < src->table_size) {
		if (state_resize(dst, src->table_size))
			return (false);
	}

	if (src->accepting == true) {
		dst->accepting = true;
	}

	for (ssize_t i = 0; i < src->table_size; i++) {
		for (transition_t *t = src->table[i]; t != NULL; t = t->next) {
			transition_t *new = transition_new();
			if (new == NULL) { return (false); }
			new->state = t->state;
			if (state_add_transition(dst, new, i) == false) {
				transition_del(new);
			}
		}
	}

	return (true);
}

automata_t	*remove_epsilon(automata_t *at) {
	ssize_t	pos = 0;
	ssize_t	epsi_idx = automata_get_input_idx(at, epsilon);

	while (pos < at->nstates) {
		state_t	*st = at->states[pos];
		if (st == NULL) { ++pos; continue ; }
		
		if (pos != at->start_state && state_is_pure_epsilon(st, epsi_idx)) {
			ssize_t	dst = st->table[epsi_idx]->state;
			automata_replace(at, dst, pos);
			pos = 0;
			continue ;
		} else if (state_has_epsilon(st, epsi_idx)) {
			for (transition_t *t = st->table[epsi_idx]; t != NULL; t = st->table[epsi_idx]) {
				ssize_t	to_append_idx = t->state;
				st->table[epsi_idx] = t->next;
				transition_del(t);
				append_state(st, at->states[to_append_idx]);
			}
		}
		++pos;
	}

	return (at);
}

bool state_is_unreachable(automata_t *at, ssize_t st_idx) {
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_t	*st = at->states[i];
		if (st == NULL || i == st_idx) { continue ; }

		for (ssize_t j = 0; j < st->table_size; j++) {
			for (transition_t *t = st->table[j]; t != NULL; t = t->next) {
				if (t->state == st_idx)
					return (false);
			}
		}
	}
	return (true);
}

automata_t	*remove_unreachable(automata_t *at) {
	ssize_t	pos = 0;

	while (pos < at->nstates) {
		state_t	*st = at->states[pos];
		if (pos == at->start_state || st == NULL) { ++pos; continue ; }
		
		if (state_is_unreachable(at, pos)) {
			state_del(at->states[pos]);
			at->states[pos] = NULL;
		}
		++pos;
	}

	return (at);
}

automata_t	*validate_end(automata_t *at) {
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_t *st = at->states[i];
		if (st == NULL) { continue ; }

		if (st->accepting == true && state_is_empty(st)) {
			for (ssize_t j = 0; j < at->nstates; j++) {
				state_t *tmp = at->states[i];
				if (tmp == NULL) { continue ; }
				
				if (state_contains(tmp, i)) {
					tmp->accepting = true;
				}
			}
			state_del(at->states[i]);
			at->states[i] = NULL;
		}
	}
	return (at);
}

automata_t	*remove_ambiguity(automata_t *nfa) {
	automata_t	*new = nfa, *old = nfa;

	while (automata_has_ambiguity(old) || automata_has_epsilon(old)) {
		while (automata_has_ambiguity(old)) {
			new = combine_states(old);
			// printf("\nCOMBINE STATES\n");
			// print_automata(new);
			automata_del(old);
			if (new == NULL) {
				break ;
			}
			old = new;
		}
		new = remove_epsilon(new);
		// printf("\nREMOVE EPSILON\n");
		// print_automata(new);
		new = remove_unreachable(new);
		// printf("\nREMOVE UNREACHABLE\n");
		// print_automata(new);
		new = remove_null_states(new);
		// printf("\nREMOVE NULL STATES\n");
		// print_automata(new);
		old = new;
	}

	return (new);
}

int main(int ac, char const *av[]) {
	struct {
		bool	nfa;
		bool	mermaid;
		bool	regex;
	}	flags;
	flags.mermaid = false;
	flags.nfa = false;
	flags.regex = false;

	automata_t	*at;
	ssize_t	pos = 1;

	while (pos < ac && av[pos][0] == '-') {
		switch (av[pos][1]) {
			case 'm': flags.mermaid = true; break ;
			case 'n': flags.nfa = true; break ;
			case 'r': ++pos;
				if (pos == ac) { return (-1); }
				if (flags.regex != true) {
					at = enfa_from_str(av[pos]); flags.regex = true;
				}
				break ;
		}
		++pos;
	}

	if (pos == ac)
		return (automata_del(at), -1);

	if (flags.regex == false) {
		at = enfa_from_str(av[pos]);
	}

	if (at == NULL)
		return (automata_del(at), -1);

	if (flags.nfa == true) {
		if (flags.mermaid == true) {
			mermaid_automata(at, "stateDiagram-v2");
		} else {
			printf("eNFA\n");
			print_automata(at);
		}
		return (automata_del(at), -1);
	}
	automata_t	*dfa = remove_ambiguity(at);
	if (flags.mermaid == true) {
		mermaid_automata(dfa, "stateDiagram-v2");
	} else {
		printf("DFA\n");
		print_automata(dfa);
	}
	automata_del(dfa);
	return (0);
}
