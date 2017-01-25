#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

bool appropriate_file_to_add(FILE*, FILE*);
bool file_exists(FILE*, char*);
void read_byte(FILE*, int*, int);
int get_file_size(FILE*);
void write_to_directory(FILE*, FILE*, char*, int);
void uppercase_string(char*);
int get_minimum_size(int, int);
void write_filename(FILE*, int, char*);
void write_extension(FILE*, int, char*);
struct tm* get_time();
void write_time(FILE*, int, struct tm*);
void write_date(FILE*, int, struct tm*);
void write_first_cluster(FILE*, int, int);
void write_file_size(FILE*, FILE*, int);
int convert_to_little_endian_16(int);
int convert_to_little_endian_32(int);
int get_free_directory(FILE*);
int get_next_sector(FILE*);
int arrange_bytes_FAT(int, int*, int*);
void write_to_sector(FILE*, FILE*, int, int, int);
void read_file_bytes(FILE*, char*, int, int);
void write_file_bytes(FILE*, char*, int, int);
void write_fat_entry(FILE*, int, int);
void write_previous_cluster_into_entry(FILE*, int, int);
void write_next_cluster_into_entry(FILE*, int);
void read_bytes_FAT(FILE*, int, int, int*, int*);
void append_FAT_bytes_middle(int, int, int*, int*, unsigned int*, unsigned int*);
void append_FAT_bytes_last(int, int*, int*, unsigned int*, unsigned int*);
void write_bytes_FAT(FILE*, int, int, unsigned int*, unsigned int*);
int get_total_sectors(FILE*);
int get_free_size(FILE*);
