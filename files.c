

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "files.h"
#include "errors.h"

typedef struct file_t {
    char *name;
    FILE *fp;
    int line_no;
    char unget_buffer[50];
    int unget_index;
    struct file_t *next;
} file_t;

static file_t *fstack;
static int lines_read = 1;

int files_close(void) {

    file_t *fp = fstack ->next;

    if(NULL != fstack) {
        if(NULL != fstack->fp)
            fclose(fstack->fp);
        if(NULL != fstack->name)
            free(fstack->name);
        free(fstack);
        fstack = fp;
        return 0;
    }

    return 1;
}

int files_open(char *name) {

    file_t *filep;
    FILE *fp;

    if(name == NULL)
        SERROR(FATAL_ERROR, "File name is NULL");

    if(NULL == (fp = fopen(name, "r")))
        SERROR(FILE_ERROR, "Cannot open file \"%s\": ", name);

    if(NULL == (filep = (file_t *)calloc(1, sizeof(file_t))))
        SERROR(FATAL_ERROR, "Cannot allocate memory for scanner");

    if(NULL == (filep->name = strdup(name)))
        SERROR(FATAL_ERROR, "Cannot allocate file name");

    // everything else set to 0 by calloc()
    filep->fp = fp;
    filep->line_no = 1;

    // add it to the file stack
    if(NULL != fstack)
        filep->next = fstack;
    fstack = filep;

    return 0;
}

int read_character(void) {

    int ch;

    if(NULL != fstack) {
        if(0 != fstack->unget_index) {
            fstack->unget_index--;
            ch = fstack->unget_buffer[fstack->unget_index];
        }
        else {
            ch = fgetc(fstack->fp);
            if(EOF == ch) {
                files_close();
                ch = read_character();
            }
        }
    }
    else {
        ch = EOF;
    }

    if(ch == '\n') {
        fstack->line_no++;
        lines_read++;
    }

    return ch & 0xFF;
}

void unread_character(int ch) {

    if(NULL != fstack) {
        if(fstack->unget_index > sizeof(fstack->unget_buffer)) {
            SERROR(FATAL_ERROR, "unget_buffer overflow");
        }
        else {
            if(ch == '\n') {
                fstack->line_no--;
                lines_read--;
            }
            fstack->unget_buffer[fstack->unget_index] = ch;
            fstack->unget_index++;
        }
    }
}

char *file_name(void) {

    if(NULL != fstack) {
        return fstack->name;
    }
    else {
        return "no file is open";
    }
}

int line_number(void) {

    if(NULL != fstack) {
        return fstack->line_no;
    }
    else {
        return -1;
    }
}

int total_lines(void) {
    return lines_read;
}
