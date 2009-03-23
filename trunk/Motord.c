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

int main (int argc, char **argv)
{
	int ret, motors, i;
	struct timeval time_n, time_l;
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

	gettimeofday (&time_l, 0);

	while (1)
	{
		write (sockfd, &sock_id, sizeof (int));
		read (sockfd, &step_now, sizeof (struct motor_step));

		do
			gettimeofday (&time_n, 0);
		while (((time_n.tv_sec - time_l.tv_sec) * 1000 + (time_n.tv_usec - time_l.tv_usec) / 1000) < 20);

		time_l = time_n;

#ifdef HAS_MOTORS
		ret = SendMotors (motors, step_now);
#endif

#ifdef VERBOSE
		for (i = 0; i < 24; i++)
			printf ("%d,", step_now.onestep[i]);
		printf ("\n");
#endif
	}

	return 0;
}
