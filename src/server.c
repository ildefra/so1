#include <sys/socket.h>

#define LISTEN_PORT 2015

/* The server */
int main(int argc, char **argv) {
	int ds_sock;
	struct sockaddr_in my_addr;
	
	ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = LISTEN_PORT;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	bind(ds_sock, &my_addr, sizeof(my_addr));
	
	//do something
	
	close(ds_sock);
}
