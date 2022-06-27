#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>

#include <ctype.h>

int main(int argc, char *argv[])
{
    if (!argv[1])
        return puts("A file is need");
    
    char *filename = argv[1];

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return printf("Can't open the file %s because of %s\n", filename, strerror(errno));

    unsigned char filebuffer[1024 * 8]; /* 8 Kibibyte */
    for (; ;) {
        unsigned char v;
        unsigned char *fb_array = filebuffer;
        const int r_ret = read(fd, filebuffer, sizeof(filebuffer));
        if (r_ret == -1 || r_ret == 0)
            break;
        for (int i = 0; i < r_ret; i++)
            if ((*(fb_array+i)) == '\0' && isprint(fb_array[i - 1]) && isprint(fb_array[i - 2]))
                puts("");
            else if (isprint((v = *(fb_array+i))))
                printf("%c", v);
    }
    close(fd);
    puts("");
}