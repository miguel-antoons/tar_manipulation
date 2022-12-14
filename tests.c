#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}


/**  
*   zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
*   -1 if the archive contains a header with an invalid magic value,
*   -2 if the archive contains a header with an invalid version value,
*   -3 if the archive contains a header with an invalid checksum value
*/
void test_check_archive() {
    int fd = open("archive.tar", O_RDONLY);
    if (fd == -1) {
        perror("open(archive.tar)");
        return;
    }

    int ret = check_archive(fd);
    if(ret == 0){
        printf("check_archive returned 0, archive is valid :D\n");
    } else if (ret == -1){
        printf("check_archive returned -1, archive contains a header with an invalid magic value\n");
    } else if (ret == -2){
        printf("check_archive returned -2, archive contains a header with an invalid version value\n");
    } else if (ret == -3){
        printf("check_archive returned -3, archive contains a header with an invalid checksum value\n");
    } else {
        printf("check_archive returned %d, archive is valid and contains %d non-null headers :D\n", ret, ret);
    }

    printf("\n");
}

void test_exists() {
    int fd = open("archive.tar", O_RDONLY);
    if (fd == -1) {
        perror("open(archive.tar)");
        return;
    }

    int ret = exists(fd, "folder/tests.c");
    if(ret == 0) {
        printf("exists returned 0, file exists :D\n");
    } else {
        printf("exists returned %d, file does not exist\n", ret);
    }

    printf("\n");
}


void test_is_dir() {
    int fd = open("archive.tar", O_RDONLY);
    if (fd == -1) {
        perror("open(archive.tar)");
        return;
    }

    int ret = is_dir(fd, "folder/");
    if(ret == 0) {
        printf("is_dir returned 0, folder exists :D\n");
    } else {
        printf("is_dir returned %d, folder does not exist\n", ret);
    }

    printf("\n");
}

/**
 *  -1 if no entry at the given path exists in the archive or the entry is not a file,
 *  -2 if the offset is outside the file total length,
 *  zero if the file was read in its entirety into the destination buffer,
 *  a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *  the end of the file.
*/

void test_read_file() {
    int fd = open("archive.tar", O_RDONLY);
    if (fd == -1) {
        perror("open(archive.tar)");
        return;
    }

    // create buffer to store the file
    size_t size = 9728;
    uint8_t *buffer = malloc(size);

    size_t ret = read_file(fd, "lib_tar.c", 6000, buffer, &size);
    printf("read_file returned %ld\n", ret);

   if(ret == 0) {
        printf("read_file returned 0, file was read in its entirety into the destination buffer :D\n");
    } else if (ret == -1) {
        printf("read_file returned -1, no entry at the given path exists in the archive or the entry is not a file\n");
    } else if (ret == -2) {
        printf("read_file returned -2, the offset is outside the file total length\n");
    } else {
        printf("read_file returned %ld, file was partially read, representing the remaining bytes left to be read to reach the end of the file :D\n", ret);
    }

    printf("\n");

    free(buffer);
}



/** 
 *     zero if no directory at the given path exists in the archive,
 *     any other value otherwise.
*/

void test_list() {
    int fd = open("archive.tar", O_RDONLY);
    if (fd == -1) {
        perror("open(archive.tar)");
        return;
    }

    // create An array of char arrays, each one is long enough to contain a tar entry path.
    size_t no_entries = 6;
    // char **string_array = malloc(no_entries * 100 * sizeof(char *));
    char *string_array[no_entries];
    char string[100] = {0};
    char string1[100] = {0};
    char string2[100] = {0};
    char string3[100] = {0};
    char string4[100] = {0};
    char string5[100] = {0};

    string_array[0] = string;
    string_array[1] = string1;
    string_array[2] = string2;
    string_array[3] = string3;
    string_array[4] = string4;
    string_array[5] = string5;

    // for (int i = 0; i < no_entries; i++) {
    //     char string[100] = {0};
    //     string_array[i] = string;
    // }
    
    printf("\n\nlist returned %d\n", list(fd, "folder2/folder1/", string_array, &no_entries));
    printf("no_entries: %ld\n", no_entries);

    for (int i = 0; i < no_entries; i++) printf("%s\n", string_array[i]);

    if(no_entries == 0) {
        printf("list returned 0, no directory at the given path exists in the archive :D\n");
    } else {
        printf("list returned %ld, any other value otherwise\n", no_entries);
    }

    printf("\n");

    // for (int i = 0; i < no_entries; i++) free(string_array[i]);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    //test_check_archive();
    //test_exists();
    //test_is_dir();
    //test_read_file();
    test_list();

    return 0;
}