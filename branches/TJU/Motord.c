#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "Motord.h"
#include "BottomLayer.h"
#include "SocketServer.h"

int main (void)
{
	int ret, i;
	struct motor_step step_now;

	int sockfd, server_id, sock_id = MOTORD_ID;

#ifdef HAS_MOTORS
	if (unlikely((motors = InitMotors ()) < 0))
	{
		perror ("oops: Motord");
		exit (-1);
	}
#endif

	sockfd = InitSocket (sock_id, "Motord", &server_id, REMOTE_ADDR);

#ifdef HAS_MOTORS
	ret = SendMotors (motors, step_now);
#endif

	while (1)
	{
		write (sockfd, &sock_id, sizeof (int));
		read (sockfd, &step_now, sizeof (struct motor_step));

#ifdef HAS_MOTORS
		if (unlikely(ret = SendMotors (motors, step_now))) {
#ifdef VERBOSE
			printf ("An error occured when sending data to motors. Delaying to recover...\n");
#endif
			usleep (20000);
		}
		usleep (10000);
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
