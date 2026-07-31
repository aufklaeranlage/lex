#include "state.h"

#include "transition.h"

#include <stdlib.h>
#include <string.h>

/**	@brief Initializes a `state_t` to have no transition table.
 */ 
bool state_init(state_t *st) {
	if (st == NULL)
		return (false);
	st->table = NULL;
	st->table_size = 0;
	st->accepting = false;
	return (true);
}

/**	@brief Allocates a new `state_t` and initialzes it.
 *  @return Returns the new `state_t` on successfull allocation and
 *  initialization; `NULL` on failure of either.
 */ 
state_t *state_new() {
	state_t	*new = malloc(sizeof(state_t));
	if (state_init(new) == false)
		return (state_del(new), NULL);
	return (new);
}

/**	@brief Cleans and frees all transtitions in a `state_t`.
 */
void state_clean(state_t *st) {
	if (st == NULL)
		return ;
	for (ssize_t i = 0; i < st->table_size; i++) {
		transition_del(st->table[i]);
	}
	free(st->table);
}

/**	@brief Cleans and frees all transitions in a `state_t`, initilaizes it to
 *  hold an empty transition table after.
 *  @return Returns `true` on successfull clean and initialization; `false` on
 *  failure of either.
 */ 
bool state_re(state_t *st) {
	state_clean(st);
	return (state_init(st));
}

/**	@brief Cleans and frees all transitions in a `state_t` and then frees the
 *  it.
 */ 
void state_del(state_t *st) {
	state_clean(st);
	free(st);
}

/**	@brief Resizes the table of a state to be of `size`.
 *  @return Returns `true` on successful reallocation; `false` on failure.
 *
 *  If a smaller size than the states table size is given this will remove
 *  transitions after index `size`.
 *
 *  Does not check for `st` being `NULL`.
 */ 
bool state_resize(state_t *st, ssize_t size) {
	transition_t	**new = realloc(st->table, size * sizeof(transition_t *));
	if (new == NULL)
		return (false);
	st->table = new;
	if (size > st->table_size)
		bzero(st->table + st->table_size, (size - st->table_size) * sizeof(transition_t *));
	st->table_size = size;
	return (true);
}

/**	@brief Adds the transition `t` to the state `st` at index `idx`.
 * 	@return Returns `true` if the addition was successfull; `false` if idx is
 * 	outside of the states tablesize.
 */
bool state_add_transition(state_t *st, transition_t *t, ssize_t idx) {
	if (idx >= st->table_size)
		return (false);
	t->next = st->table[idx];
	st->table[idx] = t;
	return (true);
}
