/* minigzip.c -- simulate gzip using the zlib compression library
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/*
 * minigzip is a minimal implementation of the gzip utility. This is
 * only an example of using zlib and isn't meant to replace the
 * full-featured gzip. No attempt is made to deal with file systems
 * limiting names to 14 or 8+3 characters, etc... Error checking is
 * very limited. So use minigzip only for testing; use gzip for the
 * real thing. On MSDOS, use only on file names without extension
 * or in pipe mode.
 */

/* $Id: minigzip.c,v 1.5 1995/05/03 17:27:11 jloup Exp $ */

#include <stdio.h>
#include "zlib.h"

#ifndef __GO32__
extern void exit  __P((int));
#endif
extern int unlink __P((const char *));

#ifdef STDC
#  include <string.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(WIN32)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define BUFLEN 4096
#define MAX_NAME_LEN 1024

#define local static
/* For MSDOS and other systems with limitation on stack size. For Unix,
    #define local
   works also.
 */

char *prog;

void error           __P((char *msg));
void gz_compress     __P((FILE   *in, gzFile out));
void gz_uncompress   __P((gzFile in, FILE   *out));
void file_compress   __P((char  *file));
void file_uncompress __P((char  *file));
void main            __P((int argc, char *argv[]));

/* ===========================================================================
 * Display error message and exit
 */
void error(msg)
    char *msg;
{
    fprintf(stderr, "%s: %s\n", prog, msg);
    exit(1);
}

/* ===========================================================================
 * Compress input to output then close both files.
 */
void gz_compress(in, out)
    FILE   *in;
    gzFile out;
{
    local char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = fread(buf, 1, sizeof(buf), in);
        if (ferror(in)) {
            perror("fread");
            exit(1);
        }
        if (len == 0) break;

        if (gzwrite(out, buf, len) != len) error(gzerror(out, &err));
    }
    fclose(in);
    if (gzclose(out) != Z_OK) error("failed gzclose");
}

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
void gz_uncompress(in, out)
    gzFile in;
    FILE   *out;
{
    local char buf[BUFLEN];
    int len;
    int err;

    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0) error (gzerror(in, &err));
        if (len == 0) break;

        if (fwrite(buf, 1, len, out) != (uInt)len) error("failed fwrite");
    }
    if (fclose(out)) error("failed fclose");

    if (gzclose(in) != Z_OK) error("failed gzclose");
}


/* ===========================================================================
 * Compress the given file: create a corresponding .gz file and remove the
 * original.
 */
void file_compress(file)
    char  *file;
{
    local char outfile[MAX_NAME_LEN];
    FILE  *in;
    gzFile out;

    strcpy(outfile, file);
    strcat(outfile, ".gz");

    in = fopen(file, "rb");
    if (in == NULL) {
        perror(file);
        exit(1);
    }
    out = gzopen(outfile, "wb");
    if (out == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, outfile);
        exit(1);
    }
    gz_compress(in, out);

    unlink(file);
}


/* ===========================================================================
 * Uncompress the given file and remove the original.
 */
void file_uncompress(file)
    char  *file;
{
    local char buf[MAX_NAME_LEN];
    char *infile, *outfile;
    FILE  *out;
    gzFile in;
    int len = strlen(file);

    strcpy(buf, file);

    if (len > 3 && strcmp(file+len-3, ".gz") == 0) {
        infile = file;
        outfile = buf;
        outfile[len-3] = '\0';
    } else {
        outfile = file;
        infile = buf;
        strcat(infile, ".gz");
    }
    in = gzopen(infile, "rb");
    if (in == NULL) {
        fprintf(stderr, "%s: can't gzopen %s\n", prog, infile);
        exit(1);
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        perror(file);
        exit(1);
    }

    gz_uncompress(in, out);

    unlink(infile);
}


/* ===========================================================================
 * Usage:  minigzip [-d] [files...]
 */

void main(argc, argv)
    int argc;
    char *argv[];
{
    int uncompr = 0;
    gzFile file;

    prog = argv[0];
    argc--, argv++;

    if (argc > 0) {
        uncompr = (strcmp(*argv, "-d") == 0);
        if (uncompr) {
            argc--, argv++;
        }
    }
    if (argc == 0) {
        SET_BINARY_MODE(stdin);
        SET_BINARY_MODE(stdout);
        if (uncompr) {
            file = gzdopen(fileno(stdin), "rb");
            if (file == NULL) error("can't gzdopen stdin");
            gz_uncompress(file, stdout);
        } else {
            file = gzdopen(fileno(stdout), "wb");
            if (file == NULL) error("can't gzdopen stdout");
            gz_compress(stdin, file);
        }
    } else {
        do {
            if (uncompr) {
                file_uncompress(*argv);
            } else {
                file_compress(*argv);
            }
        } while (argv++, --argc);
    }
    exit(0);
}
