#include <stdio.h>
#include <string.h>

#include <getopt.h>

#include <stdlib.h>
#include <ctype.h>

#include <sys/select.h>

#include <unistd.h>

static const char PROGRAM_NAME[] = "obfuscator";
static const char PROGRAM_VERSION[] = "0.1";

size_t strnobf(char *buffer, size_t bfsize, const char *strhand, size_t len)
{
    size_t bytesproc = 0;

    for (; *strhand != '\0' && bfsize > 0; bfsize--)
    {
        char value = *strhand++;
        char result = value ^ len;
        *buffer++ = result;
        bytesproc++;
    }

    return bytesproc;
}

int main(int argc, char **argv)
{
    char c = 0;

    while ((c = getopt(argc, argv, "hvedk:")) != -1)
    switch (c)
    {
    case 'h': return printf("usage of %s: (%s): -d | -e TEXT\n", PROGRAM_NAME, PROGRAM_VERSION);
    case 'v': return printf("version %s\n", PROGRAM_VERSION);
    }

    fd_set set;
    FD_ZERO(&set);
    FD_SET(fileno(stdin), &set);
    
    select(fileno(stdin) + 1, &set, NULL, NULL, NULL);

    char strhand[300];
    memset(strhand, 0, sizeof(strhand));

    read(fileno(stdin), strhand, sizeof(strhand));

    /* Reading and storing in the same buffer, this maybe is insecure,
     * you can avoid this as well!
    */ 
    strnobf(strhand, sizeof(strhand), strhand, strlen(strhand));

    printf("%s", strhand);

    exit(0);
}
