# Linux-Filesystem-Reader-Writer

A C program that allows the user to read, write, and get from a FAT12 filesystem image on Linux

Instructions:

1. Open terminal
2. Use cd to get to the directory where the files and Makefile are stored
3. Type "make" to compile the C program
4. Type "./diskinfo disk.IMA" to get filesystem info
5. Type "./disklist disk.IMA" to list files in filesystem image
6. Type "./diskget disk.IMA nameOffile.extension" to get a file from filesystem image
7. Type "./diskput disk.IMA nameOffile.extension" to put a file into the filesystem image
