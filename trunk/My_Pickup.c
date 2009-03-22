/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Picking up the colors from a window displaying the frame from camera by clicking. Click on 
 * the window, and that point will become black, and its HSV values are recorded. Then press one
 * key to indicate which color the points you clicked is to be, and their HSV values will be
 * saved to a file called "colordatafile.txt", which can be read by the scaning program.
 */

#include "MyVision.h"

struct Points points; /* record the coordinates of all points clicked */
struct HSVColors colors; /* record the HSV values of points clicked in one kind of color */
IplImage* frame; /* one frame */
FILE* fp; /* file to write HSV values in */


int RecordColor()
{
	/*
	 * Save the HSV values of one color to the file.
	 * The file will look like this:
	 * 1,red
	 * xxx,xxx,xxx
	 * xxx,xxx,xxx
	 * 3,blue
	 * xxx,xxx,xxx
	 * ...
	 */
	int i;

	fprintf(fp, "%d,%s\n", colors.name, color_name[colors.name]);

	for(i = 0; i < colors.num; i++)
	{
		fprintf(fp, "%d,%d,%d\n", colors.HSVColor[i].H, colors.HSVColor[i].S, colors.HSVColor[i].V);
	}

	return 0;
}


void on_mouse(int event, int x, int y, int flags, void* param)
{
	/*
	 * When left button is pressed down, record the color of this point,
	 * and convert it from RGB to HSV, then save it both in
	 * structure colors, which saves the HSV values of this kind of color,
	 * and structure points, which saves the HSV values of all points.
	 * At most 9 points are allowed to form one kind of color.
	 */
	if(event == CV_EVENT_LBUTTONDOWN && colors.num <= 9)
	{
		printf("%d, %d, %d, %d\n", event, x, y, flags); /* for debugging */

		points.point[points.num].x = x;
		points.point[points.num].y = y;
		points.point[points.num].color = COLOR_BLACK; /* display black by default */
		points.num++;

		int p = (y * frame->width + x) * 3; /* for each point, three integers (B, G, R) are saved */

		colors.HSVColor[colors.num] = RGB2HSV((unsigned char)frame->imageData[p + 2],
				(unsigned char)frame->imageData[p + 1], (unsigned char)frame->imageData[p]);
		printf("%d, %d, %d\n", colors.HSVColor[colors.num].H,
				colors.HSVColor[colors.num].S, colors.HSVColor[colors.num].V); /* for debugging */
		colors.num++;
	}
}

struct HSVColor RGB2HSV(int R, int G, int B)
{
	/*
	 * Convert RGB color to HSV.
	 * FIXME: This algorithm may be lossy, when S is cut short,
	 * and therefore makes H lossy.
	 * TODO: This algorithm may or may not be efficient, which should be
	 * verified in thoroughly examination.
	 */
	int H, S, V;

	if (R > G)
	{
		if (B > R) /* max = B; min = R; */
		{
			V = B;
			S = (B - R) * 255 / B;
			H = 120 + (R - G) * 30 / S;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G > B) /* max = R; min = B; */
		{
			V = R;
			S = (R - B) * 255 / R;
			H = (G - B) * 30 / S;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else /* max = R; min = G; */
		{
			V = R;
			S = (R - G) * 255 / R;
			H = 180 + (G - B) * 30 / S;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
	}
	else
	{
		if (B < R) /* max = G; min = B; */
		{
			V = G;
			S = (G - B) * 255 / G;
			H = 90 + (B - R) * 30 / S;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G < B) /* max = B; min = R; */
		{
			V = B;
			S = (B - R) * 255 / B;
			H = 120 + (R - G) * 30 / S;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else /* max = G; min = R; */
		{
			V = G;
			S = (G - R) * 255 / G;
			H = 90 + (B - R) * 30 / S;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
	}

	/*
	 * Another algorithm using float variables.
	 * Rf = R / 255; Gf = G / 255; Bf = B / 255;
	 * This is lossless, but may not be as efficient as the former one.
	 * TODO: thoroughly examination
	 */
#if 0
	Vf = (Rf > Gf) ? ((Bf > Rf) ? Bf : Rf) : ((Gf > Bf) ? Gf : Bf);
	Sf = (Vf - ((Rf < Gf) ? ((Bf < Rf) ? Bf : Rf) : ((Gf < Bf) ? Gf : Bf))) / Vf;
	if (Vf == Rf) H = (Gf - Bf) * 60 / Sf;
	if (Vf == Gf) H = 180 + (Bf - Rf) * 60 / Sf;
	if (Vf == Bf) H = 240 + (Rf - Gf) * 60 / Sf;
	if (H < 0) H += 360;
	V = Vf * 255;
	S = Sf * 255;
	H /= 2;
#endif

	/*
	 * The algorithm used in opencv, converting a matrix from RGB to HSV.
	 * TODO: thoroughly examination
	 */
#if 0
static CvStatus CV_STDCALL
icvBGRx2HSV_32f_CnC3R( const float* src, int srcstep,
                       float* dst, int dststep,
                       CvSize size, int src_cn, int blue_idx )
{
    int i;
    srcstep /= sizeof(src[0]);
    dststep /= sizeof(dst[0]);
    srcstep -= size.width*src_cn;
    size.width *= 3;

    for( ; size.height--; src += srcstep, dst += dststep )
    {
        for( i = 0; i < size.width; i += 3, src += src_cn )
        {
            float b = src[blue_idx], g = src[1], r = src[2^blue_idx];
            float h, s, v;

            float vmin, diff;

            v = vmin = r;
            if( v < g ) v = g;
            if( v < b ) v = b;
            if( vmin > g ) vmin = g;
            if( vmin > b ) vmin = b;

            diff = v - vmin;
            s = diff/(float)(fabs(v) + FLT_EPSILON);
            diff = (float)(60./(diff + FLT_EPSILON));
            if( v == r )
                h = (g - b)*diff;
            else if( v == g )
                h = (b - r)*diff + 120.f;
            else
                h = (r - g)*diff + 240.f;

            if( h < 0 ) h += 360.f;

            dst[i] = h;
            dst[i+1] = s;
            dst[i+2] = v;
        }
    }

    return CV_OK;
}
#endif

}

int main(int argc, char** argv)
{
	CvCapture* capture;
	int i, j, p[9];

	/* capture from camera */
	capture = cvCaptureFromCAM(-1);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	/* open a new window, and catch mouse events to it */
	cvNamedWindow("MyVision_PickingUp", 0);
	cvSetMouseCallback("MyVision_PickingUp", on_mouse, 0);

	/* open the file for writing */
	if((fp = fopen(filename, "w")) == NULL){
		printf("cannot open file \n");
		return -1;
	}

	/* initial structure colors and points */
	colors.num = 0;
	points.num = 0;
 
	for(;;)
	{
		/* query a frame */
		frame = cvQueryFrame(capture);
		if(!frame)
			break;

		/*
		 * for points already recorded, mark them and their surrounding pixels
		 * (9 in all) with their color.
		 */
		for (i = 0; i < points.num; i++)
		{
			p[0] = (points.point[i].y * frame->width + points.point[i].x) * 3;
			p[1] = p[0] - 3; /* left pixel */
			p[2] = p[0] + 3; /* right pixel */
			for (j = 0; j < 3; j++)
			{
				p[3 + j] = p[j] - frame->width * 3; /* the line above */
				p[6 + j] = p[j] + frame->width * 3; /* the line below */
			}
			for (j = 0; j < 9; j++)
			{
				frame->imageData[p[j]] = color_value_rgb[points.point[i].color][0];
				frame->imageData[p[j] + 1] = color_value_rgb[points.point[i].color][1];
				frame->imageData[p[j] + 2] = color_value_rgb[points.point[i].color][2];
			}
		}

		/* show this modified frame */
		cvShowImage("MyVision_PickingUp", frame);

		switch((unsigned char)cvWaitKey(10))
		{
			case 'r': /* mark the former points as red, and save them */
				colors.name = COLOR_RED;
				while(RecordColor());
				for (i = 0; i < colors.num; i++)
					points.point[points.num - i - 1].color = COLOR_RED;
				colors.num = 0;
				break;
			case 'g': /* mark the former points as green, and save them */
				colors.name = COLOR_GREEN;
				while(RecordColor());
				for (i = 0; i < colors.num; i++)
					points.point[points.num - i - 1].color = COLOR_GREEN;
				colors.num = 0;
				break;
			case 'b': /* mark the former points as blue, and save them */
				colors.name = COLOR_BLUE;
				while(RecordColor());
				for (i = 0; i < colors.num; i++)
					points.point[points.num - i - 1].color = COLOR_BLUE;
				colors.num = 0;
				break;
			case 'c': /* delete all points of the former color */
				points.num -= colors.num;
				colors.num = 0;
				break;
			case 'd': /* delete one former point of the former color, if there is */
				if (colors.num > 0)
				{
					points.num--;
					colors.num--;
				}
				break;
			case 'q': /* quit */
				goto quit;
			default:
				break;
		}
	}

quit:	fclose(fp); /* close the file */
	cvReleaseCapture(&capture); /* release the capture from the camera */
	cvDestroyWindow("MyVision_PickingUp"); /* shutdown the window */

	return 0;
}
