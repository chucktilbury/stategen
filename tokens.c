
/*
 *  Convert generic words to tokens.  Provides interface into the parser.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "scanner.h"
#include "symbols.h"
#include "tokens.h"
#include "errors.h"

#define STATIC_TOKEN -1
token_t tokens[] = {
    {"|",           PIPE_SYMBOL,        STATIC_TOKEN},
    {"{",           OCURLY_SYMBOL,      STATIC_TOKEN},
    {"}",           CCURLY_SYMBOL,      STATIC_TOKEN},
    {"};",          CCURSEMI_SYMBOL,    STATIC_TOKEN},
    {",",           COMMA_SYMBOL,       STATIC_TOKEN},
    {";",           SEMI_SYMBOL,        STATIC_TOKEN},
    {":",           COLON_SYMBOL,       STATIC_TOKEN},

    {"include",     INCLUDE_SYMBOL,     STATIC_TOKEN},
    {"machine",     MACHINE_SYMBOL,     STATIC_TOKEN},
    {"input",       INPUT_SYMBOL,       STATIC_TOKEN},
    {"states",      STATES_SYMBOL,      STATIC_TOKEN},
    {"transitions", TRANS_SYMBOL,       STATIC_TOKEN},
    {"trans",       TRANS_SYMBOL,       STATIC_TOKEN},
    {"state",       STATE_SYMBOL,       STATIC_TOKEN},
    {"pre_code",    PRECODE_SYMBOL,     STATIC_TOKEN},
    {"post_code",   POSTCODE_SYMBOL,    STATIC_TOKEN},
    {NULL, -1 -1}
};

static symbol_table_h stable;
static token_t *tok_stack[1024*1]; // a -lot- bigger than required
static int tok_stack_idx = 0;

static inline int qtest(int ch) {
    return (ch == '\'' || ch == '\"' || ch == ' ' || ch == '\t');
}

static inline char *strip_quotes(char *strg) {

    char *spt1, *spt2;

    for(spt1 = strg; qtest(*spt1); spt1++);
    memmove(strg, spt1, strlen(spt1)+1);
    for(spt2 = &spt1[strlen(spt1)]; qtest(*spt2); spt2--)
        *spt2 = 0;
    return strg;
}


int init_tokens(char *fname) {

    int i;
    symbol_t *sym;

    init_scanner();
    scanner_open_file(fname);

    stable = symbol_table_create(50);

    // save the static symbols
    for(i = 0; tokens[i].strg != NULL; i++) {
        sym = create_symbol(tokens[i].strg,
                            tokens[i].type,
                            tokens[i].stype,
                            UNION_TYPE_VOID, 0);

        symbol_table_add(stable, tokens[i].strg, sym);
    }
    return 0;
}

int unget_token(token_t *tok) {

    if(tok_stack_idx > (sizeof(tok_stack) / sizeof(token_t*)))
        SERROR(FATAL_ERROR, "Unget buffer overrun");
    tok_stack[tok_stack_idx] = tok;
    tok_stack_idx++;
    return 0;
}

token_t *get_token(void) {

    token_t *tok;
    symbol_t *sym;
    char *strg; // = get_word();

    // get from the token stack instead of the input stream.
    if(tok_stack_idx > 0) {
        tok_stack_idx--;
        tok = tok_stack[tok_stack_idx];
        return tok;
    }

    if(NULL == (tok = (token_t*)calloc(1, sizeof(token_t))))
        SERROR(FATAL_ERROR, "Cannot allocate token buffer");

    strg = get_word();
    if(NULL == (tok->strg = strdup(strg)))
        SERROR(FATAL_ERROR, "Cannot allocate token string");

    // find out what kind of token this is from the string
    if(strlen(strg) == 0) {
        tok->type = FILE_END_SYMBOL;
        tok->stype = FILE_END_SYMBOL;
    }
    else if(strg[0] == '\'' || strg[0] == '\"') {
        strip_quotes(strg);
        tok->type = QSTRG_SYMBOL;
        tok->stype = QSTRG_SYMBOL;
    }
    else if(strncmp(strg, "%{", 2) == 0) {
        tok->type = RAW_BLOCK;
        tok->stype = RAW_BLOCK;
    }
    else if(strncmp(strg, "{{", 2) == 0) {
        tok->type = INLINE_BLOCK;
        tok->stype = INLINE_BLOCK;
    }
    else {
        sym = symbol_table_find(stable, strg);
        if(sym != NULL) {
            tok->type = sym->type;
            tok->stype = sym->subtype;
        }
        else {
            tok->type = UNKNOWN_SYMBOL;
            tok->stype = UNKNOWN_SYMBOL;
        }
    }

    return tok;
}

void free_token(token_t *tok) {
    if(tok != NULL) {
        if(tok->strg != NULL)
            free(tok->strg);
        free(tok);
    }
}

int token_open_file(char *name) {
    return scanner_open_file(name);
}
//#define UNIT_TEST
#ifdef UNIT_TEST

#define TYPE_NAME(t)  ((PIPE_SYMBOL == t)? "PIPE_SYMBOL": \
                        (OCURLY_SYMBOL == t)? "OCURLY_SYMBOL": \
                        (CCURLY_SYMBOL == t)? "CCURLY_SYMBOL": \
                        (CCURSEMI_SYMBOL == t)? "CCURSEMI_SYMBOL": \
                        (COMMA_SYMBOL == t)? "COMMA_SYMBOL": \
                        (SEMI_SYMBOL == t)? "SEMI_SYMBOL": \
                        (COLON_SYMBOL == t)? "COLON_SYMBOL": \
                        (INCLUDE_SYMBOL == t)? "INCLUDE_SYMBOL": \
                        (MACHINE_SYMBOL == t)? "MACHINE_SYMBOL": \
                        (POSTCODE_SYMBOL == t)? "POSTCODE_SYMBOL": \
                        (PRECODE_SYMBOL == t)? "PRECODE_SYMBOL": \
                        (INPUT_SYMBOL == t)? "INPUT_SYMBOL": \
                        (STATES_SYMBOL == t)? "STATES_SYMBOL": \
                        (TRANS_SYMBOL == t)? "TRANS_SYMBOL": \
                        (TRANS_SYMBOL == t)? "TRANS_SYMBOL": \
                        (STATE_SYMBOL == t)? "STATE_SYMBOL": \
                        (RAW_BLOCK == t)? "RAW_BLOCK": \
                        (INLINE_BLOCK == t)? "INLINE_BLOCK": \
                        (UNKNOWN_SYMBOL == t)? "UNKNOWN_SYMBOL": \
                        (FILE_END_SYMBOL == t)? "FILE_END_SYMBOL" : "UNKNOWN")

int main(void) {

    token_t *tok;
    int type;

    init_tokens("..\\sm\\scanner.sm");

    do {
        tok = get_token();
        printf("strg: %s type: %s stype = %s\n", tok->strg, TYPE_NAME(tok->type), TYPE_NAME(tok->stype));
        type = tok->type;
        free_token(tok);
    } while(type != FILE_END_SYMBOL);

    return 0;
}

#endif
