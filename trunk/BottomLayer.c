#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>

#include <linux/types.h>
#include <linux/videodev.h>

#include "BottomLayer.h"

struct video_mmap map;
struct video_mbuf mbuf;
int nextframe = 1;

int get_brightness_adj (unsigned char *image, long size, int *brightness)
{
	long i, tot = 0;

	for (i = 0; i < size * 3; i++)
		tot += image[i];
	*brightness = (128 - tot / (size * 3)) / 3;

	return !((tot / (size * 3)) >= 126 && (tot / (size * 3)) <= 130);
}

int InitVideo ()
{
	int file;
	struct video_capability cap;
	struct video_window win;
	struct video_picture pic;

	if ((file = open (VIDEO_DEV, O_RDWR)) < 0)
	{
		perror(VIDEO_DEV);
		return file;
	}

	if (ioctl (file, VIDIOCGCAP, &cap) < 0)
	{
		perror("VIDIOGCAP");
		fprintf(stderr, "(" VIDEO_DEV " not a video4linux device?)\n");
		return -1;
	}

	if (ioctl (file, VIDIOCGWIN, &win) < 0)
	{
		perror("VIDIOCGWIN");
		return -1;
	}
	win.width = CAPTURE_WIDTH;
	win.height = CAPTURE_HEIGHT;
	if (ioctl (file, VIDIOCSWIN, &win) < 0)
	{
		perror("VIDIOCSWIN");
		return -1;
	}

	if (ioctl (file, VIDIOCGPICT, &pic) < 0)
	{
		perror("VIDIOCGPICT");
		return -1;
	}
	pic.depth = CAPTURE_BPP;
	pic.palette = VIDEO_PALETTE_RGB24;
	if (ioctl (file, VIDIOCSPICT, &pic) < 0)
	{
		perror("VIDIOCSPICT");
		return -1;
	}

//	frame = malloc(CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);
//	if (!frame) {
//		fprintf(stderr, "Out of memory.\n");
//		exit(1);
//	}
	map.frame = 2;
	map.width = CAPTURE_WIDTH;
	map.height = CAPTURE_HEIGHT;
	map.format = VIDEO_PALETTE_RGB24;
	if (ioctl (file, VIDIOCMCAPTURE, &map) < 0)
	{
		perror ("VIDIOCMCAPTURE");
		return -1;
	}

	if (ioctl (file, VIDIOCGMBUF, &mbuf) < 0)
	{
		perror ("VIDIOCGMBUF");
		return -1;
	}

	if ((frame = (unsigned char *) mmap (0, mbuf.size,
					PROT_READ | PROT_WRITE, MAP_SHARED, file, 0)) < (unsigned char *) 0)
	{
		fprintf (stderr, "Can't open memory map.\n");
		return -1;
	}

	return file;
}

int RetrieveFrame (int video)
{
//	int f, newbright;

//	do {
//		read (video, frame, CAPTURE_WIDTH * CAPTURE_HEIGHT * CAPTURE_BPP);
//		f = get_brightness_adj (frame, CAPTURE_WIDTH * CAPTURE_HEIGHT, &newbright);
//		if (f) {
//			pic.brightness += (newbright << 8);
//			if (ioctl (file, VIDIOCSPICT, &video->pic) == -1)
//			{
//				perror("VIDIOSPICT");
//				break;
//			}
//		}
//	} while (f);
	
	nextframe ^= 1;

	if (ioctl (video, VIDIOCSYNC, &nextframe) < 0)
	{
		perror ("VIDIOCSYNC");
		return -1;
	}

	if (nextframe)
	{
		if (ioctl (video, VIDIOCMCAPTURE, &map) < 0)
		{
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

void *InitShared (char *tempfile)
{
	int shared_fd;
	void *map;

	if (tempfile)
		shared_fd = open (tempfile, O_CREAT|O_RDWR|O_TRUNC , 00777);
	printf ("Share file %s opened.\n", tempfile);

	lseek (shared_fd, CAPTURE_WIDTH * CAPTURE_HEIGHT * 3 - 1, SEEK_SET);
	write (shared_fd, "" , 1);

	map = mmap (NULL, CAPTURE_WIDTH * CAPTURE_HEIGHT * 3, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);

	close (shared_fd);

	return map;
}

void *OpenShared (char *tempfile)
{
	int shared_fd = open(tempfile, O_CREAT|O_RDWR, 00777);
	void *map;

	map = mmap (NULL, CAPTURE_WIDTH * CAPTURE_HEIGHT * 3, PROT_READ|PROT_WRITE, MAP_SHARED, shared_fd, 0);

	close (shared_fd);

	return map;
}

int CloseShared (void *map)
{
	munmap (map, mbuf.size);

	return 1;
}

int InitMotors ()
{
	int file;

	if ((file = open (MOTORS_DEV, O_RDWR)) < 0)
	{
		perror (MOTORS_DEV);
		return -1;
	}
	
	return file;
}

int SendMotors (int file, struct motor_step step)
{
	struct motor_response ret;

	if (ioctl (file, RM_EXEC_STEP, &step) < 0)
	{
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

	for (i = 0; i < 24; i++)
	{
		fscanf (file, "%d,", &motion[i]);
		ret.onestep[i] = (unsigned char) motion[i];
	}

	return ret;
}
