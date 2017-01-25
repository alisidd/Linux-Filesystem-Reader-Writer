#include "diskput.h"
#include "constant_def.h"

int main(int argc, char* argv[]) {
	if (argc == NUM_ARGUMENTS_FOR_RW) {
    FILE* disk_image_file = fopen(argv[1], "r+");
    FILE* file_to_write = fopen(argv[2], "r+");
		int bytes_written = 0;

		if (disk_image_file && file_to_write) {
			if (!appropriate_file_to_add(disk_image_file, file_to_write)) {
				printf("Exiting\n");
				return -1;
			}

			int current_sector = get_next_sector(disk_image_file);
			int previous_sector = current_sector;
			write_to_directory(disk_image_file, file_to_write, argv[2], current_sector);
			int file_size_left = get_file_size(file_to_write);

			while (file_size_left > 0) {
				write_to_sector(disk_image_file, file_to_write, current_sector, file_size_left, bytes_written);
				write_fat_entry(disk_image_file, previous_sector, current_sector);
				file_size_left -= SECTOR_SIZE;
				bytes_written += SECTOR_SIZE;

				previous_sector = current_sector;
				current_sector = get_next_sector(disk_image_file);
			}
    } else {
			printf("File not found\n");
		}
  } else {
		printf("Not correct number of arguments\n");
	}
}

bool appropriate_file_to_add(FILE* disk_image_file, FILE* file_to_write) {
	if (get_free_size(disk_image_file) < get_file_size(file_to_write)) {
		printf("Not enough space\n");
		return false;
	}
	return true;
}

void read_byte(FILE* file, int* byte, int seek_position) {
	fseek(file, seek_position, SEEK_SET);
	fread(byte, ONE_BYTE, ONE_BYTE, file);
}

int get_file_size(FILE* file) {
	fseek(file, 0, SEEK_END);
	return ftell(file);
}

void write_to_directory(FILE* disk_image_file, FILE* file_to_write, char* filename, int cluster_pointer) {
	int directory_pointer = get_free_directory(disk_image_file);

	uppercase_string(filename);

	strtok(filename, ".");
	write_filename(disk_image_file, directory_pointer, filename);

	filename = strtok(NULL, ".");
	write_extension(disk_image_file, directory_pointer, filename);

	struct tm* time_now = get_time();
	write_time(disk_image_file, directory_pointer, time_now);
	write_date(disk_image_file, directory_pointer, time_now);

	write_first_cluster(disk_image_file, directory_pointer, cluster_pointer);

	write_file_size(disk_image_file, file_to_write, directory_pointer);
}

int get_minimum_size(int firstLength, int secondLength) {
	return firstLength < secondLength ? firstLength : secondLength;
}

void write_filename(FILE* disk_image_file, int directory_pointer, char* filename) {
	int name_size = get_minimum_size(strlen(filename), FILENAME_SIZE);
	fseek(disk_image_file, directory_pointer + FILENAME_POS_OFFSET, SEEK_SET);
	fwrite(filename, sizeof(char), name_size, disk_image_file);
}

void write_extension(FILE* disk_image_file, int directory_pointer, char* extension) {
	int extension_size = get_minimum_size(strlen(extension), EXTENSION_SIZE);
	fseek(disk_image_file, directory_pointer + EXTENSION_POS_OFFSET, SEEK_SET);
	fwrite(extension, sizeof(char), extension_size, disk_image_file);
}

struct tm* get_time() {
	time_t now = time(NULL);
	return localtime(&now);
}

void write_time(FILE* disk_image_file, int directory_pointer, struct tm* now_tm) {
		int hour = now_tm->tm_hour;
		int minute = now_tm->tm_min;
		int second = now_tm->tm_sec / 2;

		int first_byte = (hour << 3) + (minute >> 3);
		int second_byte = (minute << 5) + second;

		fseek(disk_image_file, directory_pointer + CREATION_TIME_POS_OFFSET, SEEK_SET);
		fwrite(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
		fwrite(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
}

void write_date(FILE* disk_image_file, int directory_pointer, struct tm* now_tm) {
	int year = now_tm->tm_year - 80;
	int month = now_tm->tm_mon + 1;
	int day = now_tm->tm_mday;

	int first_byte = (year << 1) + (month >> 3);
	int second_byte = (month << 5) + day;

	fseek(disk_image_file, directory_pointer + CREATION_DATE_POS_OFFSET, SEEK_SET);
	fwrite(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fwrite(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
}

void write_first_cluster(FILE* disk_image_file, int directory_pointer, int cluster_pointer) {
	cluster_pointer = convert_to_little_endian_16(cluster_pointer);

	int first_byte = cluster_pointer >> 8;
	int second_byte = cluster_pointer;

	fseek(disk_image_file, directory_pointer + DIR_FIRST_CLUSTER_POS_OFFSET, SEEK_SET);
	fwrite(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fwrite(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
}

void write_file_size(FILE* disk_image_file, FILE* file_to_write, int directory_pointer) {
	int file_size = get_file_size(file_to_write);
	file_size = convert_to_little_endian_32(file_size);

	int first_byte 	= file_size >> 24;
	int second_byte = file_size >> 16;
	int third_byte 	= file_size >> 8;
	int fourth_byte = file_size;

	fseek(disk_image_file, directory_pointer + FILE_SIZE_POS_OFFSET, SEEK_SET);
	fwrite(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fwrite(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fwrite(&third_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fwrite(&fourth_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
}

// Convert to little endian for 16 bit byte
int convert_to_little_endian_16(int integer) {
	return (integer << 8) | (integer >> 8);
}

// Convert to little endian for 32 bit byte
int convert_to_little_endian_32(int integer) {
	return ((integer >> 24) & 0xFF) 			|
            ((integer << 8) & 0xFF0000) |
            ((integer >> 8) & 0xFF00) 	|
          	((integer << 24) & 0xFF000000);
}

int get_free_directory(FILE* disk_image_file) {
	int root_sector_pointer = ROOT_SECTOR * SECTOR_SIZE;
	int* directory_status = malloc(sizeof(int));

	read_byte(disk_image_file, directory_status, root_sector_pointer + FILENAME_POS_OFFSET);

	while (*directory_status != DIR_UNUSED && *directory_status != DIR_EMPTY) {
		root_sector_pointer += DIR_SIZE;
		*directory_status = 0;
		read_byte(disk_image_file, directory_status, root_sector_pointer + FILENAME_POS_OFFSET);
	}

	free(directory_status);
	return root_sector_pointer;
}

int get_next_sector(FILE* disk_image_file) {
	int fat_sector_pointer = FAT_SECTOR * SECTOR_SIZE;
	int* first_byte = malloc(sizeof(int));
	int* second_byte = malloc(sizeof(int));
	int logical_cluster = 1;
	int value = -1;

	while (value != CLUSTER_UNUSED) {
		logical_cluster++;

		read_bytes_FAT(disk_image_file, fat_sector_pointer, logical_cluster, first_byte, second_byte);
		value = arrange_bytes_FAT(logical_cluster, first_byte, second_byte);

		if (logical_cluster >= CLUSTER_END) {
			printf("No enough free space in the disk image\n");
			exit(1);
		}
	}

	free(first_byte);
	free(second_byte);

	return logical_cluster;
}

int arrange_bytes_FAT(int logical_cluster, int* first_byte, int* second_byte) {
	if (logical_cluster % 2 == 0) {
		*second_byte &= LOWER_4_BITS_ON;
		return (*second_byte << 8) + *first_byte;
	} else {
		*first_byte &= UPPER_4_BITS_ON;
		return (*first_byte >> 4) + (*second_byte << 4);
	}
}

void write_to_sector(FILE* disk_image_file, FILE* file_to_write, int logical_cluster, int file_size_left, int seek_position) {
	int physical_sector = (FAT_PREDEFINED_SECTORS + logical_cluster - FAT_RESERVED_SECTORS) * SECTOR_SIZE;
	char* buffer = malloc(SECTOR_SIZE);
	int size_to_write = get_minimum_size(file_size_left, SECTOR_SIZE);

	read_file_bytes(file_to_write, buffer, seek_position, size_to_write);
	write_file_bytes(disk_image_file, buffer, physical_sector, size_to_write);

	free(buffer);
}

void read_file_bytes(FILE* file, char* buffer, int seek_position, int size) {
	fseek(file, seek_position, SEEK_SET);
	fread(buffer, ONE_BYTE, size, file);
}

void write_file_bytes(FILE* file, char* buffer, int seek_position, int size) {
	fseek(file, seek_position, SEEK_SET);
	fwrite(buffer, ONE_BYTE, size, file);
}

void write_fat_entry(FILE* disk_image_file, int logical_cluster, int next_logical_cluster) {
	write_previous_cluster_into_entry(disk_image_file, logical_cluster, next_logical_cluster);
	write_next_cluster_into_entry(disk_image_file, next_logical_cluster);
}

void write_previous_cluster_into_entry(FILE* disk_image_file, int logical_cluster, int next_logical_cluster) {
	int fat_sector_pointer = FAT_SECTOR * SECTOR_SIZE;
	int *first_byte = malloc(sizeof(int));
	int *second_byte = malloc(sizeof(int));
	unsigned int *first_byte_to_write = malloc(sizeof(unsigned int));
	unsigned int *second_byte_to_write = malloc(sizeof(unsigned int));

	read_bytes_FAT(disk_image_file, fat_sector_pointer, logical_cluster, first_byte, second_byte);
	append_FAT_bytes_middle(logical_cluster, next_logical_cluster, first_byte, second_byte, first_byte_to_write, second_byte_to_write);
	write_bytes_FAT(disk_image_file, fat_sector_pointer, logical_cluster, first_byte_to_write, second_byte_to_write);

	free(first_byte);
	free(second_byte);
	free(first_byte_to_write);
	free(second_byte_to_write);
}

void write_next_cluster_into_entry(FILE* disk_image_file, int next_logical_cluster) {
	int fat_sector_pointer = FAT_SECTOR * SECTOR_SIZE;
	int* first_byte = malloc(sizeof(int));
	int* second_byte = malloc(sizeof(int));
	unsigned int* first_byte_to_write = malloc(sizeof(unsigned int));
	unsigned int* second_byte_to_write = malloc(sizeof(unsigned int));

	read_bytes_FAT(disk_image_file, fat_sector_pointer, next_logical_cluster, first_byte, second_byte);
	append_FAT_bytes_last(next_logical_cluster, first_byte, second_byte, first_byte_to_write, second_byte_to_write);
	write_bytes_FAT(disk_image_file, fat_sector_pointer, next_logical_cluster, first_byte_to_write, second_byte_to_write);

	free(first_byte);
	free(second_byte);
	free(first_byte_to_write);
	free(second_byte_to_write);
}

void read_bytes_FAT(FILE* disk_image_file, int fat_sector_pointer, int logical_cluster, int* first_byte, int* second_byte) {
	fseek(disk_image_file, fat_sector_pointer + (3 * logical_cluster) / 2, SEEK_SET);
	fread(first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fread(second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
}

void write_bytes_FAT(FILE* disk_image_file, int fat_sector_pointer, int logical_cluster, unsigned int* first_byte, unsigned int* second_byte) {
	fseek(disk_image_file, fat_sector_pointer + (3 * logical_cluster) / 2, SEEK_SET);
	fwrite(first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fwrite(second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
}

void append_FAT_bytes_middle(int logical_cluster, int next_logical_cluster, int* first_byte, int* second_byte, unsigned int* first_byte_to_write, unsigned int* second_byte_to_write) {
	if (logical_cluster % 2 == 0) {
		*first_byte_to_write = next_logical_cluster & ALL_8_BITS_ON;
		*second_byte_to_write = ((next_logical_cluster >> 8) & LOWER_4_BITS_ON) | (*second_byte & UPPER_4_BITS_ON);
	} else {
		*first_byte_to_write = ((next_logical_cluster & LOWER_4_BITS_ON) << 4)  | (*first_byte & LOWER_4_BITS_ON);
		*second_byte_to_write = (next_logical_cluster >> 4) & ALL_8_BITS_ON;
	}
}

void append_FAT_bytes_last(int logical_cluster, int* first_byte, int* second_byte, unsigned int* first_byte_to_write, unsigned int* second_byte_to_write) {
	if (logical_cluster % 2 == 0) {
		*first_byte_to_write = ALL_8_BITS_ON;
		*second_byte_to_write = LOWER_4_BITS_ON | (*second_byte & UPPER_4_BITS_ON);
	} else {
		*first_byte_to_write = UPPER_4_BITS_ON | (*first_byte & LOWER_4_BITS_ON);
		*second_byte_to_write = ALL_8_BITS_ON;
	}
}

void uppercase_string(char* string) {
	int i = 0;
	while (string[i]) {
      string[i] = toupper(string[i]);
      i++;
  }
}

int get_total_sectors(FILE* disk_image_file) {
	int first_byte = 0;
	int second_byte = 0;

	fseek(disk_image_file, 19, SEEK_SET);
	fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

	int sector_count = (second_byte << 8) + first_byte;

	return sector_count;
}

int get_free_size(FILE* disk_image_file) {
	int fat_sector_pointer = FAT_SECTOR * SECTOR_SIZE;
	int first_byte = 0;
	int second_byte = 0;
	int n;
	int value;
	int sector_count = 0;

	for (n = 2; n < get_total_sectors(disk_image_file); n++) {

		fseek(disk_image_file, fat_sector_pointer + (3 * n) / 2, SEEK_SET);
		fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
		fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

		// Get relevant FAT bytes
		if (n % 2 == 0) {
			second_byte &= 0x0F;
			value = (second_byte << 8) + first_byte;
		} else {
			first_byte &= 0xF0;
			value = (first_byte >> 4) + (second_byte << 4);
		}

		if (value == FAT_UNUSED) {
			sector_count++;
		}
	}

	return sector_count * SECTOR_SIZE;
}
