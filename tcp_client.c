/* tcp_client.c */

/* we begin our tcp client by including the header file, chap03.h */
#include "chap03.h"

/* on windows, we also need the conio.h header. This is required for the _kbhit() function */
#if defined(_WIN32)
#include <conio.h>
#endif

/* we can then begin the main() and initialize Winsock */
int main(int argc, char *argv[])
{
#if defined(_WIN32)
	WSADATA d;
	if (WSAstatup(MAKEWORD(2, 2), &d))
	{
		fprintf(stderr, "Failed to initialize.\n");
		return -1;
	}
#endif

/* We would like our program to take the hostname and port number of the server it should connect to as command-line arguments. 
 * This makes our program flexible. We have our program check that thes command-line arguments are given. If they aren't, it
 * displays usage information */
	if (argc < 3)	
	{
		fprintf(stderr, "usage: tcp_client hostname port\n");
		return -1;
	}

	/* We then use values to configure a remote address for connection */
	printf("Configuring remote address...\n");
	struct addrinfo hints;

	memset(&hints, 0,sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *peer_address;
	if (getaddrinfo(argv[1], argv[2], &hints, &peer_address))
	{
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return -1;
	}

	/* After getaddrinf() configures the remote address, we print it out. This isn't really necessary, butis a good
	 * debugging measure. We use getnameinfo() to convert the address back into a string, like this */
	printf("Remote address is: ");
	char address_buffer[100];
	char service_buffer[100];

	getnameinfo(peer_address -> ai_addr, peer_address -> ai_addrlen, address_buffer, sizeof(address_buffer),
									 service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	printf("%s %s\n", address_buffer, service_buffer);

	/* We can then create our socket */
	printf("Creatin socket...\n");
	SOCKET socket_peer;

	/* We use peer_address to set the proper socke family and protocol. This keeps our program very flexible, as the 
	 * socket() call createsan ipv4 or ipv6 socket needed */
	socket_peer = socket(peer_address -> ai_family, peer_address -> ai_socktype, peer_address -> ai_protocol);
	if (!ISVALIDSOCKET(socket_peer))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	/* After the socket has been created, we call connect() to establish a connection to the remote server */
	printf("Connecting...\n");
	if (connect(socket_peer, peer_address -> ai_addr, peer_address -> ai_addrlen))
	{
		fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(peer_address);

	/* if we've made it this far, then a TCP connection has been established to the remote server. We let the user
	 * know by printing a message and instrucctions on how to send data */
	printf("Connected.\n");
	printf("To send data, enter text followed by enter.\n");

	/* Our program should now loop while checking both the terminal and socket for new data.
	 * We begin our loop and set up the call to select(), like this */
	while(1)
	{
		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
#if !defined(_WIN32)
		FD_SET(0, &reads);
#endif

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
		{
			fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
			return -1;
		}
		/* After select() returns, we check to see whether our socket is set in read. If it is, then we know to call recv()
		 * to read the new data. The new data is printed to the console with printf() */
		if (FD_ISSET(socket_peer, &reads))
		{
			char read[4096];
			int bytes_received = recv(socket_peer, read, 4096, 0);
			if (bytes_received < 1)
			{
				printf("Connection closed by peer.\n");
				break;
			}
			printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
		}
		/* After checking for new TCP data, we also need to check for terminal input */
#if defined(_WIN32)
		if (_kbhit())
		{
#else
			if (FD_ISSET(0, &reads))
			{
#endif
				char read[4096];
				if (!fgets(read, 4096, stdin)) break;

				printf("Sending: %s", read);
				int bytes_sent = send(socket_peer, read, strlen(read), 0);
				printf("Sent %d bytes.\n", bytes_sent);
			}

		}
/* at this pont, our program is essentially done. We can end the while loop, close our socket, and
			 * clean up Winsock */
			printf("Closing socket...\n");
			CLOSESOCKET(socket_peer);
#if defined(_WIN32)
			WSCleanup();
#endif
			printf("Finished.\n");

			return 0;
	}
	
		



