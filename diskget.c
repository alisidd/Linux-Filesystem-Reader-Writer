#include "constant_def.h"
#include "diskget.h"

int main(int argc, char* argv[]) {
	if (argc == NUM_ARGUMENTS_FOR_RW) {
    FILE* disk_image_file = fopen(argv[1], "rb");
    char* filename = argv[2];

		if (disk_image_file != NULL) {
      int pointer = find_file(disk_image_file, filename);
      get_file(disk_image_file, filename, pointer);
    } else {
			printf("File not found\n");
		}
  } else {
		printf("Not correct number of arguments\n");
	}
}

int find_file(FILE* disk_image_file, char* filename) {
	int root_sector_pointer = ROOT_SECTOR * SECTOR_SIZE;
	int file_status = 0;
	char* filename_found = malloc(8);
	char* extension_found = malloc(3);
	char* full_file_found = malloc(11);

  fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
	fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);

	while (file_status != DIR_EMPTY) {
    if (file_status != DIR_UNUSED) {
      int file_attribute;
			fseek(disk_image_file, root_sector_pointer + ATTRIBUTES_POS_OFFSET, SEEK_SET);
			fread(&file_attribute, ONE_BYTE, ONE_BYTE, disk_image_file);

      if (!(file_attribute & DIR_VOLUME_LABEL)) {
				fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
				fread(filename_found, ONE_BYTE, FILENAME_SIZE, disk_image_file);
				// Remove all whitespace from filename
				strtok(filename_found, " ");

				fseek(disk_image_file, root_sector_pointer + EXTENSION_POS_OFFSET, SEEK_SET);
				fread(extension_found, ONE_BYTE, EXTENSION_SIZE, disk_image_file);

				sprintf(full_file_found, "%s.%s", filename_found, extension_found);

				if (strcmp(full_file_found, filename) == 0) {
					return root_sector_pointer;
				}
    	}

	    file_status = 0;
			root_sector_pointer += DIR_SIZE;
			fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
			fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);
  	}
	}
	return -1;
}

void get_file(FILE* disk_image_file, char* filename, int pointer) {
	if (pointer == -1) {
		printf("File Not Found\n");
		return;
	}

  FILE* output_file = fopen(filename, "w");
  int fat_sector_pointer = FAT_SECTOR * SECTOR_SIZE;
  int logical_cluster = 0;
	int physical_sector = 0;
	int first_byte = 0;
	int second_byte = 0;
	int total_size_left = get_size(disk_image_file, pointer);

	fseek(disk_image_file, pointer + DIR_FIRST_CLUSTER_POS_OFFSET, SEEK_SET);
	fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
  fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

	// Read the bytes in the correct order
	logical_cluster = (first_byte << 0) + (second_byte << 8);

	while (logical_cluster < CLUSTER_END) {
		printf("Next Logical Cluster: %d\n", logical_cluster);
		physical_sector = 33 + logical_cluster - 2;

		char* buffer = malloc(SECTOR_SIZE);
		int size_to_read = total_size_left < SECTOR_SIZE ? total_size_left : SECTOR_SIZE;

		fseek(disk_image_file, physical_sector * SECTOR_SIZE, SEEK_SET);
		fread(buffer, 1, size_to_read, disk_image_file);

		fwrite(buffer, sizeof(char), size_to_read, output_file);

		total_size_left -= SECTOR_SIZE;

		first_byte = 0;
		second_byte = 0;
		fseek(disk_image_file, fat_sector_pointer + (3 * logical_cluster) / 2, SEEK_SET);
		fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
		fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

		// Read the relevant FAT bytes
		if (logical_cluster % 2 == 0) {
			second_byte &= 0x0F;
			logical_cluster = (second_byte << 8) + first_byte;
		} else {
			first_byte &= 0xF0;
			logical_cluster = (first_byte >> 4) + (second_byte << 4);
		}
	}
}

int get_size(FILE* disk_image_file, int pointer) {
  int first_byte = 0;
  int second_byte = 0;
  int third_byte = 0;
  int fourth_byte = 0;

  fseek(disk_image_file, pointer + FILE_SIZE_POS_OFFSET, SEEK_SET);
	fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
  fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
  fread(&third_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
  fread(&fourth_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

  return (first_byte << 0) + (second_byte << 8) + (third_byte << 16) + (fourth_byte << 24);
}
