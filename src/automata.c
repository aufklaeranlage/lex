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
ssize_t automata_get_input_idx(automata_t const *at, input_t in) {
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

/* COMBINATION NODES **********************************************************/

/** @brief Initializes a new combination node and allocates memory for the
 * 	mapped combinations
 * 	@return Returns `true` on successfull allocation; `false` on failure to
 * 	allocate
 *
 * 	Creates a new combination node, sets its `idx` to the provided idx that
 * 	represents the new states idx that is a combination of the mapped states.
 * 	Allocates an array of size `size` in `mapped` to put the idx of the nodes
 * 	the new one should represent.
 */ 
bool cnode_init(cnode_t *cn, ssize_t idx, ssize_t size) {
	if (cn == NULL) { return (false); }
	cn->idx = idx;
	cn->size = size;
	cn->accepting = false;
	cn->mapped = calloc(size, sizeof(ssize_t));
	return (cn->mapped != NULL);
}

/**	@brief Allocates a new combination node and initializes it.
 *  @return Returns the new node on success; `NUll` on failure to allocate or
 *  initialize.
 *
 *  For information on the initialization check `cnode_init()`
 */ 
cnode_t *cnode_new(ssize_t idx, ssize_t size) {
	cnode_t	*new = malloc(sizeof(cnode_t));
	if (cnode_init(new, idx, size) == false) { return (cnode_del(new), NULL); }
	return (new);
}

/**	@brief Cleans the combination node for reuse.
 *  
 *  Frees the `mapped` array inside a node
 */ 
void cnode_clean(cnode_t *cn) {
	if (cn == NULL) { return ; }
	free(cn->mapped);
}

/**	@brief Cleans and initializes a node to the provided values.
 * 	@return Returns `true` on successfull initialization; `false` on failure to
 * 	initialize.
 */
bool cnode_re(cnode_t *cn, ssize_t idx, ssize_t size) {
	cnode_clean(cn);
	return (cnode_init(cn, idx, size));
}

/**	@brief Cleans and frees a node
 */ 
void cnode_del(cnode_t *cn) {
	cnode_clean(cn);
	free(cn);
}

/* COMBINATION MAP ************************************************************/

/**	@brief Initializes a combination map to hold no nodes and have a size of
 * 	zero
 * 	@return Returns `true` on successfull initialization; `false` on being given
 * 	a `NULL` pointer.
 */
bool cmap_init(cmap_t *cm) {
	if (cm == NULL) { return (false); }
	cm->nodes = NULL;
	cm->size = 0;
	return (true);
}

/**	@brief Allocates and initializes a new combination map.
 * 	@return Returns the new node on successfull allocatio nand initialization;
 * 	`NULL` on failure to do so.
 */
cmap_t *cmap_new() {
	cmap_t	*new = malloc(sizeof(cmap_t));
	if (cmap_init(new) == false) { return (NULL); }
	return (new);
}

/**	@brief Cleans a combination map and all it's nodes.
 */ 
void cmap_clean(cmap_t *cm) {
	if (cm == NULL) { return ; }
	for (ssize_t i = 0; i < cm->size; i++) {
		cnode_del(cm->nodes[i]);
	}
	free(cm->nodes);
	cm->nodes = NULL;
	cm->size = 0;
}

/**	@brief Cleans an initializes a combination map
 * 	@return Returns `true` on successfull re-initialization; `false` on failure
 * 	to do so.
 */
bool cmap_re(cmap_t *cm) {
	cmap_clean(cm);
	return (cmap_init(cm));
}

/**	@brief Cleans a combination map and deallocates it.
 */
void cmap_del(cmap_t *cm) {
	cmap_clean(cm);
	free(cm);
}

/**	@brief Adds a node to the combination map.
 * 	@return Returns `true` on successfull reallocation and adding; `false` on
 * 	failure to do so.
 */
bool cmap_add_node(cmap_t *cm, cnode_t *cn) {
	cnode_t	**tmp = realloc(cm->nodes, (cm->size + 1) * sizeof(cnode_t *));
	if (tmp == NULL) { return (false); }
	cm->nodes = tmp;
	cm->nodes[cm->size++] = cn;
	return (true);
}

ssize_t cmap_contains(cmap_t *cm, cnode_t *cn) {
	for (ssize_t i = 0; i < cm->size; i++) {
		// Not same size; can't match
		if (cm->nodes[i]->size != cn->size)
			continue ;

		cnode_t	*node = cm->nodes[i];
		bool	match = false;

		for (ssize_t j = 0; j < cn->size; j++) {
			ssize_t k = 0;
			for (; k < node->size; k++) {
				if (node->mapped[k] == cn->mapped[j]) {
					match = true;
					break ;
				}
			}
			if ( k == node->size) {
				match = false;
				break ;
			}
		}
		if (match == true) { return (i); }
	}
	return (-1);
}
