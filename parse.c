
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tokens.h"
#include "files.h"
#include "hashtable.h"
#include "parse.h"
#include "errors.h"

//static unsigned int parse_errors = 0;

static inline int count_strings(string_list_t *list) {

    int i;
    string_list_t *elem;

    for(elem = list, i = 0;
        elem != NULL; elem = elem->next, i++) { /* empty */; }

    return i;
}
/*
static inline int qtest(int ch) {
    return (ch == '\'' || ch == '\"' || ch == ' ' || ch == '\t');
}
*/
/*
 *  Recursive decent parser for the state machine definition
 */

/*
 *  Remove the quote character from both ends of the string and also remove
 *  any white space at the beginning or the end.
 */
/*
static char *strip_quotes(char *strg) {

    char *spt1, *spt2;

    for(spt1 = strg; qtest(*spt1); spt1++);
    for(spt2 = &spt1[strlen(spt1)]; qtest(*spt2); spt2--)
        *spt2 = 0;
    return spt1;
}
 */
/*
 *  Handle the include directive.
 */
static int include_file(void) {

    token_t *tok = get_token();
    if(tok->type != QSTRG_SYMBOL) {
        SERROR(SYNTAX_ERROR, "Include directive requires a quoted string");
        return 1;
    }
    else {
        token_open_file(tok->strg);
    }
    free_token(tok);
    return 0;
}

/*
 *  Get a string that is terminated with a ';'.  Can be either a quoted string
 *  or a single word.
 */
static char *get_single(void) {

    char *strg; //, *spt;
    token_t *tok = get_token();

    // get the name of the element
    switch(tok->type) {
        case QSTRG_SYMBOL:
        case INLINE_BLOCK:
        case UNKNOWN_SYMBOL:
            if(NULL == (strg = strdup(tok->strg)))
                SERROR(FATAL_ERROR, "Cannot allocate single");
            break;
        default:
            SERROR(SYNTAX_ERROR, "Unexpected \"%s\" token", tok->strg);
            return NULL;
    }
    free_token(tok);

    // eat the ';' character
    tok = get_token();
    if(SEMI_SYMBOL != tok->type) {
        SERROR(SYNTAX_ERROR, "Expected  a \';\' but got a \"%s\" token", tok->strg);
        return NULL;
    }
    free_token(tok);
    return strg;
}

/*
 *  Free a string list
 */
static void free_string_list(string_list_t *list) {

    string_list_t *sl, *nsl;

    for(sl = list; sl != NULL; sl = nsl) {
        nsl = sl->next;
        if(sl->strg != NULL)
            free(sl->strg);
        free(sl);
    }
}

/*
 *  Add the given string to the given list.
 */
static void add_to_string_list(string_list_t **slist, char *str) {

    string_list_t *nelem;

    if(NULL == (nelem = calloc(1, sizeof(string_list_t))))
        SERROR(FATAL_ERROR, "Cannot allocate the string list element");

    if(NULL == (nelem->strg = strdup(str)))
        SERROR(FATAL_ERROR, "Cannot allocate the string element");

    if(NULL != *slist)
        nelem->next = *slist;
    *slist = nelem;
}

/*
 *  Read a list of tokens where they are separated by a comma and terminated
 *  with a semi-colon.  Save the list into the first parameter and then return
 *  the number of items that were stored.
 */
static int get_list(string_list_t **list, int sep, int term) {

    token_t *tok;
    int items = 0;

    do {
        // get the name
        tok = get_token();
        if(UNKNOWN_SYMBOL == tok->type) {
            add_to_string_list(list, tok->strg);
            items++;
        }
        else {
            SERROR(SYNTAX_ERROR, "Expected  a name but got a \"%s\" token", tok->strg);
            free_string_list(*list);
            return 0;
        }
        free_token(tok);

        // get a ',' or a ';'.  If it is the ';', then exit.
        tok = get_token();
        if(term == tok->type) {
            return items;
        }
        else if(sep != tok->type) {
            SERROR(SYNTAX_ERROR, "Unexpected \"%s\" token", tok->strg);
            free_string_list(*list);
            return 0;
        }
        free_token(tok);
    } while(1);

    free_string_list(*list);
    return 0; // never happens
}

static int state_definition(machine_t *machine) {

    token_t *tok;
    state_def_t *sd;
    transition_t *tl;
    int finished = 0;

    // create the state
    if(NULL == (sd = calloc(1, sizeof(state_def_t))))
        SERROR(FATAL_ERROR, "Cannot allocate state structure");

    // read the state name
    tok = get_token();
    if(NULL == (sd->name = strdup(tok->strg)))
        SERROR(FATAL_ERROR, "Cannot allocate state name");
    free_token(tok);

    // read the obligatory "{"
    tok = get_token();
    if(tok->type != OCURLY_SYMBOL) {
        SERROR(SYNTAX_ERROR, "Expected a \"{\" but got a \"%s\" token", tok->strg);
        return 1;
    }
    free_token(tok);

    // read all of the lines for the state until the "};" token is seen
    do {
        // allocate the state transition
        if(NULL == (tl = calloc(1, sizeof(transition_t))))
            SERROR(FATAL_ERROR, "Cannot allocate transition structure");

        // read the trans list
        if(0 == get_list(&tl->list, PIPE_SYMBOL, COLON_SYMBOL)) {
            SERROR(PARSE_ERROR, "Cannot read transition list");
            return 1;
        }

        // get the state
        tok = get_token();
        if(UNKNOWN_SYMBOL != tok->type) {
            SERROR(SYNTAX_ERROR, "Expected a name but got a \"%s\" token", tok->strg);
            return 1;
        }
        else {
            if(NULL == (tl->state = strdup(tok->strg)))
                SERROR(FATAL_ERROR, "Cannot allocate transition name");
        }
        free_token(tok);

        // get the code
        tok = get_token();
        if(UNKNOWN_SYMBOL != tok->type && INLINE_BLOCK != tok->type) {
           SERROR(SYNTAX_ERROR, "Expected a name or inline block but got a \"%s\" token", tok->strg);
            return 1;
        }
        else {
            if(NULL == (tl->func = strdup(tok->strg)))
                SERROR(FATAL_ERROR, "Cannot allocate transition name");
        }
        free_token(tok);

        tok = get_token();
        if(SEMI_SYMBOL != tok->type) {
            SERROR(SYNTAX_ERROR, "Expected a \";\" but got a \"%s\" token", tok->strg);
            return 1;
        }
        free_token(tok);

        // add the transition to the list
        if(sd->list != NULL)
            tl->next = sd->list;
        sd->list = tl;

        // check to see if the "};" token is present
        tok = get_token();
        if(CCURSEMI_SYMBOL != tok->type) {
            /*
            printf("SYNTAX ERROR: %s: %d: Expected a \"};\" but got a \"%s\" token\n",
                    file_name(), line_number(), tok->strg);
            parse_errors++;
            return parse_errors;
            */
            unget_token(tok);
        }
        else {
            finished = 1;
            free_token(tok);
        }
    } while(0 == finished);

    // add the state def to the machine
    if(machine->list != NULL)
        sd->next = machine->list;
    machine->list = sd;

    return 0;   // no error
}

/*
 *  Read and store a machine definition.  Returns 0 if there is no error.
 */
static int machine_definition(definition_t *def) {

    token_t *tok;
    machine_t *machine;
    //state_def_t *states;
    int errors = 0, finished = 0; // count = 0,

    // create a new machine data strucutre
    if(NULL == (machine = calloc(1, sizeof(machine_t))))
        SERROR(FATAL_ERROR, "Cannot allocate machine structure");

    // get the name of the machine
    tok = get_token();
    if(NULL == (machine->name = strdup(tok->strg))) {
        free_token(tok);
        SERROR(FATAL_ERROR, "Cannot allocate machine name");
    }
    free_token(tok);

    // get the "{" token
    tok = get_token();
    if(tok->type != OCURLY_SYMBOL) {
        SERROR(SYNTAX_ERROR, "Expected a \"{\" but got a \"%s\" token", tok->strg);
        free_token(tok);
        return 1;
    }
    free_token(tok);

    // loop looking for the '};' token
    do {
        tok = get_token();
        switch(tok->type) {
            case INPUT_SYMBOL:
                if(machine->input != NULL) {
                    SERROR(SYNTAX_ERROR, "Only one \"input\" directive is allowed per machine");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }

                machine->input = get_single();
                if(NULL == machine->input) {
                    SERROR(PARSE_ERROR, "Cannot read input specification");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                break;

            case PRECODE_SYMBOL:
                if(machine->precode != NULL) {
                    SERROR(SYNTAX_ERROR, "Only one \"precode\" directive is allowed per machine");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }

                machine->precode = get_single();
                if(NULL == machine->precode) {
                    SERROR(PARSE_ERROR, "Cannot read pre_code specification");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                break;

            case POSTCODE_SYMBOL:
                if(machine->postcode != NULL) {
                    SERROR(SYNTAX_ERROR, "Only one \"postcode\" directive is allowed per machine");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }

                machine->postcode = get_single();
                if(NULL == machine->postcode) {
                    SERROR(PARSE_ERROR, "Cannot read post_code specification");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                break;

            case TRANS_SYMBOL:
                if(machine->trans != NULL) {
                    SERROR(SYNTAX_ERROR, "Only one \"transitions\" directive is allowed per machine");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }

                machine->num_trans  = get_list(&machine->trans, COMMA_SYMBOL, SEMI_SYMBOL);
                if(NULL == machine->trans) {
                    SERROR(PARSE_ERROR, "Cannot read transition list specification");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                break;

            case STATES_SYMBOL:
                if(machine->states != NULL) {
                    SERROR(SYNTAX_ERROR, "Only one \"states\" directive is allowed per machine");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                //add_to_string_list(&machine->states, "ERROR");
                //add_to_string_list(&machine->states, "END");

                machine->num_states = get_list(&machine->states, COMMA_SYMBOL, SEMI_SYMBOL);
                if(NULL == machine->states) {
                    SERROR(PARSE_ERROR, "Cannot read states list specification");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                break;

            case STATE_SYMBOL:
                if( 0 != state_definition(machine)) {
                    SERROR(PARSE_ERROR, "Cannot read state specification");
                    errors++;
                    finished = 1;
                    break; // return parse_errors;
                }
                break;

            case FILE_END_SYMBOL:
                SERROR(PARSE_ERROR, "Unexpected end of file encountered");
                errors++;
                /* fall through */
            case CCURSEMI_SYMBOL:
                finished = 1;
                break;

            default:
                SERROR(SYNTAX_ERROR, "Unexpected \"%s\" token", tok->strg);
                errors++;
                finished = 1;
                break; //return parse_errors;
        }
        free_token(tok);
    } while(0 == finished && 0 == errors);

    add_to_string_list(&machine->states, "START");
    machine->num_states += 1;

    if(errors == 0) {
        // Add the machine to the list.  Last in list = first defined.
        if(def->machine_list != NULL)
            machine->next = def->machine_list;
        def->machine_list = machine;
    }

    return errors;   // no error
}

static int parse(char *name, definition_t *def) {

    token_t *tok;
    int finished = 0;

    init_tokens(name);

    do {
        tok = get_token();
        switch(tok->type) {
            case RAW_BLOCK:
                if(NULL == def->preamble) {
                    if(NULL == (def->preamble = strdup(tok->strg)))
                        SERROR(FATAL_ERROR, "Cannot allocate preamble");
                }
                else if(NULL == def->postamble) {
                    if(NULL == (def->postamble = strdup(tok->strg)))
                        SERROR(FATAL_ERROR, "Cannot allocate postamble");
                }
                else {
                    SERROR(SYNTAX_ERROR, "Unexpected raw block found");
                    return 1;
                }
                break;

            case INCLUDE_SYMBOL:
                if(0 != include_file())
                    return 1;
                break;

            case MACHINE_SYMBOL:
                if(0 != machine_definition(def))
                    return 1;
                break;

            case FILE_END_SYMBOL:
                finished = 1;
                break;

            default:
                SERROR(SYNTAX_ERROR, "Unexpected \"%s\" token", tok->strg);
                break;
        }
        free_token(tok);
    } while(0 == finished);

    return 0;
}
/*
static void free_string_list(string_list_t *list) {

    string_list_t *sl, *next;

    for(sl = list; sl != NULL; sl = next) {
        next = sl->next;
        if(NULL != sl->strg)
            free(sl->strg);
        free(sl);
    }
}
 */
static void free_trans_list(transition_t *list) {

    transition_t *tl, *next;

    for(tl = list; tl != NULL; tl = next) {
        next = tl->next;
        if(NULL != tl->state)
            free(tl->state);
        if(NULL != tl->func)
            free(tl->func);
        free_string_list(tl->list);
        free(tl);
    }
}

static void free_state_list(state_def_t *state_def) {

    state_def_t *sd, *next;

    for(sd = state_def; sd != NULL; sd = next) {
        next = sd->next;
        if(NULL != sd->name)
            free(sd->name);
        free_trans_list(sd->list);
        free(sd);
    }
}

static void free_machine_list(machine_t *machine) {

    machine_t *mac, *next;

    for(mac = machine; mac != NULL; mac = next) {
        next = mac->next;
        if(NULL != mac->name)
            free(mac->name);
        if(NULL != mac->precode)
            free(mac->precode);
        if(NULL != mac->postcode)
            free(mac->postcode);
        if(NULL != mac->trans)
            free_string_list(mac->trans);
        if(NULL != mac->states)
            free_string_list(mac->states);
        if(NULL != mac->list)
            free_state_list(mac->list);
        free(mac);
    }
}

/*
 *  External user interface.
 */
void free_definition(definition_t *def) {

    if(NULL != def) {
        if(NULL != def->preamble)
            free(def->preamble);
        if(NULL != def->postamble)
            free(def->postamble);
        free_machine_list(def->machine_list);
        free(def);
    }
}

definition_t *get_definition(char *name) {

    definition_t *def;

    if(NULL == (def = calloc(1, sizeof(definition_t))))
        SERROR(FATAL_ERROR, "Cannot allocate definition data strucutre");

    if(0 != parse(name, def)) {
        free_definition(def);
        return NULL;
    }
    return def;
}

//#define UNIT_TEST
#ifdef UNIT_TEST

static void dump_list(string_list_t *list) {

    string_list_t *l;

    for(l = list; l != NULL; l = l->next)
        printf("        %s\n", l->strg);
}

static void dump_trans(transition_t *trans) {

    transition_t *tr;

    for(tr = trans; tr != NULL; tr = tr->next) {
        printf("    TRANS LIST:\n");
        dump_list(tr->list);
        if(tr->state != NULL)
            printf("        STATE: %s\n", tr->state);
        else
            printf("        STATE: (none defined)\n");

        if(tr->func != NULL)
            printf("        FUNC: %s\n", tr->func);
        else
            printf("        FUNC: (none defined)\n");

    }
}

static void dump_states(state_def_t *state_def) {

    state_def_t *sd;

    for(sd = state_def; sd != NULL; sd = sd->next) {
        if(sd->name != NULL)
            printf("    STATE NAME: %s\n", sd->name);
        else
            printf("    STATE NAME: (none defined)\n");

        dump_trans(sd->list);
    }
}

static void dump_mac(machine_t *mac) {

    if(mac->name != NULL)
        printf("MACHINE NAME: %s\n", mac->name);
    else
        printf("MACHINE NAME: (none defined)\n");
    if(mac->input != NULL)
        printf("    INPUT: %s\n", mac->input);
    else
        printf("    INPUT: (none defined)\n");
    if(mac->precode != NULL)
        printf("    PRECODE: %s\n", mac->precode);
    else
        printf("    PRECODE: (none defined)\n");
    if(mac->postcode != NULL)
        printf("    POSTCODE: %s\n", mac->postcode);
    else
        printf("    POSTCODE: (none defined)\n");
    if(mac->states != NULL) {
        printf("    STATES:\n");
        dump_list(mac->states);
    }
    else
        printf("    STATES:\n        NONE\n");
    if(mac->trans != NULL) {
        printf("    TRANSITIONS:\n");
        dump_list(mac->trans);
    }
    else
        printf("    TRANSITIONS:\n        NONE\n");
    dump_states(mac->list);
}

static void dump_def(definition_t *def) {

    machine_t *mac;

    if(def->preamble != NULL)
        printf("PREAMBLE\n%s\n", def->preamble);
    else
        printf("PREAMBLE (none)\n");

    if(def->postamble != NULL)
        printf("POSTAMBLE\n%s\n", def->postamble);
    else
        printf("POSTAMBLE (none)\n");

    for(mac = def->machine_list; mac != NULL; mac = mac->next)
        dump_mac(mac);
}

int main(void) {

    definition_t *def;

    def = get_definition("..\\sm\\scanner.sm");

    dump_def(def);

    free_definition(def);

    return 0;
}

#endif
