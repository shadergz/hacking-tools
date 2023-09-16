#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

static const char PROGRAM_NAME[] = "ps";
static const char PROGRAM_VERSION[] = "0001";

static int getports(const char *strports, int *portlist)
{
    if (strstr(strports, ":")) {
        /* A port range has been specified */
        *portlist++ = (int)strtoul(strports, NULL, 10);
        *portlist = (int)strtoul(strstr(strports, ":") + 1, NULL, 10);
        return portlist[1];
    }
    *portlist = (int)strtoul(strports, NULL, 10);
    return 0;
}

static void validate(const int *portlist)
{
    for (int i = 0; i < 2; i++)
        if (portlist[i] < 0 || portlist[i] > 65535)
            exit(printf("Invalid port range (%d)\n", portlist[i]));
}

int main(int argc, char **argv, char **env)
{
    (void)env;

    int c;
    int portrange[2] = {0};

    while ((c = getopt(argc, argv, "p:vh")) != -1)
    switch (c) {
    case 'h':
        return printf("Usage of (%s): ./%s [OPTIONS] [IP ADDRESS]\n", PROGRAM_NAME, PROGRAM_NAME);
    case 'v':
        return printf("version %s\n", PROGRAM_VERSION);
    case 'p':
        if (!optarg)
            return puts("Port argument can't be null");
        getports(optarg, portrange);
        validate(portrange);
        if (portrange[1])
            printf("Port range: %d to %d\n", portrange[0], portrange[1]);
        else
            printf("Port for scan: %d\n", portrange[0]);
        break;
    }
    if (*portrange == 0)
        return puts("Specify a port number");
    if (portrange[1] == 0)
        portrange[1] = *portrange;

    char *destination = argv[argc - 1];
    if (strcmp(destination, "localhost") == 0)
        destination = "127.0.0.1";
    else if (strcmp(destination, "route") == 0)
        destination = "10.0.0.1";
    if (!strstr(destination, "."))
        return printf("Invalid destination (%s)\n", destination);
    printf("Destination address: %s\n", destination);

    struct sockaddr_in dest_sock;
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socket_fd == -1)
        return puts("Can't create the destination socket");
    
    puts("Destination socket created and opened");

    /* Setting static information */
    dest_sock.sin_family = AF_INET;
    memset(&dest_sock.sin_zero, 0, sizeof(dest_sock.sin_zero));

    if (inet_aton(destination, &dest_sock.sin_addr) == 0)
        return puts("Destination not setted (something is wrong!)");
    else
        puts("Destination configure succeed");

    for (int port = portrange[0]; port <= portrange[1]; port++)
    {
        /* Setting the port to scan */
        dest_sock.sin_port = htons(port);
        printf("Port %d [state]: %s\n", port, connect(socket_fd, (struct sockaddr*)&dest_sock, sizeof(dest_sock)) == 0 ? "\033[0;32mopened\033[0m" : "\033[0;31mclosed\033[0m");

    }

    close(socket_fd);
    puts("Destination socket closed");

    return 0;
}

