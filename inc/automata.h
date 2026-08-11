#ifndef AUTOMATA_H
# define AUTOMATA_H

# include <unistd.h>
# include <stdbool.h>

typedef char			input_t;

static const input_t	lsqbrack = '[';
static const input_t	rsqbrack = ']';
static const input_t	lbrack = '(';
static const input_t	rbrack = ')';
static const input_t	lcurly = '{';
static const input_t	rcurly = '}';
static const input_t	pipeor = '|';
static const input_t	asterisk = '*';
static const input_t	setconnect = '-';
static const input_t	escape = '\\';
static const input_t	epsilon = -1;

typedef struct state_s	state_t;

typedef struct automata_s {
	input_t		*alphabet;
	ssize_t		alphabet_size;
	state_t		**states;
	ssize_t		nstates;
	ssize_t		start_state;
}	automata_t;

bool		automata_init(automata_t *at);
automata_t	*automata_new();
void		automata_clean(automata_t *at);
bool		automata_re(automata_t *at);
void		automata_del(automata_t *at);

ssize_t		automata_add_state(automata_t *at, state_t *st);

ssize_t		automata_get_input_idx(automata_t const *at, input_t in);
ssize_t		automata_add_input(automata_t *at, input_t in);

typedef struct combination_node_s {
	ssize_t	idx;
	ssize_t	*mapped;
	ssize_t	size;
	bool	accepting;
}	cnode_t;

bool	cnode_init(cnode_t *cn, ssize_t idx, ssize_t size);
cnode_t	*cnode_new(ssize_t idx, ssize_t size);
void	cnode_clean(cnode_t *cn);
bool	cnode_re(cnode_t *cn, ssize_t idx, ssize_t size);
void	cnode_del(cnode_t *cn);

typedef struct combination_map_s {
	cnode_t	**nodes;
	ssize_t	size;
}	cmap_t;

bool	cmap_init(cmap_t *cm);
cmap_t	*cmap_new();
void	cmap_clean(cmap_t *cm);
bool	cmap_re(cmap_t *cm);
void	cmap_del(cmap_t *cm);

bool	cmap_add_node(cmap_t *cm, cnode_t *cn);
ssize_t	cmap_contains(cmap_t *cm, cnode_t *cn);

#endif
