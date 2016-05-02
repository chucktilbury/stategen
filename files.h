
#ifndef FILES_H
#define FILES_H

int files_open(char *name);
int files_close(void);
int read_character(void);
void unread_character(int ch);
char *file_name(void);
int line_number(void);
int total_lines(void);

#endif /* FILES_H */
