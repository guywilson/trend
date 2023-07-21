/******************************************************************************
Like truncate but keeps the end number of specified bytes.
Copyright (C) 2023  Guy Wilson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define READ_BUFFER_SIZE                    64

const char * pszWarranty = 
    "trend  Copyright (C) 2023  Guy Wilson" \
    "This program comes with ABSOLUTELY NO WARRANTY." \
    "This is free software, and you are welcome to redistribute it" \
    "under certain conditions.\n\n";

void printUsage(void) {
    printf("%s", pszWarranty);
    printf("Usage:\n");
    printf("\ttrend -s size[K][M][G] file\n\n");
}

int main(int argc, char ** argv) {
    int             i;
    int             fdi;
    int             fdo;
    int             rtn;
    off_t           newLength;
    size_t          multiplier;
    char *          pszSizeArg;
    char *          pszFilename;
    char            szTempfile[16];
    uint8_t         buffer[READ_BUFFER_SIZE];
    ssize_t         bytesRead;
    ssize_t         bytesWritten;

	if (argc == 4) {
		for (i = 1;i < argc - 1;i++) {
			if (argv[i][0] == '-') {
				if (argv[i][1] == 's') {
					pszSizeArg = strdup(&argv[++i][0]);
				}
				else {
					fprintf(stderr, "Unknown argument '%s'", &argv[i][0]);
					printUsage();
					return 0;
				}
			}
		}

        pszFilename = strdup(&argv[i][0]);
	}
	else {
		printUsage();
		return -1;
	}

    if (isalpha(pszSizeArg[strlen(pszSizeArg) - 1])) {
        switch (pszSizeArg[strlen(pszSizeArg) - 1]) {
            case 'K':
                multiplier = 1024;
                break;

            case 'M':
                multiplier = 1024 * 1024;
                break;

            case 'G':
                multiplier = 1024 * 1024 * 1024;
                break;

            default:
                multiplier = 1;
                break;
        }

        pszSizeArg[strlen(pszSizeArg) - 1] = 0;
    }
    else {
        multiplier = 1;
    }

    newLength = -strtoll(pszSizeArg, NULL, 10) * multiplier;

    fdi = open(pszFilename, O_RDONLY);

    lseek(fdi, newLength, SEEK_END);

    strncpy(szTempfile, "trendAZ_XXXXXX", 16);

    fdo = mkstemp(szTempfile);

    if (fdo < 0) {
        fprintf(stderr, "ERROR: Failed to open file %s:%s\n", szTempfile, strerror(errno));
        exit(-1);
    }

    do {
        bytesRead = read(fdi, buffer, READ_BUFFER_SIZE);
        bytesWritten = write(fdo, buffer, bytesRead);

        if (bytesWritten < bytesRead) {
            fprintf(
                    stderr, 
                    "ERROR: failed to write sufficient bytes to file %s:%s\n", 
                    szTempfile, 
                    strerror(errno));

            close(fdi);
            close(fdo);

            exit(-1);
        }
    }
    while (bytesRead > 0);

    if (bytesRead < 0) {
        fprintf(stderr, "ERROR: Failed to read file %s:%s\n", pszFilename, strerror(errno));
    }

    close(fdi);
    close(fdo);

    rtn = unlink(pszFilename);

    if (rtn < 0) {
        fprintf(stderr, "ERROR: Failed to unlink file %s:%s\n", pszFilename, strerror(errno));
        exit(-1);
    }

    rtn = rename(szTempfile, pszFilename);

    if (rtn < 0) {
        fprintf(stderr, "ERROR: Failed to rename file %s:%s\n", szTempfile, strerror(errno));
        exit(-1);
    }

    return 0;
}
