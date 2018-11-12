#include <math.h>
//#include <def_val.h>

void meanshift(unsigned char *backprojection, unsigned short *CamshiftWindow,
				unsigned short image_width, unsigned short image_height)
{
	unsigned char MeanCoverging = 1;
	unsigned char loop_count = 0;
	unsigned int oldX = 0;
	unsigned int oldY = 0;
	unsigned short xc = 0;
	unsigned short yc = 0;
	unsigned short size = 0;
	unsigned int M00, M01, M10;
	unsigned int i, j;
	unsigned char P = 5;
	unsigned char T = 3;
	while(MeanCoverging)
	{
		M00 = 0;
		M01 = 0;
		M10 = 0;
		for(i = CamshiftWindow[1] - P; i < CamshiftWindow[1] + CamshiftWindow[3] + P; i++)
		{
			for(j = CamshiftWindow[0] - P; j < CamshiftWindow[0] + CamshiftWindow[2] + P; j++)
			{
				if((i > image_height) || (j > image_width) || i < 1 || j < 1)
				{
					continue;
				}
				M00 = M00 + backprojection[i*image_width + j];
				M10 = M10 + backprojection[i*image_width + j]*i;
				M01 = M01 + backprojection[i*image_width + j]*j;
			}
		}
		xc = M10 / M00;
		yc = M01 / M00;
		oldY = CamshiftWindow[1];
		oldX = CamshiftWindow[0];
		CamshiftWindow[0] = yc - (CamshiftWindow[2] >> 1);
		CamshiftWindow[1] = xc - (CamshiftWindow[3] >> 1);
		//Check threshold
		if(abs(oldY - CamshiftWindow[1]) < T || abs(oldX - CamshiftWindow[0]) < T || loop_count > 10)
		{
			MeanCoverging = 0;
		}
		loop_count++;
	}

	size = sqrt(M00 >> 8);
	size = size << 1;
	CamshiftWindow[2] = size + (size >> 2);
	CamshiftWindow[3] = size + (size >> 1);
	CamshiftWindow[0] = yc - (CamshiftWindow[2] >> 1);
	CamshiftWindow[1] = xc - (CamshiftWindow[3] >> 1);
}
