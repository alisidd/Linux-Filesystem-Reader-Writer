#include "constant_def.h"
#include "diskinfo.h"

int main(int argc, const char* argv[]) {
	if (argc == NUM_ARGUMENTS) {
		FILE* disk_image_file = fopen(argv[1], "rb");

		if (disk_image_file != NULL) {
			char* os_name = malloc(8);
			char* disk_label = malloc(12);

			get_os_name(disk_image_file, os_name);
			get_disk_label(disk_image_file, disk_label);

			// Disk Label may be empty and would have to be retrieved from root sector
			if (is_empty(disk_label)) {
				get_disk_label_from_files(disk_image_file, disk_label);
			}

			int total_size = get_total_size(disk_image_file);
			int free_size = get_free_size(disk_image_file);

			int num_files = get_num_files(disk_image_file);

			int num_fat_copies = get_num_fat_copies(disk_image_file);
			int num_sectors_per_fat = get_sectors_per_fat(disk_image_file);

			printf("\nOS Name: %s\n", os_name);
			printf("Label of the disk: %s\n", disk_label);
			printf("Total size of the disk: %d bytes\n", total_size);
			printf("Free size of the disk: %d bytes\n", free_size);

			printf("\n=============\n");

			printf("The number of files in the root directory (not including subdirectories): %d \n", num_files);

			printf("\n=============\n");

			printf("Number of FAT copies: %d\n", num_fat_copies);
			printf("Sectors per FAT: %d\n", num_sectors_per_fat);

			fclose(disk_image_file);
		} else {
			printf("File not found\n");
		}
	} else {
		printf("Not correct number of arguments\n");
	}
}

bool is_empty(char* string) {
	//Check for an all white spaces string
	return strspn(string, " \r\n\t") == strlen(string);
}

void get_os_name(FILE* disk_image_file, char* os_name) {
	fseek(disk_image_file, 3, SEEK_SET);
	fread(os_name, ONE_BYTE, 8, disk_image_file);
}

int get_total_size(FILE* disk_image_file) {
	return get_total_sectors(disk_image_file) * SECTOR_SIZE;
}

int get_total_sectors(FILE* disk_image_file) {
	int first_byte = 0;
	int second_byte = 0;

	fseek(disk_image_file, 19, SEEK_SET);
	fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
	fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

	// rearrange bytes to read in the right order
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

		// Get the right bytes from the FAT sector
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

void get_disk_label(FILE* disk_image_file, char* disk_label) {
	fseek(disk_image_file, VOL_LABEL_POS, SEEK_SET);
	fread(disk_label, ONE_BYTE, VOL_LABEL_SIZE, disk_image_file);
}

void get_disk_label_from_files(FILE* disk_image_file, char* disk_label) {
	int root_sector_pointer = ROOT_SECTOR * SECTOR_SIZE;
	int file_status;
	int counter = 0;

	fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
	fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);

	while (file_status != DIR_EMPTY) {
		if (file_status != DIR_UNUSED) {
			file_status = 0;
			int file_attribute;
			fseek(disk_image_file, root_sector_pointer + ATTRIBUTES_POS_OFFSET, SEEK_SET);
			fread(&file_attribute, ONE_BYTE, ONE_BYTE, disk_image_file);

			if (file_attribute & DIR_VOLUME_LABEL && file_attribute != LONG_FILENAME) {
				fseek(disk_image_file, root_sector_pointer, SEEK_SET);
				fread(disk_label, ONE_BYTE, 8, disk_image_file);
			}
		}
		root_sector_pointer += DIR_SIZE;
		fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
		fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);
	}
}

int get_num_files(FILE* disk_image_file) {
	int root_sector_pointer = ROOT_SECTOR * SECTOR_SIZE;
	int file_status;
	int file_count = 0;

	fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
	fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);

	while (file_status != DIR_EMPTY) {
		if (file_status != DIR_UNUSED) {
			int file_attribute;
			fseek(disk_image_file, root_sector_pointer + ATTRIBUTES_POS_OFFSET, SEEK_SET);
			fread(&file_attribute, ONE_BYTE, ONE_BYTE, disk_image_file);

			if (!(file_attribute & DIR_SUBDIRECTORY) && !(file_attribute & DIR_VOLUME_LABEL) && file_attribute != LONG_FILENAME) {
				file_count++;
			}
		}
		file_status = 0;
		root_sector_pointer += DIR_SIZE;
		fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
		fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);
	}

	return file_count;
}

int get_num_fat_copies(FILE* disk_image_file) {
	int num_copies = 0;
	fseek(disk_image_file, NUM_FAT_COPIES_POS, SEEK_SET);
	fread(&num_copies, NUM_FAT_COPIES_SIZE, ONE_BYTE, disk_image_file);
	return num_copies;
}

int get_sectors_per_fat(FILE* disk_image_file) {
	int sectors_per_fat = 0;
	fseek(disk_image_file, NUM_SECTORS_PER_FAT_POS, SEEK_SET);
	fread(&sectors_per_fat, ONE_BYTE, NUM_SECTORS_PER_FAT_SIZE, disk_image_file);
	return sectors_per_fat;
}
