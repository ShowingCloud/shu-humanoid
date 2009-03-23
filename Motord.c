#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Motord.h"
#include "BottomLayer.h"
#include "SocketServer.h"

int main (int argc, char **argv)
{
	int ret, motors, i;
	struct motor_step step_now;

	int sockfd, sockresult, sock_id = MOTORD_ID, server_id;
	socklen_t len;
	struct sockaddr_in address;

#ifdef HAS_MOTORS
	if ((motors = InitMotors ()) < 0)
	{
		perror ("oops: Motord");
		exit (-1);
	}
#endif

	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr ("127.0.0.1");
	address.sin_port = htons(10200);
	len = sizeof (address);

	if ((sockresult = connect (sockfd, (struct sockaddr *) &address, len)) == -1)
	{
		perror ("oops: Visiond");
		exit(-1);
	}

	write (sockfd, &sock_id, sizeof (int));
	read (sockfd, &server_id, sizeof (int));
	if ((server_id & ID_MASK) != SOCKET_LISTENER_ID)
	{
		printf ("Error: unknown socket server!\n");
		exit(-1);
	}
	else
		printf ("Connected with the socket server!\n");

	step_now = step_init;
#ifdef HAS_MOTORS
	ret = SendMotors (motors, step_now);
#endif

	while (1)
	{
		write (sockfd, &sock_id, sizeof (int));
		read (sockfd, &step_now, sizeof (struct motor_step));

#ifdef HAS_MOTORS
		ret = SendMotors (motors, step_now);
#else
		usleep (20000);
#endif

#ifdef VERBOSE
		for (i = 0; i < 24; i++)
			printf ("%d,", step_now.onestep[i]);
		printf ("\n");
#endif
	}

	return 0;
}
