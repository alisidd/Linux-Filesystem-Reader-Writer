#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool is_empty(char*);
void get_os_name(FILE*, char*);
int get_total_size(FILE*);
int get_total_sectors(FILE*);
int get_free_size(FILE*);
int get_num_files(FILE*);
void get_disk_label(FILE*, char*);
void get_disk_label_from_files(FILE*, char*);
int get_num_fat_copies(FILE*);
int get_sectors_per_fat(FILE*);
