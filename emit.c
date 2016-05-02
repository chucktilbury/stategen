
/*
 *  Accept the the definition_t data structure and emit the output to the
 *  stream indicated.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parse.h"
#include "errors.h"
#include "validate.h"

static FILE *fp;

static char *first_part[] = {
    "/*******************************************************************************",
    "*  Generated code below this point",
    "******************************************************************************/",
    "",
    "typedef struct {",
    "    int state;",
    "    int (*func)(void);",
    "} state_t;",
    "",
    "//#define DEBUGGING",
    "",
    "#ifdef DEBUGGING",
    "#  define PRINT(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)",
    "#else",
    "#  define PRINT(fmt, ...)",
    "#endif",
    "",
    "// Function protos",
    NULL,
};

// do not change this unless you know what you are doing.
static char *runner[] = {
    "\n",
    "    int state = START;\n",
    "    PRINT(\"\\nSM %s() ENTER\\n\", __func__);\n",
    "    do{\n",
    "        int trans = %s();\n",
    "        PRINT(\"state = %d: trans = %d: char = \'%c\' (0x%02X) => func: %s state: %d\\n\",\n",
    "                state, trans, (character == 0x0a)? ' ': character, character,\n",
    "                func_to_strg(states[state][trans].func), states[state][trans].state);\n",
    "        (*states[state][trans].func)();\n",
    "        state = states[state][trans].state;\n",
    "    }while(state != END && state != ERROR);\n",
    "    PRINT(\"SM %s() RETURNING\\n\", __func__);\n",
    "\n",
    NULL
};

static void emit_runner(char *name) {

    int i;

    for(i = 0; i < 4; i++)
        fprintf(fp, "%s", runner[i]);
    fprintf(fp, runner[i], name);
    for(i++; runner[i] != NULL; i++)
        fprintf(fp, "%s", runner[i]);

}

static char *last_part[] = {
    "",
    "// End of generated code",
    NULL
};

static inline void emit_section(char *text[]) {

    int i;

    for(i = 0; text[i] != NULL; i++)
        fprintf(fp, "%s\n", text[i]);
}

static state_def_t *select_state(machine_t *mac, char *name) {

    state_def_t *state;
    for(state = mac->list; state != NULL; state = state->next) {
        if(!strcmp(state->name, name))
            return state;
    }

    PERROR(name, "Defined in the state list but does not have a definition");
    return NULL;
}

static transition_t *select_trans(state_def_t *state, char *name) {

    transition_t *tran, *def;
    string_list_t *lst;

    for(tran = state->list; tran!= NULL; tran = tran->next) {
        for(lst = tran->list; lst != NULL; lst = lst->next)
            if(!strcmp(lst->strg, name))
                return tran;
            else if(!strcmp(lst->strg, "DEFAULT"))
                def = tran;
    }

    if(def != NULL)
        return def;
    else {
        PERROR(name, "Defined in the trans list but does not have a definition");
        return NULL;
    }
}

static void emit_trans(state_def_t *state, char *name) {

    transition_t *tran = select_trans(state, name);
    fprintf(fp, "{%s, %s}", tran->state, tran->func);
}

static void emit_states(machine_t *mac) {

    string_list_t *mlst;
    string_list_t *tlst;


    fprintf(fp, "    state_t states[%d][%d] = {\n", mac->num_states, mac->num_trans);
    for(mlst = mac->states; mlst != NULL; mlst = mlst->next) {
        state_def_t *state = select_state(mac, mlst->strg);
        //emit_line(mac, lst->strg);
        fprintf(fp, "        {");
        for(tlst = mac->trans; tlst != NULL; tlst = tlst->next) {
            emit_trans(state, tlst->strg);
            if(tlst->next != NULL)
                fprintf(fp, ", ");
        }
        fprintf(fp, "}");
        if(mlst->next == NULL)
            fprintf(fp, "\n");
        else
            fprintf(fp, ",\n");

    }
    fprintf(fp, "    };\n");
}

static void emit_machine(machine_t *machine) {

    machine_t *mac;
    string_list_t *lst;

    // emit the machine protos
    for(mac = machine; mac != NULL; mac = mac->next)
        fprintf(fp, "static int %s(void);\n", mac->name);
    fprintf(fp, "\n\n");

    // emit all of the machine definitions
    for(mac = machine; mac != NULL; mac = mac->next) {
        fprintf(fp, "static int %s(void) {\n\n", mac->name);

        fprintf(fp, "    enum { ");
        for(lst = mac->states; lst != NULL; lst = lst->next)
            fprintf(fp, "%s, ", lst->strg);
        fprintf(fp, "END, ERROR, };\n\n");
/*
fprintf(fp, "// ");
for(lst = mac->trans; lst != NULL; lst = lst->next)
    fprintf(fp, "%s, ", lst->strg);
fprintf(fp, "\n\n");
*/
        emit_states(mac);

        if(mac->precode)
            fprintf(fp, "    %s();\n", mac->precode);
        emit_runner(mac->input);
        //fprintf(fp, "    RUN_STATE(%s, %s_states);\n", mac->input, mac->name);
        if(mac->postcode)
            fprintf(fp, "    %s();\n", mac->postcode);
        fprintf(fp, "    return (state == END)? 0: -1;\n");
        fprintf(fp, "}\n\n\n");
    }
}

static void emit_amble(char *amb) {
    if(amb != NULL) {
        size_t size = fwrite(&amb[2], 1, strlen(&amb[2]) - 2, fp);
        if(size != (strlen(&amb[2]) - 2)) {
            fprintf(stderr, "EMIT ERROR: Cannot write the amble: ");
            perror("");
            exit(1);
        }
    }
}

static string_list_t *func_list = NULL;

static void add_to_string_list(string_list_t **slist, char *str) {

    string_list_t *elem, *nelem;

    for(elem = *slist; elem != NULL; elem = elem->next) {
        if(!strcmp(elem->strg, str))
            return; // do not add it if it already exists.
    }

    if(NULL == (nelem = calloc(1, sizeof(string_list_t))))
        SERROR(FATAL_ERROR, "Cannot allocate the string list element");

    if(NULL == (nelem->strg = strdup(str)))
        SERROR(FATAL_ERROR, "Cannot allocate the string element");

    if(NULL != *slist)
        nelem->next = *slist;
    *slist = nelem;
}

static void emit_inline_func(char **func) {

    static int func_no = 0;
    char buffer[10];

    snprintf(buffer, sizeof(buffer), "_%04X", func_no);
    func_no++;
    fprintf(fp, "static int %s(void) {\n", buffer);
    emit_amble(*func);
    fprintf(fp, "\n    return 0;\n}\n\n");

    free(*func);
    if(NULL == (*func = strdup(buffer)))
        SERROR(FATAL_ERROR, "Cannot allocate function from inline code");
    add_to_string_list(&func_list, *func);
}

static void emit_inline_code(definition_t *def) {

    machine_t *mac;
    state_def_t *sd;
    transition_t *trans;

    fprintf(fp, "// inline code definitions generated by software\n");
    for(mac = def->machine_list; mac != NULL; mac = mac->next) {
        for(sd = mac->list; sd != NULL; sd = sd->next) {
            for(trans = sd->list; trans != NULL; trans = trans->next) {
                if(!strncmp(trans->func, "{{", 2)) {
                    emit_inline_func(&trans->func);
                }
                else {
                    add_to_string_list(&func_list, trans->func);
                }
            }
        }

        if(NULL != mac->input) {
            if(!strncmp(mac->input, "{{", 2)) {
                emit_inline_func(&mac->input);
            }
            else {
                add_to_string_list(&func_list, mac->input);
            }
        }

        if(NULL != mac->precode) {
            if(!strncmp(mac->precode, "{{", 2)) {
                emit_inline_func(&mac->precode);
            }
            else {
                add_to_string_list(&func_list, mac->precode);
            }
        }

        if(NULL != mac->postcode) {
            if(!strncmp(mac->postcode, "{{", 2)) {
                emit_inline_func(&mac->postcode);
            }
            else {
                add_to_string_list(&func_list, mac->postcode);
            }
        }
    }
    fprintf(fp, "// end of inline code definitions\n");
}

static void emit_func_list(void) {

    string_list_t *lst = func_list;

    fprintf(fp, "#define func_to_strg(func) ( \\\n");
    for(lst = func_list; lst != NULL; lst = lst->next) {
        fprintf(fp, "                   (func == %s)? \"%s\": \\\n", lst->strg, lst->strg);
    }
    fprintf(fp, " \"UNKNOWN\")\n\n");

}

/*
 *  Top level UI
 */
void emit_definition(definition_t *def, char *name) {

    if(NULL == (fp = fopen(name, "w")))
        SERROR(FILE_ERROR, "Cannot open the output file \"%s\": ", name);

    emit_amble(def->preamble);

    emit_section(first_part);
    emit_inline_code(def);
    //emit_section(runner1);
    emit_func_list();
    //emit_section(runner2);

    emit_machine(def->machine_list);
    emit_section(last_part);

    emit_amble(def->postamble);
}


#ifdef UNIT_TEST

#include "parse.h"

int main(void) {

    definition_t *def;

    def = get_definition("scanner.sm");

    if(validate(def) != 0)
        return 1;

    emit_definition(def, "outtest.c");

    free_definition(def);

    return 0;
}

#endif
