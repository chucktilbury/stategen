
#ifndef SCANNER_H
#define SCANNER_H

int init_scanner(void);
void destroy_scanner(void);
int scanner_open_file(char *name);
char *get_word(void);

#endif /* SCANNER_H */
