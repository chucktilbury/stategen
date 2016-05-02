
/*
 *  The purpose of this module is to catch all definition errors that would
 *  matter when the output is being compiled.
 *
 *  Among other things, this module should:
 *
 *  1.  Verify that there are no name collisions between states and transition
 *      names. This is not a bad error of its self, but it probably indicates an
 *      error in the definition.
 *
 *  2.  Verify that all of the state names that are defined with the states
 *      keyword are actually represented by a state definition.
 *
 *  3.  Verify that all of the state definitions are actually declaired in the
 *      states definition.  This is an error that is caught by the compiler.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parse.h"
#include "errors.h"

/*
 *  Stub this out.
 */
int validate(definition_t *def) {

    return 0;   // no errors
}
