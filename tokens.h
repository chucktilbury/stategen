
#ifndef TOKENS_H
#define TOKENS_H

typedef struct {
    char *strg;
    int type;
    int stype;
} token_t;

int init_tokens(char *name);
token_t *get_token(void);
int unget_token(token_t *t);
int token_open_file(char *name);
void free_token(token_t *tok);

// all of the token types that are used in the parser
enum {
    PIPE_SYMBOL,
    OCURLY_SYMBOL,
    CCURLY_SYMBOL,
    CCURSEMI_SYMBOL,
    COMMA_SYMBOL,
    SEMI_SYMBOL,
    COLON_SYMBOL,
    INCLUDE_SYMBOL,
    MACHINE_SYMBOL,
    INPUT_SYMBOL,
    STATES_SYMBOL,
    TRANS_SYMBOL,
    STATE_SYMBOL,
    PRECODE_SYMBOL,
    POSTCODE_SYMBOL,
    QSTRG_SYMBOL,
    UNKNOWN_SYMBOL,
    FILE_END_SYMBOL,
    RAW_BLOCK,
    INLINE_BLOCK,

};


#endif /* TOKENS_H */
