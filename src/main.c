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
server_response_handler(int fd, short what, void *arg);

void 
discover_server();

void connect_to_game();

struct event server_response_event;
struct sockaddr_in server_address;
int game_sock;
int main(int argc, char **argv)
{
	event_init();
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
		
	event_set(&server_response_event, client_sock, 
		EV_READ, server_response_handler, 
		&server_response_event);
		
	event_add(&server_response_event, NULL);
		
	SNAKE_DEBUG("Discovering server");
	sendto(client_sock, &reuse, sizeof(reuse), 0, 
		(struct sockaddr *)&client_addr, sizeof(client_addr)); 
	
}

void 
server_response_handler(int fd, short what, void *arg)
{
	struct sockaddr_in addr;
	NegotiationPacket *packet;
	struct hostent* host_info_ptr; 
    long host_address;
	SNAKE_DEBUG("Got response from server");
	
	
	packet = negotiation_packet_read(fd, &addr);
	SNAKE_DEBUG("Got response from server %s:%d", 
		packet->host_name, packet->port);
	
	host_info_ptr = gethostbyname(packet->host_name);
    memcpy(&host_address, host_info_ptr->h_addr, 
		host_info_ptr->h_length);
		
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(packet->port);
    
	free(packet);
	close(fd);	
	connect_to_game();
}


void connect_to_game()
{
	int reuse = 1;   
   
	SNAKE_DEBUG("Connecting to game");
    if ((game_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        SNAKE_ERROR("Failed to open socket");
        exit(EXIT_FAILURE);
    }
    
    
    if (connect(game_sock, (struct sockaddr *)&server_address,
		sizeof(struct sockaddr_in)) < 0)
    {
        SNAKE_ERROR("Can't connect");
        exit(EXIT_FAILURE);
    }
      
    SNAKE_DEBUG("Connected to server"); 
    setsockopt(game_sock, SOL_SOCKET, SO_REUSEADDR, &reuse,
		sizeof(reuse));
		  	
}
