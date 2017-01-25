#include "constant_def.h"
#include "disklist.h"

int main(int argc, const char* argv[]) {
  if (argc == NUM_ARGUMENTS) {
    FILE *disk_image_file = fopen(argv[1], "rb");

		if (disk_image_file != NULL) {
      for_every_file(disk_image_file);
      fclose(disk_image_file);
    } else {
      printf("File not found\n");
    }
  } else {
    printf("Not correct number of arguments\n");
  }
}

void for_every_file(FILE* disk_image_file) {
  int root_sector_pointer = ROOT_SECTOR * SECTOR_SIZE;
	int file_status;

  fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
	fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);

	while (file_status != DIR_EMPTY) {
    if (file_status != DIR_UNUSED) {
      int file_attribute;
			fseek(disk_image_file, root_sector_pointer + ATTRIBUTES_POS_OFFSET, SEEK_SET);
			fread(&file_attribute, ONE_BYTE, ONE_BYTE, disk_image_file);

      if (!(file_attribute & DIR_VOLUME_LABEL) && file_attribute != LONG_FILENAME)
        print_file_details(disk_image_file, root_sector_pointer);
    }

    file_status = 0;
		root_sector_pointer += DIR_SIZE;
		fseek(disk_image_file, root_sector_pointer + FILENAME_POS_OFFSET, SEEK_SET);
		fread(&file_status, ONE_BYTE, ONE_BYTE, disk_image_file);
  }
}

void print_file_details(FILE* disk_image_file, int pointer) {
  char type = get_type(disk_image_file, pointer);
  int size = get_size(disk_image_file, pointer);

  // Read filename and extension separately and join them together
  char* filename = malloc(11);
  get_filename(disk_image_file, pointer, filename);
  filename = strtok(filename, " ");

  char* extension = malloc(3);
  get_extension(disk_image_file, pointer, extension);

  printf("%c %10d %20s.%s ", type, size, filename, extension);
  get_creation_date(disk_image_file, pointer);
  get_creation_time(disk_image_file, pointer);
}

char get_type(FILE* disk_image_file, int pointer) {
  int file_type = 0;
  fseek(disk_image_file, pointer + ATTRIBUTES_POS_OFFSET, SEEK_SET);
	fread(&file_type, ONE_BYTE, ONE_BYTE, disk_image_file);

  if (file_type & DIR_SUBDIRECTORY) {
    return 'D';
  } else {
    return 'F';
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

  return (first_byte << 0) + (second_byte << 8) + (third_byte << 16) + (fourth_byte << 24);;
}

void get_filename(FILE* disk_image_file, int pointer, char* filename) {
  fseek(disk_image_file, pointer + FILENAME_POS_OFFSET, SEEK_SET);
	fread(filename, ONE_BYTE, FILENAME_SIZE, disk_image_file);
}

void get_extension(FILE* disk_image_file, int pointer, char* extension) {
  fseek(disk_image_file, pointer + EXTENSION_POS_OFFSET, SEEK_SET);
	fread(extension, ONE_BYTE, EXTENSION_SIZE, disk_image_file);
}

void get_creation_date(FILE* disk_image_file, int pointer) {
  int first_byte = 0;
  int second_byte = 0;

  fseek(disk_image_file, pointer + CREATION_DATE_POS_OFFSET, SEEK_SET);
  fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
  fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

  // Extract the correct bits and convert them to year, month and day
  int year = ((second_byte & 0xFE) >> 1) + 1980;
  int month = ((first_byte & 0xE0) >> 5) + ((second_byte & 0x01) << 3);
  int day = first_byte & 0x1F;
  printf("%d-%d-%d ", year, month, day);
}

void get_creation_time(FILE* disk_image_file, int pointer) {
  int first_byte = 0;
  int second_byte = 0;

  fseek(disk_image_file, pointer + CREATION_TIME_POS_OFFSET, SEEK_SET);
  fread(&first_byte, ONE_BYTE, ONE_BYTE, disk_image_file);
  fread(&second_byte, ONE_BYTE, ONE_BYTE, disk_image_file);

  // Extract the correct bits and convert them to hour and minute
  int hours = (second_byte & 0xF8) >> 3;
  int minutes = ((first_byte & 0xE0) >> 5) + ((second_byte & 0x07) << 3);
  printf("%02d:%02d\n", hours, minutes);
}
