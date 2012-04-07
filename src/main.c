#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <event.h>
#include <snake-proto.h>
#include <snake-log.h>

#define SNAKE_PORT 5100 

void 
init_server_response_handler();

void 
server_response_handler(int fd, short what, void *arg);

void 
discover_server();

struct event server_response_event;
int server_port;
char *server_host;
int main(int argc, char **argv)
{
	event_init();
    init_server_response_handler();
    discover_server();
    event_dispatch();
    
    for(;;);
    
	return 0;
}

void discover_server()
{
	int client_sock; 
	struct sockaddr_in client_addr;
	int broadcast = 1;
	int reuse = 1;
	if ((client_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        SNAKE_ERROR("Failed to open socket");
        exit(EXIT_FAILURE);
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(SNAKE_PORT);
    
    setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &reuse,
		sizeof(reuse));
	setsockopt(client_sock, SOL_SOCKET, SO_BROADCAST, &broadcast,
		sizeof(broadcast));
		
	sendto(client_sock, &reuse, sizeof(reuse), 0, 
		(struct sockaddr *)&client_addr, sizeof(client_addr)); 
	close(client_sock);
}

void 
init_server_response_handler()
{
	int broadcast = 1;
	int reuse = 1;
    struct sockaddr_in server_response_addr;
    int server_response_sock;
   
    if ((server_response_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        SNAKE_ERROR("Failed to open socket");
        exit(EXIT_FAILURE);
    }
    memset(&server_response_addr, 0, sizeof(server_response_addr));
    server_response_addr.sin_family = AF_INET;
    server_response_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_response_addr.sin_port = htons(SNAKE_PORT + 1);
   
    if (bind(server_response_sock, (struct sockaddr *)&server_response_addr,
		sizeof(server_response_addr)) < 0)
    {
        SNAKE_ERROR("Can't bind");
        exit(EXIT_FAILURE);
    }
    event_set(&server_response_event, server_response_sock, 
		EV_READ, server_response_handler, 
		&server_response_event);
	event_add(&server_response_event, NULL);
}

void 
server_response_handler(int fd, short what, void *arg)
{
	int packet_size = 
		sizeof(NegotiationPacket) + SNAKE_HOST_MAX + 1;
	SNAKE_DEBUG("Got response from server");
	NegotiationPacket *packet = (NegotiationPacket*)malloc(packet_size);
	read(fd, packet, packet_size);
	
	server_host = (char*)malloc(packet->host_name_length + 1);
	negotiation_packet_parse(packet, &server_port,server_host);
	
	free(packet);
	
	close(fd);
}
