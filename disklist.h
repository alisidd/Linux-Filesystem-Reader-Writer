#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void for_every_file(FILE*);
void print_file_details(FILE*, int);
char get_type(FILE*, int);
int get_size(FILE*, int);
void get_filename(FILE*, int, char*);
void get_extension(FILE*, int, char*);
void get_creation_date(FILE*, int);
void get_creation_time(FILE*, int);
