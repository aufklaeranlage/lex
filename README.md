# ft_lex

## Structures

In general Deterministic and Non-Deterministic state machines are very similar. They consist of a number of states, an alphabet of valid input and a table of transitions for each state that gives information about what state should follow for a certain input. 

The main difference is that a Deterministic state machine hast only one valid state for each input while a Non-Deterministic state machine has multiple.

This prompts a certain design for state machines:

## The Automata

The automata holds the alphabet and the states:

```c
typedef struct automata {
    input_t     *alphabet;
    ssize_t     alphabet_size;
    state_t     **states;
    ssize_t     nstates;
    ssize_t     start_state;
}   automata_t;
```

In the above example `input_t` is a vaguely typed representation of the valid inputs for this automata.

To be able to quickly identify a next state without using up to much space a function that converts an `input_t` into a index of the alphabet should be implemented. This way each state only needs `alphabet_size` number of transitions in it's transition table to be functional, instead of a full range from the lowest possible imput to the highest.

To be able to achieve this indexing through the alphabet a function that converts an `input_t` based on an automatas alphabet should be implemented.

Each Automata has one start state. This state should be saved in the automata for easy access. Since all the states are indexed starting at 0 the start state can just be a number that refers to the index of the start state.

## The State

In both Deterministic and Non-Deterministic Automatas the states look very similar: they consists of a table of transitions that has the same size as the alphabet of the automata. To make sure that later additions to an automatas alphabet can be reflected, these tables should be able to be resized.

Since not every state has a transition for each `input_t` the transition table should be an array of pointers to transitions. This way for every `input_t` that is not registered for this state, a `NULL` pointer can be put into the corresponding slot. This way we can easily recognize an unregistered `input_t` for a state.

Since the state is mostly useful to be a reference for the possible transitions the states themselves don't need to hold information about the possible inputs that led to them. This information is stored in the transitions through their index in the states table.

This leads to a rough desing lke this:

```c
typedef struct state {
    transition_t    **table;
    ssize_t         table_size;
}   state_t;
```

## The Transitions

A transition itself only needs to hold the information about what state should be the next active state. The difference is that a Deterministic Automata can only have one viable transition while a Non-Deterministic one can have multiple.

To simplify the design and not have to recreated the structs that use a transition the transitions themselves can be designed as linked lists and only the logic to add new connections needs to be changed for Deterministic and Non-Deterministic state machines.

```c
typedef struct transition {
    state_t             *state;
    struct transition   *next;
}   transition_t;
```
