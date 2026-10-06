// SPDX-License-Identifier: GPLv2
// Copyright: Steffen Kothe <steffen.kothe@skothe.net>  2026

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
#include <string.h>

#define HEXDUMP_CLONE "hexdump-clone"

/* CVS_GIT_VERSION is obtained from cmake project configuration */
#ifndef CVS_GIT_VERSION
#define CVS_GIT_VERSION "unset"
#endif

#define ASCII_PRINTABLE_START (uint8_t)0x20
#define ASCII_PRINTABLE_END (uint8_t)0x7e

#define BYTESPERLINE_DEFAULT (uint8_t)8
#define BYTESPERLINE_MAX 32
#define BYTESPERLINE_MIN 0
#define BYTESPERLINE_RANGE(bpl) \
	((bpl > BYTESPERLINE_MIN) && (bpl <= BYTESPERLINE_MAX))

void printByteLine(uint8_t *buf, uint8_t bytesperline, size_t size,
		   off_t offset)
{
	assert(offset >= 0);
	assert(bytesperline > 0);
	assert(buf != NULL);

	if (buf == NULL)
		return;

	if (size == 0)
		return;

	printf("%.*lx | ", bytesperline, offset);

	for (size_t i = 0; i < size; i++)
		printf("%.2x ", buf[i]);

	if (size < bytesperline)
		for (size_t i = size; i < bytesperline; i++)
			printf("   ");

	printf(" | ");

	for (size_t i = 0; i < size; i++)
		buf[i] >= ASCII_PRINTABLE_START &&buf[i] <=
				ASCII_PRINTABLE_END ?
			printf("%c", buf[i]) :
			printf(".");

	printf(" | \n");
}

ssize_t readBytesFromFileDesc(int fd, uint8_t *bytebuf, size_t bytes)
{
	size_t count = 0;

	assert(bytebuf != NULL);
	assert(bytes > 0);

	while (count < bytes) {
		ssize_t ret = read(fd, bytebuf + count, bytes - count);

		if (ret == 0)
			return (ssize_t)count;

		if (ret < 0)
			return -1;

		count += (size_t)ret;
	}

	return (ssize_t)count;
}

void printVersion()
{
	printf("git-%s\n", CVS_GIT_VERSION);
}

void printUsage()
{
	fprintf(stdout,
		"Usage: %s <flags> <path-to-file> or '-'(stdin)"
		"\n\t-v version"
		"\n\t-s <skip bytes> (> 0) "
		"\n\t-b <bytes per line> (1 to 32)"
		"\n\t-n <num of bytes> (> 0)\n\n",
		HEXDUMP_CLONE);
	printVersion();
}

int main(int argc, char *argv[])
{
	uint8_t bytesperline = BYTESPERLINE_DEFAULT;
	off_t offsetcnt = 0, skip = 0;
	ssize_t readlimit = 0, ret = 0;
	int fd, opt;
	uint8_t *bytebuf;
	unsigned long value;

	while ((opt = getopt(argc, argv, "vhn:b:s:")) != -1) {
		switch (opt) {
		case 'n':
			value = strtoul(optarg, NULL, 10);
			if (errno == EINVAL || errno == ERANGE) {
				fprintf(stderr, "Flag -n out of range\n");
				exit(EXIT_FAILURE);
			}
			readlimit = (ssize_t)value;
			break;
		case 'b':
			value = strtoul(optarg, NULL, 10);
			if (!BYTESPERLINE_RANGE(value) || errno == EINVAL ||
			    errno == ERANGE) {
				fprintf(stderr, "Flag -b out of range\n");
				exit(EXIT_FAILURE);
			}
			bytesperline = (uint8_t)value;
			break;
		case 's':
            value = strtoul(optarg, NULL, 10);
			if (errno == EINVAL || errno == ERANGE) {
				fprintf(stderr, "Flag -s out of range\n");
				exit(EXIT_FAILURE);
			}
			skip = (off_t)value;
			break;
		case 'h':
			printUsage();
			exit(EXIT_SUCCESS);
		case 'v':
			printVersion();
			exit(EXIT_SUCCESS);
		default:
			printUsage();
			exit(EXIT_FAILURE);
		}
	}

	if (argv[optind] == NULL) {
		fprintf(stderr, "Source for hexdump not selected!\n");
		printUsage();
		exit(EXIT_FAILURE);
	}

	if (argv[optind][0] != '-') {
		fd = open(argv[optind], O_RDONLY);
		if (fd == -1) {
			perror("Failed to open file");
			exit(EXIT_FAILURE);
		}
	} else
		fd = STDIN_FILENO;

	bytebuf = (uint8_t *)malloc(bytesperline * sizeof(uint8_t));
	if (bytebuf == NULL) {
		fprintf(stderr, "Allocation of byte buffer failed");
		exit(EXIT_FAILURE);
	}

	if (skip != 0) {
		if (-1 == lseek(fd, skip, SEEK_SET)) {
			fprintf(stderr, "%s : Skipping %ld bytes failed!\n",
				strerror(errno), skip);
			free(bytebuf);
			exit(EXIT_FAILURE);
		}
	}

	offsetcnt = skip;
	while (1) {
		ret = readBytesFromFileDesc(fd, bytebuf, bytesperline);

		if (ret <= 0)
			break;

		if (readlimit > 0) {
			readlimit -= ret;

			if (readlimit < 0) {
				printByteLine(bytebuf, bytesperline,
					      (size_t)(readlimit + ret),
					      offsetcnt);
				break;
			}
		}

		printByteLine(bytebuf, bytesperline, (size_t)ret, offsetcnt);
		offsetcnt += bytesperline;
	}

	free(bytebuf);
	bytebuf = NULL;

	if (fd != STDIN_FILENO) {
		if (close(fd)) {
			perror("Failed to close file\n");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
