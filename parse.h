
/*
 *  The emit functionality needs access to this complete file.  This defines
 *  a complex data strucutre used to describe one or more complete state
 *  machines.
 */
#ifndef PARSE_H
#define PARSE_H


/*
 *  A generic list of strings.
 */
typedef struct string_list_t {
    char *strg;
    struct string_list_t *next;
} string_list_t;

typedef struct inline_list_t {
    char *name;
    char *body;
    struct inline_list_t *next;
} inline_list_t;

/*
 *  A state transition definition line.  Exactly one of these exists for every
 *  transition defined in the transitions list.
 */
typedef struct trans_line_t {
    string_list_t *names;
    //char *name;
    char *state;
    char *func;
    struct trans_line_t *next;
} trans_line_t;

typedef struct transition_t {
    string_list_t *list;
    char *state;
    char *func;
    struct transition_t *next;
} transition_t;

/*
 *  This is a full state definition with all of the transitions defined.
 */
typedef struct state_def_t {
    char *name;
    transition_t *list;
    struct state_def_t *next;
} state_def_t;

/*
 *  This is the complete machine with all of the states and transitions ready
 *  to be converted into the state transition table.
 */
typedef struct machine_t {
    char *name;
    char *input;
    char *precode;
    char *postcode;

    int num_trans;  // number of columns in the state transition table.

    // num_states and num_state_defs must match.
    int num_states; // number of lines in the state transition table.
    int num_state_defs; // number of actual state definitions.

    string_list_t *trans;   // transition enum names
    string_list_t *states;  // state enum names

    struct state_def_t *list;   // list of states with the transitions

    struct machine_t *next; // next machine definition
} machine_t;

/*
 *  List of state machines redy to be emitted to the output.
 */
typedef struct {
    char *preamble;
    char *postamble;
    inline_list_t *inline_list;
    machine_t *machine_list;
} definition_t;

definition_t *get_definition(char *name);
void free_definition(definition_t *def);

#endif /* PARSE_H */
