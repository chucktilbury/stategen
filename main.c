
/*
 *  Bind the whole thing together and provide for command line parsing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parse.h"
#include "emit.h"
#include "files.h"
#include "errors.h"
#include "validate.h"

static char *infile = NULL, *outfile = NULL;
static char *use_message[] = {
    "use: -i:inputfilename -o:outputfilename",
    "  -i:name   Specify the file to read from",
    "  -o:name   Specify the file to write to",
    NULL
};

static void show_use(void) {

    int i;

    for(i = 0; use_message[i] != NULL; i++)
        fprintf(stderr, "%s\n", use_message[i]);
}

/*
 *  -i:filename
 *  -o:filename
 */
static int cmd_line(int argc, char **argv) {

    int i;

    if(argc < 3) {
        fprintf(stderr, "ERROR: Input and output file names required\n");
        show_use();
        return -1;
    }

    for(i = 1; i < argc; i++) {
        switch(argv[i][1]) {
            case 'o':
                if(NULL != outfile) {
                    fprintf(stderr, "ERROR: Only one outfile may be specified\n");
                    show_use();
                    return -1;
                }
                outfile = &argv[i][3];
                break;
            case 'i':
                if(NULL != infile) {
                    fprintf(stderr, "ERROR: Only one file may be specified\n");
                    show_use();
                    return -1;
                }
                infile = &argv[i][3];
                break;
            default:
                fprintf(stderr, "ERROR: Unknown command line: %s\n", argv[i]);
                show_use();
                return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {

    definition_t *def;

    if(cmd_line(argc, argv))
        return -1;

    if(NULL == (def = get_definition(infile)))
        return 1;

    if(validate(def) != 0)
        return 1;

    emit_definition(def, outfile);

    free_definition(def);
    printf("input file: %s\n", infile);
    printf("output file: %s\n", outfile);
    printf("read %d lines, total\n", total_lines());
    return 0;
}
