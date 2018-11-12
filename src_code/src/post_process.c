#include <def_val.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

int post_process(unsigned char num_detected_hand)
{
	unsigned short resmax[30][4];
	unsigned short res1[30][4];
	unsigned char res2[30];
	unsigned char size_of_res1 = 0;
	unsigned char num_hands_final = 0;
	unsigned i, j, yet;
	unsigned short x_i;
	unsigned short y_i;
	unsigned short w_i;
	unsigned short h_i;
	for(i = 0; i < num_detected_hand; i++)
	{
		yet = 0;
		x_i = Hands[i][0];
		y_i = Hands[i][1];
		w_i = Hands[i][2];
		h_i = Hands[i][3];
		for(j = 0; j < size_of_res1; j++)
		{
			if(!(resmax[j][0] > (x_i + w_i)
				|| (resmax[j][0] + resmax[j][2]) < x_i
				|| resmax[j][1] > (y_i + h_i)
				|| (resmax[j][1] + resmax[j][3]) < y_i))
			{
				resmax[j][0] = MIN(resmax[j][0],x_i);
				resmax[j][1] = MIN(resmax[j][1],y_i);
				resmax[j][2] = MAX(resmax[j][0] + resmax[j][2],x_i + w_i);
				resmax[j][3] = MAX(resmax[j][1] + resmax[j][3],y_i + h_i);
				res1[j][0] = (res1[j][0] + x_i) >> 1;
				res1[j][1] = (res1[j][1] + y_i) >> 1;
				res1[j][2] = (res1[j][2] + w_i) >> 1;
				res1[j][3] = (res1[j][3] + h_i) >> 1;
				res2[j]++;
				yet = 1;
				break;
			}
		}
		if(yet == 0)
		{
			res1[size_of_res1][0] = x_i;
			res1[size_of_res1][1] = y_i;
			res1[size_of_res1][2] = w_i;
			res1[size_of_res1][3] = h_i;
			resmax[size_of_res1][0] = x_i;
			resmax[size_of_res1][1] = y_i;
			resmax[size_of_res1][2] = w_i;
			resmax[size_of_res1][3] = h_i;
			res2[size_of_res1] = 1;
			size_of_res1++;
		}
	}
	for(i = 0; i < size_of_res1; i++)
	{
		if(res2[i] > 2)
		{
			Hands_Combine[num_hands_final][0] = res1[i][0];
			Hands_Combine[num_hands_final][1] = res1[i][1];
			Hands_Combine[num_hands_final][2] = res1[i][2];
			Hands_Combine[num_hands_final][3] = res1[i][3];
			num_hands_final++;
		}
	}
	return num_hands_final;
}
