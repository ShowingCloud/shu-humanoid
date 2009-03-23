#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "MyVision.h"

int main (int argc, char **argv)
{
	int motors = InitMotors ();
	FILE *file;
	struct motor_step step;
	file = fopen ("walk12pace.txt", "r");
	while (!feof (file))
	{
		step = ReadMotionFile (file);
		SendMotors (motors, step);
	}

	return 0;
}
