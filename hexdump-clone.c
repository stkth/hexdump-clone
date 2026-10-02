// SPDX-License-Identifier: GPLv2

/* Self-Exercise:
 * Implement a hexdump or simple hd clone while focusing on read, open, close
 * syscalls, some formatting, basic code seperation and proper error handling.
 *
 * Finanlize the refresher with small -n option for count of bytes to be read.
 * Add a skip option which uses lseek to manipulate the file offset
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>

#define ASCII_PRINT_START   (uint8_t)0x1f
#define ASCII_PRINT_END     (uint8_t)0x7f

#define HEXDUMP_CLONE       (char*) "hexdump-clone"

uint8_t bytesperline = 8;

void printSingleString(uint8_t* buf, size_t size, off_t offset)
{
    if (buf == NULL)
        return;

    printf("%.*lx | ", bytesperline, offset);

    for(size_t i = 0; i < size; i++)
        printf("%.2x ", buf[i]);

    if (size < bytesperline)
        for(size_t i=size; i < bytesperline; i++)
            printf("   ");

    printf(" | ");

    for(size_t i = 0; i < size; i++)
        buf[i] > ASCII_PRINT_START && buf[i] < ASCII_PRINT_END ?
            printf("%c", buf[i]):
            printf(".");

    printf(" | \n");
}

void printUsage()
{
    fprintf(stdout, "Usage %s"
            "\n\t-s <skip bytes>"
            "\n\t-b <bytes per line>"
            "\n\t-n <num of bytes>"
            "\n\t<path-to-file>\n", HEXDUMP_CLONE);
}

int main(int argc, char *argv[])
{
    int fd, opt;
    uint8_t *bytebuf;
    ssize_t readret;
    off_t offsetcnt = 0;
    off_t skip = 0;
    size_t readlimit = 0;
    ssize_t readbytes = 0;
    size_t toread;

    if (argc < 2)
    {
        fprintf(stderr, "Missing file for dump\n");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "hn:b:s:")) != -1)
    {
        switch(opt) {
            case 'n':
                readlimit=strtoul(optarg, NULL, 10);
                break;
            case 'b':
                bytesperline=(uint8_t)strtoul(optarg, NULL, 10);
                break;
            case 's':
                skip=(uint32_t)strtoul(optarg, NULL, 10);
                break;
            case 'h':
                printUsage();
                exit(EXIT_SUCCESS);
            default:
                printUsage();
                exit(EXIT_FAILURE);
        }
    }

    if (argv[optind] == NULL)
    {
        fprintf(stderr, "Missing file path!\n");
        printUsage();
        exit(EXIT_FAILURE);
    }

    fd = open(argv[optind], O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    toread = bytesperline * sizeof(uint8_t);
    bytebuf = (uint8_t*)malloc(toread);
    if(bytebuf == NULL)
    {
        fprintf(stderr, "Allocating Line Byte Buffer Failed");
        exit(EXIT_FAILURE);
    }

    readbytes = (ssize_t)readlimit;

    if(-1 != lseek(fd, skip, SEEK_SET))
    {
        fprintf(stderr, "Skipping %ld bytes failed!\n", skip);
        exit(EXIT_FAILURE);
    }

    offsetcnt = skip;

    while(0 != (readret = read(fd, bytebuf, toread)))
    {
        if(readlimit != 0)
        {
            readbytes -= readret;

            if (readbytes < 0)
            {
                printSingleString(bytebuf, (size_t)(readbytes + readret), offsetcnt);
                readret=0;
                break;
            }
        }

        printSingleString(bytebuf, toread, offsetcnt);
        offsetcnt += bytesperline;
    }

    if(readret != 0)
    {
        perror("Error occured during read of file");
        exit(EXIT_FAILURE);
    }

    free(bytebuf);
    bytebuf = NULL;

    if (close(fd))
    {
        perror("Failed to close file\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
