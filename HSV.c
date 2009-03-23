#include "MyVision.h"

/* names of the colors */
const char color_name[][10] = {"red", "green", "blue", "white", "black"};
/* standard color value, in RGB */
const int color_value_rgb[][3] = {{0, 0, 255}, {0, 255, 0}, {255, 0, 0}, {255, 255, 255}, {0, 0, 0}};

struct HSVColor RGB2HSV(int R, int G, int B)
{
	/*
	 * Convert RGB color to HSV.
	 * TODO: This algorithm may or may not be efficient, which should be
	 * verified in thoroughly examination.
	 */
	int H, S, V;

	if (R > G)
	{
		if (B > R) /* max = B; min = R; B != 0; B != R; */
		{
			V = B;
			S = (B - R) * 255 / B;
			H = 240 + (R - G) * 60 / (B - R);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G < B) /* max = R; min = G; R != 0; R != G; */
		{
			V = R;
			S = (R - G) * 255 / R;
			H = 360 + (G - B) * 60 / (R - G);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else /* B <= R && B <= G */ /* max = R; min = B; R != 0; R != B; */
		{
			V = R;
			S = (R - B) * 255 / R;
			H = (G - B) * 60 / (R - B);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
	}
	else /* R <= G */
	{
		if (B < R) /* max = G; min = B; G != 0; G != B; */
		{
			V = G;
			S = (G - B) * 255 / G;
			H = 120 + (B - R) * 60 / (G - B);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G < B) /* max = B; min = R; B != 0; B != R; */
		{
			V = B;
			S = (B - R) * 255 / B;
			H = 240 + (R - G) * 60 / (B - R);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G != R) /* max = G; min = R; G != 0; G != R; */
		{
			V = G;
			S = (G - R) * 255 / G;
			H = 120 + (B - R) * 60 / (G - R);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else /* B = R = G = B */ /* max = G; min = R; G == 0; G == R; */
		{
			V = G;
			S = 0;
			H = 0;
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
