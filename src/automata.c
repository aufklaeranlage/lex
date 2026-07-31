#include "automata.h"

#include "state.h"

#include <stdlib.h>

/**	@brief Initialize an automata to hold no states, no alphabet, and an invalid
 * 	start_state.
 * 	@return Returns `true` on successful initialization; `false` on failure to
 * 	initialize.
 *
 * 	Protects against a `NULL` pointer being passed and returns `false` in that
 * 	case.
 */ 
bool automata_init(automata_t * at) {
	if (at == NULL)
		return (false);
	at->alphabet = NULL;
	at->alphabet_size = 0;
	at->states = NULL;
	at->nstates = 0;
	at->start_state = -1;
	return (true);
}

/**	@brief Allocates a new `automata_t` and initializes it to hold no alphabet,
 * 	no states, and an invalid start state.
 * 	@return Returns the new `automata_t` on successfull allocation and
 * 	initialization; `NULL` on failure of either.
 */ 
automata_t *automata_new() {
	automata_t	*new = malloc(sizeof(automata_t));
	if (automata_init(new) == false)
		return (automata_del(new), NULL);
	return (new);
}

/**	@brief Cleans and frees the states in an `automata_t` and frees the
 * 	alphabet.
 */ 	
void automata_clean(automata_t *at) {
	if (at == NULL)
		return ;
	free(at->alphabet);
	for (ssize_t i = 0; i < at->nstates; i++) {
		state_del(at->states[i]);
	}
	free(at->states);
}

/**	@brief Cleans and frees the states in an `automata_t`, frees the alphabet,
 *  and initializes the automata again.
 *  @return Returns `true` on successful clearing and initialization; `false` on
 *  the initialization failing or being handed a `NULL` pointer.
 */ 	
bool automata_re(automata_t *at) {
	automata_clean(at);
	return (automata_init(at));
}

/** @brief Cleans and deletes an `automata_t`.
 */ 
void automata_del(automata_t *at) {
	automata_clean(at);
	free(at);
}

/**	@brief Adds the state `st` to the automata `at`.
 * 	@return Returns the index of the state inside `at` on success; -1 on failure
 * 	to allocate memory.
 *
 * 	Does not check for any of the arguments being `NULL`.
 */
ssize_t automata_add_state(automata_t *at, state_t *st) {
	state_t	**new = realloc(at->states, (at->nstates + 1) * sizeof(state_t *));
	if (new == NULL)
		return (-1);
	ssize_t	idx = at->nstates;
	at->states = new;
	at->states[idx] = st;
	++at->nstates;
	return (idx);
}

/**	@brief Gives the index of an input `in` in automata `at`s alphabet.
 * 	@return The index of the given input if present in the alphabet; -1 if
 * 	the input is not present in the alphabet.
 */
ssize_t automata_get_input_idx(automata_t *at, input_t in) {
	ssize_t	idx = 0;
	while (idx < at->alphabet_size) {
		if (at->alphabet[idx] == in)
			return (idx);
		++idx;
	}
	return (-1);
}

/**	@brief Adds the input `in` to the automata `at`s alphabet.
 * 	@return Returns the index of the input in the alphabet on success or if the
 * 	input is already in the alphabet; -1 on not being able to add the input
 * 	because the memory allocation failed.
 */
ssize_t	automata_add_input(automata_t *at, input_t in) {
	ssize_t	idx = automata_get_input_idx(at, in);
	if (idx != -1)
		return (idx);
	input_t	*new = realloc(at->alphabet, (at->alphabet_size + 1) * sizeof(input_t));
	if (new == NULL)
		return (-1);
	idx = at->alphabet_size;
	at->alphabet = new;
	at->alphabet[idx] = in;
	++at->alphabet_size;
	return (idx);
}
