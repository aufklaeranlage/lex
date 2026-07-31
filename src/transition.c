#include "transition.h"

# include <stdlib.h>

bool transition_init(transition_t *t) {
	if (t == NULL)
		return (false);
	t->state = -1;
	t->next = NULL;
	return (true);
}

transition_t *transition_new() {
	transition_t	*new = malloc(sizeof(transition_t));
	if (transition_init(new) == false)
		return (transition_del(new), NULL);
	return (new);
}

void transition_del(transition_t *t) {
	free(t);
}

void transition_del_list(transition_t *head) {
	if (head == NULL)
		return ;

	transition_t	*next;
	while (head != NULL) {
		next = head->next;
		transition_del(head);
		head = next;
	}
}
