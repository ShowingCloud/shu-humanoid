#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/types.h>
#include <linux/videodev.h>
 
#include "BottomLayer.h"
#include "Decision.h"

struct video_mmap map;
struct video_mbuf mbuf;
int nextframe = 0;

int InitVideo ()
{
	int file;
	struct video_capability cap;
	struct video_window win;
	struct video_picture pic;

	if ((file = open (VIDEO_DEV, O_RDWR)) < 0) {
		perror(VIDEO_DEV);
		return file;
	}

	if (ioctl (file, VIDIOCGCAP, &cap) < 0) {
		perror("VIDIOGCAP");
		fprintf(stderr, "(" VIDEO_DEV " not a video4linux device?)\n");
		return -1;
	}

	if (ioctl (file, VIDIOCGWIN, &win) < 0) {
		perror("VIDIOCGWIN");
		return -1;
	}
	win.width = CAPTURE_WIDTH;
	win.height = CAPTURE_HEIGHT;
	if (ioctl (file, VIDIOCSWIN, &win) < 0) {
		perror("VIDIOCSWIN");
		return -1;
	}

	if (ioctl (file, VIDIOCGPICT, &pic) < 0) {
		perror("VIDIOCGPICT");
		return -1;
	}
	pic.depth = CAPTURE_BPP;
	pic.palette = VIDEO_PALETTE_RGB24;
	pic.brightness = 20000;
	if (ioctl (file, VIDIOCSPICT, &pic) < 0) {
		perror("VIDIOCSPICT");
		return -1;
	}

	map.frame = 2;
	map.width = CAPTURE_WIDTH;
	map.height = CAPTURE_HEIGHT;
	map.format = VIDEO_PALETTE_RGB24;
	if (ioctl (file, VIDIOCMCAPTURE, &map) < 0) {
		perror ("VIDIOCMCAPTURE");
		return -1;
	}

	if (ioctl (file, VIDIOCGMBUF, &mbuf) < 0) {
		perror ("VIDIOCGMBUF");
		return -1;
	}

	if ((frame = (unsigned char *) mmap (0, mbuf.size,
					PROT_READ | PROT_WRITE, MAP_SHARED, file, 0)) < (unsigned char *) 0) {
		fprintf (stderr, "Can't open memory map.\n");
		return -1;
	}

	while (ioctl (file, VIDIOCSYNC, &nextframe) < 0)
		usleep (1000);

	return file;
}

int RetrieveFrame (int video)
{
	nextframe ^= 1;

	if (ioctl (video, VIDIOCSYNC, &nextframe) < 0) {
		perror ("VIDIOCSYNC");
		return -1;
	}

	if (nextframe) {
		if (ioctl (video, VIDIOCMCAPTURE, &map) < 0) {
			perror ("VIDIOCMCAPTURE");
			return -1;
		}
	}

	return mbuf.offsets[nextframe];
}

int Closevideo (int video)
{
	munmap (frame, mbuf.size);
	close (video);
	return 1;
}

int InitMotors ()
{
	int file;

	if ((file = open (MOTORS_DEV, O_RDWR)) < 0) {
		perror (MOTORS_DEV);
		return -1;
	}
	
	return file;
}

inline int SendMotors (int file, struct motor_step step)
{
	struct motor_response ret;
	static struct timeval time_l = {0, 0};
	struct timeval time_n;

	gettimeofday (&time_n, 0);
	if ((time_n.tv_sec - time_l.tv_sec) < 1 && (time_n.tv_usec - time_l.tv_usec) < 15000)
		return -1;
	time_l = time_n;

	if (ioctl (file, RM_EXEC_STEP, &step) < 0) {
		perror ("RM_EXEC_STEP");
		return -1;
	}

	do
		read (file, &ret, sizeof (struct motor_response));
	while (ret.runmode != RM_EXEC_STEP);

	return ret.retcode;
}

struct motor_step ReadMotionFile (FILE *file)
{
	int motion[24];
	struct motor_step ret;
	int i;

	for (i = 0; i < 24; i++) {
		fscanf (file, "%d,", &motion[i]);
		ret.onestep[i] = (unsigned char) motion[i];
	}

	return ret;
}

int InitSocket (int sock_id, char addr[20], int server)
{
	int result, sockfd;
	struct sockaddr_in address;
	socklen_t len;

	sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	address.sin_family = AF_INET;
	if (server)
		address.sin_addr.s_addr = htonl (INADDR_ANY);
	else
		address.sin_addr.s_addr = inet_addr (addr);
	address.sin_port = htons (10200);
	len = sizeof (address);

	if (server) {
		bind (sockfd, (struct sockaddr *) &address, len);
		listen (sockfd, 5);
		return sockfd;
	}

	if ((result = connect (sockfd, (struct sockaddr *) &address, len)) == -1) {
		perror ("connect");
		exit (-1);
	}

	write (sockfd, &sock_id, sizeof (int));
	return sockfd;
}
