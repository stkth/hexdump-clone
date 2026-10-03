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
#include <assert.h>

#define ASCII_PRINT_START   (uint8_t)0x1f
#define ASCII_PRINT_END     (uint8_t)0x7f

#define HEXDUMP_CLONE       "hexdump-clone"

uint8_t bytesperline = 8;

void printSingleString(uint8_t* buf, size_t size, off_t offset)
{
    assert(offset >= 0);

    if (buf == NULL)
        return;

    if(size == 0)
        return ;

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

ssize_t readBytesFromFileDesc(int fd, uint8_t *bytebuf, size_t bytes)
{
    ssize_t readret;
    size_t count;

    assert(bytebuf != NULL);
    assert(bytes > 0);

    count = 0;
    while(1)
    {
        readret = read(fd, bytebuf+count, bytes-count);
        if(readret == 0)
        {
            if (count != 0)
                return (ssize_t)(bytes - count);

            return 0;
        }

        if(readret < 0)
            return readret;

        if (bytes == (size_t)readret)
            return readret;

        count = bytes - (size_t)readret;
   }
}

void printUsage()
{
    fprintf(stdout, "Usage %s"
            "\n\t-s <skip bytes>"
            "\n\t-b <bytes per line>"
            "\n\t-n <num of bytes>"
            "\n\t<path-to-file> or '-' to read from stdin\n", HEXDUMP_CLONE);
}

int main(int argc, char *argv[])
{
    int fd, opt;
    uint8_t *bytebuf;
    off_t offsetcnt = 0;
    off_t skip = 0;
    ssize_t readlimit = 0;
    ssize_t ret = 0;

    while ((opt = getopt(argc, argv, "hn:b:s:")) != -1)
    {
        switch(opt) {
            case 'n':
                readlimit=strtol(optarg, NULL, 10);
                if (readlimit < 0)
                    exit(EXIT_FAILURE);
                break;
            case 'b':
                bytesperline=(uint8_t)strtoul(optarg, NULL, 10);
                if(bytesperline <= 0)
                    exit(EXIT_FAILURE);
                break;
            case 's':
                skip=(off_t)strtoul(optarg, NULL, 10);
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
        fprintf(stderr, "Source for dump not selected!\n");
        printUsage();
        exit(EXIT_FAILURE);
    }

    if(argv[optind][0] != '-')
    {
        fd = open(argv[optind], O_RDONLY);
        if (fd == -1)
        {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }
    }
    else
        fd = STDIN_FILENO;

    bytebuf = (uint8_t*)malloc(bytesperline * sizeof(uint8_t));
    if(bytebuf == NULL)
    {
        fprintf(stderr, "Allocating Line Byte Buffer Failed");
        exit(EXIT_FAILURE);
    }

    if(skip != 0)
    {
        if(-1 == lseek(fd, skip, SEEK_SET))
        {
            fprintf(stderr, "Skipping %ld bytes failed!\n", skip);
            exit(EXIT_FAILURE);
        }
    }

    offsetcnt = skip;
    while(1)
    {
        ret = readBytesFromFileDesc(fd, bytebuf, bytesperline);

        if(ret <= 0)
            break;

        if(readlimit > 0)
        {
            readlimit -= ret;

            if (readlimit < 0)
            {
                printSingleString(bytebuf, (size_t)(readlimit + ret), offsetcnt);
                break;
            }
        }

        printSingleString(bytebuf, (size_t)ret, offsetcnt);
        offsetcnt += bytesperline;
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
