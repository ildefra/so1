#include <sys/socket.h>

int main(int argc, char **argv) {
	int ds_sock;
	ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	return ds_sock == -1 ? 1 : 0;
}
