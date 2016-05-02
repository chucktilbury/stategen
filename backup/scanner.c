
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "files.h"
#include "errors.h"
/*
    States and transitions -must- be defined in the state where they are used.
    There is no notion of nested states and any state can call another one.
    All state machines are independent of one another.
*/

enum {
    // character types
    INVALID = 0,// an invalid character
    WHITE,      // spaces tabs and line endings
    ALPNUM,     // alphanumeric characters
    NEWLINE,    // used to end a single line comment
    PUNCT,      // characters such as #,$, and @
    SQCHAR,     // the \' char
    DQCHAR,     // the \" char
    PERCENT,    // the % char
    SLASH,      // the / char
    STAR,       // the * char
    OCURLY,     // the { char
    CCURLY,     // the } char
    END_FILE,   // the EOF char
};

// this is a part of the get_cahracter function.
static unsigned int char_table[256];

/*
    The functions that are called by the state table are the output functions
    of the state machine.  They must have the template of void funcname(void);
    That means that all of the things that are done to manipulate the output
    have to be done internally.  Perhaps using global variables.  For example
    in this application, the output functions return a string that is built up
    one character at a time by the state machine.  That string must be returnd
    by a function called get_word().

    The function that retrievs the input to the state table must have the
    template int funcname(void);  It must return the state transition.  Any
    other stuff it has to return must be done by another means.  In this
    example, the input function must return the character that was read as well
    as the transition, so it is stored in a global that can be accessed by the
    output functions.

    The functions that implement the actual state machines are generated form
    the state machine definition.  State machine defineitions must have the
    template of void funcname(void);

    Error states are handled outside of the state machine.  An error causes the
    machine to exit with an indicator that can be read by the caller.  If the
    caller cares about what the error was, then that should be handled by the
    function that is called when the error is detected.

*/

static char buffer[1024*64];
static int index;
static int character;

static const char *stop = " \t\r\n";
static const char *special = "~!@#$^()-=+|\\[]:;<>,.?&"; // "%/*{}"  &
static const char *words = "0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static int read_trans(void) {

    character = read_character();
    return char_table[character];
}

static int nop(void) {
    // its an easy one....
    return 0;
}

static int invalid_char(void) {
    ERROR(SCAN_ERROR, "Invalid character '%c' (0x%02X) encountered", character, character);
    return 0;
}

static int unexpected_newline(void) {
    ERROR(SCAN_ERROR, "Unexpected newline encountered");
    return 0;
}

static int unexpected_eof(void) {
    ERROR(SCAN_ERROR, "Unexpected end of file encountered");
    return 0;
}

static int unexpected_ccurly(void) {
    ERROR(SCAN_ERROR, "Unexpected \'}\' encountered");
    return 0;
}

static int copy_char(void) {
    buffer[index++] = character;
    return 0;
}

static int pushback(void) {
    unread_character(character);
    return 0;
}

static int init_rawblock(void) {
    index = 0;
    memset(buffer, 0, sizeof(buffer));
    buffer[index++] = '%';
    buffer[index++] = '{';
    return 0;
}

static int init_copy(void) {
    index = 0;
    memset(buffer, 0, sizeof(buffer));
    buffer[index++] = character;
    return 0;
}
/*
static int init_block(void) {
    buffer[index++] = character;
    return 0;
}
*/
static int init_inlineblock(void) {
    index = 0;
    memset(buffer, 0, sizeof(buffer));
    buffer[index++] = '{';
    buffer[index++] = '{';
    return 0;
}

static int post_comment(void) {
    index = 0;
    memset(buffer, 0, sizeof(buffer));
    return 0;
}


/*******************************************************************************
 *  Generated code below this point
 *
 ******************************************************************************/

typedef struct {
    int state;
    int (*func)(void);
} state_t;

static int Scanner(void);
static int InlineBlock(void);
static int RawBlock(void);
//static int Block(void);
static int Mline(void);
static int Sline(void);
static int Dquote(void);
static int Squote(void);
static int Word(void);

//#define DEBUGGING

#ifdef DEBUGGING
#  define func_to_strg(func) ((func == nop)? "nop()": \
                            (func == invalid_char)? "invalid_char()": \
                            (func == unexpected_newline)? "unexpected_newline()": \
                            (func == unexpected_eof)? "unexpected_eof()": \
                            (func == unexpected_ccurly)? "unexpected_ccurly()": \
                            (func == copy_char)? "copy_char()": \
                            (func == pushback)? "pushback()": \
                            (func == Scanner)? "Scanner()": \
                            (func == InlineBlock)? "InlineBlock()": \
                            (func == RawBlock)? "RawBlock()": \
                            (func == Mline)? "Mline()": \
                            (func == Sline)? "Sline()": \
                            (func == Dquote)? "Dquote()": \
                            (func == Squote)? "Squote()": \
                            (func == Word)? "Word()": "UNKNOWN")

#  define PRINT(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#  define PRINT(fmt, ...)
#endif

static int Scanner(void) {

    // States
    enum { START, SPECIAL, HAVEPERCENT, HAVESLASH, HAVEOCURLY, END, ERROR, };
    // transitions
    // enum { INVALID, WHITE, ALPNUM, NEWLINE, PUNCT, SQCHAR, DQCHAR, PERCENT, SLASH, STAR, OCURLY, CCURLY, END_FILE, };

#ifdef DEBUGGING
char *state_names[] = {"START","SPECIAL","HAVEPERCENT","HAVESLASH", "HAVEOCURLY", "END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[5][13] = {
        //INVALID               WHITE         ALPNUM           NEWLINE          PUNCT                 SQCHAR           DQCHAR           PERCENT                   SLASH                   STAR                  OCURLY                   CCURLY                    END_FILE
        {{ERROR, invalid_char}, {START, nop}, {END, Word},     {START, nop},    {SPECIAL, copy_char}, {END, Squote},   {END, Dquote},   {HAVEPERCENT, copy_char}, {HAVESLASH, copy_char}, {SPECIAL, copy_char}, {HAVEOCURLY, copy_char}, {SPECIAL, copy_char},     {END, nop}},
        {{ERROR, invalid_char}, {END, nop},   {END, pushback}, {END, pushback}, {SPECIAL, copy_char}, {END, pushback}, {END, pushback}, {SPECIAL, copy_char},     {END, pushback},        {SPECIAL, copy_char}, {SPECIAL, copy_char},    {END, pushback},          {END, nop}},
        {{ERROR, invalid_char}, {END, nop},   {END, pushback}, {END, pushback}, {SPECIAL, copy_char}, {END, pushback}, {END, pushback}, {SPECIAL, copy_char},     {END, pushback},        {SPECIAL, copy_char}, {END, RawBlock},         {END, unexpected_ccurly}, {END, nop}},
        {{ERROR, invalid_char}, {END, nop},   {END, pushback}, {END, pushback}, {SPECIAL, copy_char}, {END, pushback}, {END, pushback}, {SPECIAL, copy_char},     {START, Sline},         {START, Mline},       {END, pushback},         {END, pushback},          {END, nop}},
        {{ERROR, invalid_char}, {END, nop},   {END, pushback}, {END, pushback}, {END, pushback},      {END, pushback}, {END, pushback}, {END, pushback},          {END, pushback},        {END, pushback},      {END, InlineBlock},      {END, pushback},          {END, nop}}
    };

    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();

        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);

        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}
/*
static int Block(void) {

    enum {
        START,
        END,
        ERROR,
    };

#ifdef DEBUGGING
char *state_names[] = {"START","END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[1][13] = {
        //INVALID            WHITE               ALPNUM              NEWLINE             PUNCT               SQCHAR              DQCHAR              PERCENT             SLASH               STAR                OCURLY              CCURLY              END_FILE
        {{START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, Block}, {END, copy_char}, {ERROR, unexpected_eof}}
    };
    init_block();
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}
 */
static int InlineBlock(void) {

    enum {START, HAVECCURLY, END, ERROR, };

#ifdef DEBUGGING
char *state_names[] = {"START", "HAVECURLY", "END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[2][13] = {
        //INVALID            WHITE               ALPNUM              NEWLINE             PUNCT               SQCHAR              DQCHAR              PERCENT             SLASH               STAR                OCURLY              CCURLY                     END_FILE
        {{START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {HAVECCURLY, copy_char}, {ERROR, unexpected_eof}},
        {{START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {END, copy_char},        {ERROR, unexpected_eof}}
    };
    init_inlineblock();
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}

static int RawBlock(void) {

    enum { START, HAVEPERCENT, HAVESLASH, MLINE, SLINE, HAVESTAR, SQUOTE, DQUOTE, END, ERROR, };

#ifdef DEBUGGING
char *state_names[] = {"START", "HAVEPERCENT", "HAVESLASH", "MLINE", "SLINE", "HAVESTAR", "SQUOTE", "DQUOTE", "END", "ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[8][13] = {
        //INVALID             WHITE                ALPNUM               NEWLINE              PUNCT                SQCHAR                DQCHAR                PERCENT                   SLASH                   STAR                   OCURLY               CCURLY               END_FILE
        {{START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},  {SQUOTE, copy_char},  {DQUOTE, copy_char},  {HAVEPERCENT, copy_char}, {HAVESLASH, copy_char}, {START, copy_char},    {START, copy_char},  {START, copy_char},  {ERROR, unexpected_eof}},
        {{START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},   {START, copy_char},   {START, copy_char},       {START, copy_char},     {START, copy_char},    {START, copy_char},  {END, copy_char},    {ERROR, unexpected_eof}},
        {{START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},  {START, copy_char},   {START, copy_char},   {START, copy_char},       {SLINE, copy_char},     {MLINE, copy_char},    {START, copy_char},  {START, copy_char},  {ERROR, unexpected_eof}},
        {{MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},   {MLINE, copy_char},   {MLINE, copy_char},       {MLINE, copy_char},     {HAVESTAR, copy_char}, {MLINE, copy_char},  {MLINE, copy_char},  {ERROR, unexpected_eof}},
        {{SLINE, copy_char},  {SLINE, copy_char},  {SLINE, copy_char},  {START, copy_char},  {SLINE, copy_char},  {SLINE, copy_char},   {SLINE, copy_char},   {SLINE, copy_char},       {SLINE, copy_char},     {SLINE, copy_char},    {SLINE, copy_char},  {SLINE, copy_char},  {ERROR, unexpected_eof}},
        {{MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},  {MLINE, copy_char},   {MLINE, copy_char},   {MLINE, copy_char},       {START, copy_char},     {MLINE, copy_char},    {MLINE, copy_char},  {MLINE, copy_char},  {ERROR, unexpected_eof}},
        {{SQUOTE, copy_char}, {SQUOTE, copy_char}, {SQUOTE, copy_char}, {SQUOTE, copy_char}, {SQUOTE, copy_char}, {START, copy_char},   {SQUOTE, copy_char},  {SQUOTE, copy_char},      {SQUOTE, copy_char},    {SQUOTE, copy_char},   {SQUOTE, copy_char}, {SQUOTE, copy_char}, {ERROR, unexpected_eof}},
        {{DQUOTE, copy_char}, {DQUOTE, copy_char}, {DQUOTE, copy_char}, {DQUOTE, copy_char}, {DQUOTE, copy_char}, {DQUOTE, copy_char},  {START, copy_char},   {DQUOTE, copy_char},      {DQUOTE, copy_char},    {DQUOTE, copy_char},   {DQUOTE, copy_char}, {DQUOTE, copy_char}, {ERROR, unexpected_eof}}
    };
    init_rawblock();
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}

static int Mline(void) {

    enum {
        START,
        HAVESTAR,
        END,
        ERROR,
    };

#ifdef DEBUGGING
char *state_names[] = {"START","HAVESTAR","END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[2][13] = {
        //INVALID      WHITE         ALPNUM        NEWLINE       PUNCT         SQCHAR        DQCHAR        PERCENT       SLASH         STAR             OCURLY        CCURLY        END_FILE
        {{START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {HAVESTAR, nop}, {START, nop}, {START, nop}, {ERROR, unexpected_eof}},
        {{START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {END, nop},   {START, nop},    {START, nop}, {START, nop}, {ERROR, unexpected_eof}}
    };
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    post_comment();

    return (state == END)? 0: 1;
}

static int Sline(void) {

    enum {
        START,
        END,
        ERROR,
    };

#ifdef DEBUGGING
char *state_names[] = {"START","END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[1][13] = {
        //INVALID      WHITE         ALPNUM        NEWLINE     PUNCT         SQCHAR        DQCHAR        PERCENT       SLASH         STAR          OCURLY        CCURLY        END_FILE
        {{START, nop}, {START, nop}, {START, nop}, {END, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {START, nop}, {END, nop}}
    };
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    post_comment();

    return (state == END)? 0: 1;
}

static int Dquote(void) {

    enum {
        START,
        END,
        ERROR,
    };

#ifdef DEBUGGING
char *state_names[] = {"START","END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[1][13] = {
        //INVALID            WHITE               ALPNUM              NEWLINE                      PUNCT               SQCHAR              DQCHAR              PERCENT             SLASH               STAR                OCURLY              CCURLY              END_FILE
        {{START, copy_char}, {START, copy_char}, {START, copy_char}, {ERROR, unexpected_newline}, {START, copy_char}, {START, copy_char}, {END, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {ERROR, unexpected_eof}}
    };
    init_copy();
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}

static int Squote(void) {

    enum {
        START,
        END,
        ERROR,
    };

#ifdef DEBUGGING
char *state_names[] = {"START","END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[1][13] = {
        //INVALID            WHITE               ALPNUM              NEWLINE                      PUNCT               SQCHAR            DQCHAR              PERCENT             SLASH               STAR                OCURLY              CCURLY              END_FILE
        {{START, copy_char}, {START, copy_char}, {START, copy_char}, {ERROR, unexpected_newline}, {START, copy_char}, {END, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {START, copy_char}, {ERROR, unexpected_eof}}
    };
    init_copy();
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}

static int Word(void) {

    enum {
        START,
        END,
        ERROR,
    };

#ifdef DEBUGGING
char *state_names[] = {"START","END","ERROR"};
char *trans_names[] = {"INVALID","WHITE","ALPNUM","NEWLINE","PUNCT","SQCHAR","DQCHAR","PERCENT","SLASH","STAR","OCURLY","CCURLY","END_FILE"};
#endif

    state_t states[1][13] = {
        //INVALID         WHITE       ALPNUM              NEWLINE     PUNCT            SQCHAR           DQCHAR           PERCENT          SLASH            STAR             OCURLY           CCURLY           END_FILE
        {{END, pushback}, {END, nop}, {START, copy_char}, {END, nop}, {END, pushback}, {END, pushback}, {END, pushback}, {END, pushback}, {END, pushback}, {END, pushback}, {END, pushback}, {END, pushback}, {END, nop}},
    };
    init_copy();
    int state = START;
    PRINT("\nSM %s() ENTER\n", __func__);
    do{
        int trans = read_trans();
        PRINT("state = %s: trans = %s: char = \'%c\' (0x%02X) => func: %s state: %s\n",
                state_names[state], trans_names[trans],
                (character == 0x0a)? ' ': character, character,
                func_to_strg(states[state][trans].func), state_names[states[state][trans].state]);
        (*states[state][trans].func)();
        state = states[state][trans].state;
    } while(state != END && state != ERROR);
    PRINT("SM %s() RETURNING\n", __func__);

    return (state == END)? 0: 1;
}


/*******************************************************************************
 *  End of Generated code below this point
 ******************************************************************************/

int init_scanner(void) {

    int i;

    for(i = 0; i < sizeof(char_table); i++)
        char_table[i] = INVALID;

    for(i = 0; stop[i] != 0; i++)
        char_table[(int)stop[i]] = WHITE;      // AKA whitespace

    for(i = 0; special[i] != 0; i++)
        char_table[(int)special[i]] = PUNCT;   // not stoppers or words

    for(i = 0; words[i] != 0; i++)
        char_table[(int)words[i]] = ALPNUM;       // not stoppers or special

    char_table['\"'] = DQCHAR;
    char_table['\''] = SQCHAR;
    char_table['%'] = PERCENT;
    char_table['/'] = SLASH;
    char_table['*'] = STAR;
    char_table['{'] = OCURLY;
    char_table['}'] = CCURLY;
    char_table['\n'] = NEWLINE;
    char_table[255] = END_FILE;

    return 0;
}

void destroy_scanner(void) {
    while(!files_close()) {/*empty*/;}
}

int scanner_open_file(char *name) {
    return files_open(name);
}

char *get_word(void) {
    memset(buffer, 0, sizeof(buffer));
    index = 0;
    Scanner();
    return buffer;
}

/*******************************************************************************
 *  End of Included code below this point
 ******************************************************************************/

#ifdef UNIT_TEST

int main(void) {

    init_scanner();

    scanner_open_file("test3.txt");

    while(strlen(get_word()) !=  0) {
        printf("word: %s: %d: %d: \"%s\"\n", file_name(), line_number(), strlen(buffer), buffer);
    }
    printf("\n%d lines, total\n", total_lines());


    return 0;
}

#endif
