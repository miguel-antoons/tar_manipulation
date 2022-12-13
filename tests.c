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

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    // int ret = check_archive(fd);
    // printf("check_archive returned %d\n", ret);

    // int exist = exists(fd, "folder/tests.c");
    // printf("exists returned %d\n", exist);

    // int dir = is_dir(fd, "folder/");
    // printf("is_dir returned %d\n", dir);

    // create buffer to store the file
    // size_t size = 100;
    // uint8_t *buffer = malloc(size);

    // read_file(fd, "folder/tests.c", 0, buffer, &size);
    
    // // print buffer
    // for (int i = 0; i < size; i++){
    //     printf("%c", buffer[i]);
    // }
    // printf("\n");

    char *string_array[100] = {NULL};
    size_t no_entries = 1;
    printf("list returned %d\n", list(fd, "folder_sym", string_array, &no_entries));
    printf("no_entries: %ld\n", no_entries);

    for (int i = 0; i < no_entries; i++){
        printf("%s\n", string_array[i]);
    }
       

    return 0;
}