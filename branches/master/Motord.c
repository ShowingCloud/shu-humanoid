#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "Motord.h"
#include "BottomLayer.h"
#include "Decision.h"

int main (int argc, char **argv)
{
	int ret, motors, i, j, step_num, num;
	struct timespec ts;
	struct motor_step step_now, steps[200], step_head;
	struct timeval tv;
	int sockfd, sockfd_head;
	fd_set rfds, rfds_head;
	ssize_t nread;

#ifdef HAS_MOTORS
	if ((motors = InitMotors ()) < 0) {
		perror ("oops: Motord");
		exit (-1);
	}
#endif

	sockfd = InitSocket (MOTORD_ID, LOCAL_ADDR, 0);
	sockfd_head = InitSocket (MOTORD_HEAD_ID, LOCAL_ADDR, 0);

	step_now = step_init;
	memset (&step_head, 0x00, sizeof (struct motor_step));
#ifdef HAS_MOTORS
	ret = SendMotors (motors, step_now);
#endif
	write (sockfd, &num, sizeof (int));

	FD_ZERO (&rfds);
	FD_ZERO (&rfds_head);
	FD_SET (sockfd, &rfds);
	FD_SET (sockfd_head, &rfds_head);

	while (1) {
		while (ioctl (sockfd, FIONREAD, &nread) == 0 && nread == 0) {
			while (ioctl (sockfd_head, FIONREAD, &nread) == 0 && nread > 0) {
				read (sockfd_head, &step_head, sizeof (struct motor_step));
				write (sockfd_head, &num, sizeof (int));
			}
			step_now.onestep[20] = step_head.onestep[20];
			step_now.onestep[21] = step_head.onestep[21];
#ifdef HAS_MOTORS
			while ((ret = SendMotors (motors, step_now)) < 0);
#endif
		}

		read (sockfd, &step_num, sizeof (int));
		for (i = 0; i < step_num; i++)
			read (sockfd, &steps[i], sizeof (struct motor_step));
		write (sockfd, &num, sizeof (int));

		for (i = 0; i < step_num; i++) {
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
			while (select (1, &rfds_head, NULL, NULL, &tv) > 0) {
				read (sockfd_head, &step_head, sizeof (struct motor_step));
				write (sockfd_head, &num, sizeof (int));
			}
			steps[i].onestep[20] = step_head.onestep[20];
			steps[i].onestep[21] = step_head.onestep[21];
#ifdef HAS_MOTORS
			while ((ret = SendMotors (motors, steps[i])) < 0);
#endif
//			tv.tv_sec = 0;
//			tv.tv_usec = 11000;
//			select (0, NULL, NULL, NULL, &tv);

#ifdef VERBOSE
			for (j = 0; j < 24; j++)
				printf ("%d,", steps[i].onestep[j]);
			printf ("\n");
#endif
		}
		step_now = steps[i];
	}

	return 0;
}
