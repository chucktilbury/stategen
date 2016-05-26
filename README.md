# Stategen
Generates a state machine in C from a yacc-like source file. Allows generating a complex state machine from a more simple set of source files. 

## Features
* State machines can be nested.
* When a nested machine is entered, then the state of the current machine is resored when the nested machine returns.
* Arbitrary code can be called for every state transition.
* Arbitrary code can be called upon machine entry and/or exit.
* State machine is generated to portable C language.
* No external libraries required.
* Testing support.
