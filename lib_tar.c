#include "lib_tar.h"

#define NEXT_HEADER_EXISTS(fd, header) (sizeof(tar_header_t) == read(fd, &header, sizeof(header)) && header.name[0] != '\0')

/**
 * DEBUG 1: explanation of checksum
 * The chksum field represents the simple sum of all bytes in the header block.
 * Each 8-bit byte in the header is added to an unsigned integer, initialized to
 * zero, the precision of which shall be no less than seventeen bits. When
 * calculating the checksum, the chksum field is treated as if it were all blanks.
*/

/**
 * DEBUG 2: explanation of size 
 * The size field is the size of the file in bytes; linked files are archived with
 * this field specified as zero.
 * ? maybe useful with read function?
*/

/**
 * DEBUG 3: null terminated strings
 * string that ends with null character '\0'
*/

/**
 * DEBUG 4: fil descriptors
 * https://stackoverflow.com/questions/5256599/what-are-file-descriptors-explained-in-simple-terms
*/

/** 
 * DEBUG 5: lseek()
 * https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-lseek-change-offset-file
*/

/** 
 * DEBUG 6: tar attribute number base
 * The name, linkname, magic, uname, and gname are null-terminated character strings.
 * All other fields are zero-filled octal numbers in ASCII. Each numeric field of width
 * w contains w minus 1 digits, and a null. (In the extended GNU format, the numeric
 * fields can take other forms.)
*/

/** 
 * TODO : docstring of this function
*/
void goto_next_header(int tar_fd, tar_header_t *tar_header) {
    uint64_t size   = strtoll(tar_header->size, NULL, 8);
    size            += (BLOCKSIZE - (size % BLOCKSIZE)) % BLOCKSIZE;
    lseek(tar_fd, (off_t) size, SEEK_CUR);
}

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    tar_header_t header;
    int n_headers = 0;
    
    while (NEXT_HEADER_EXISTS(tar_fd, header)) {
        if (strncmp(header.magic,   TMAGIC,     TMAGLEN) != 0)  return -1;      // magic check
        if (strncmp(header.version, TVERSION,   TVERSLEN) != 0) return -2;      // version check

        uint64_t sum                = 0;
        uint64_t expected_chksum    = strtoll(header.chksum, NULL, 8);
        
        int i;
        for (i = 0; i < sizeof(header); i++)        sum += ((uint8_t *) &header)[i];   // checksum
        for (i = 0; i < sizeof(header.chksum); i++) sum -= ((uint8_t) header.chksum[i] - (uint8_t) ' ');

        if (sum != expected_chksum) return -3;
        n_headers++;                                                  // number of headers

        goto_next_header(tar_fd, &header);
    }

    return n_headers;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    tar_header_t header;

    while (NEXT_HEADER_EXISTS(tar_fd, header)) {
        if (strcmp(header.name, path) == 0) return 1;

        goto_next_header(tar_fd, &header);
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    tar_header_t header;

    while (NEXT_HEADER_EXISTS(tar_fd, header)) {
        if (strcmp(header.name, path) == 0 && header.typeflag == DIRTYPE) return 1;

        goto_next_header(tar_fd, &header);
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    tar_header_t header;

    while (NEXT_HEADER_EXISTS(tar_fd, header)) {
        if(strcmp(header.name, path) == 0 && (header.typeflag == REGTYPE || header.typeflag == AREGTYPE)) return 1;

        goto_next_header(tar_fd, &header);
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    tar_header_t header;

    while (NEXT_HEADER_EXISTS(tar_fd, header)) {
        if(strcmp(header.name, path) == 0 && header.typeflag == SYMTYPE) return 1;

        goto_next_header(tar_fd, &header);
    }
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    tar_header_t header;
    const size_t expected_no_entries = *no_entries;
    *no_entries = 0;
    size_t path_len     = strlen(path);
    int return_value    = 0;

    while (NEXT_HEADER_EXISTS(tar_fd, header) && *no_entries < expected_no_entries) {
        if (strcmp(header.name, path) == 0 && header.typeflag == SYMTYPE) {
            *no_entries = expected_no_entries;
            return list(tar_fd, strcat(header.linkname, "/"), entries, no_entries);
        }

        if (strncmp(header.name, path, path_len) == 0) {
            return_value = 1;
            if (strlen(header.name) > path_len + 1) {
                entries[*no_entries] = strdup(header.name);
                (*no_entries)++;
            } else if (header.typeflag != DIRTYPE) {
                return 0;
            }
        }
        // printf("header.name: %s\n", header.name);
        // printf("header.linkname: %s\n", header.linkname);
        goto_next_header(tar_fd, &header);
    }

    return return_value;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    tar_header_t header;
    if(!is_file(tar_fd, path)) return -1;
    lseek(tar_fd, 0, SEEK_SET);

    while (NEXT_HEADER_EXISTS(tar_fd, header)) {
        if (strcmp(header.name, path) == 0 && header.typeflag == SYMTYPE) return read_file(tar_fd, header.linkname, offset, dest, len);

        if(strcmp(header.name, path) == 0) {
            uint64_t size = strtoll(header.size, NULL, 8);
            if(offset > size) return -2;

            // get size of file content
            size += (BLOCKSIZE - (size % BLOCKSIZE)) % BLOCKSIZE;

            // read maximum possible
            if(*len >= size - offset) *len = size - offset; 

            lseek(tar_fd, (off_t) offset, SEEK_CUR);
            read(tar_fd, dest, *len);

            if(size - offset - *len == 0)   return 0;
            else                            return size - offset - *len;
        }

        goto_next_header(tar_fd, &header);
    }
    return 0;
}