#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

#define LISTEN_PORT 2015

/* XXX: this is already too long */
/* The server */
int main(int argc, char **argv) {
	int ds_sock, ds_sock_acc;
	struct sockaddr_in my_addr;
    struct sockaddr addr;
	int addrlen;
    
	ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = LISTEN_PORT;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	bind(ds_sock, (struct sockaddr *) &my_addr, sizeof(my_addr));
	
    ds_sock_acc = accept(ds_sock, &addr, &addrlen);
    
	//do something
	
	close(ds_sock); /* only use of unistd.h */
    close(ds_sock_acc);
    return EXIT_SUCCESS;    /* only use of stdlib.h */
}
