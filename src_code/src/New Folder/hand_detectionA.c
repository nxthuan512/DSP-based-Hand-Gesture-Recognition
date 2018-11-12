#include <math.h>
#include <fastrts62x64x.h>
#include <def_val.h>
void hands_detectionA(unsigned int *ii_m, 
					unsigned short xmin, 
					unsigned short xmax,
					unsigned short ymin,
					unsigned short ymax)
{
	//unsigned int count = 0;
	unsigned int i;
	unsigned int wy;
	unsigned int wx;
	unsigned char Scale;
	unsigned short w;
	unsigned short h;
	unsigned short normalize;
	unsigned int i_stage, i_trees, i_feature;
	unsigned int curr_threshold_index;
	unsigned int curr_feature_index;
	int  sum_stage;
	int Rectangle_sum;
	unsigned int RectX, RectY, RectW, RectH;
	int RectWeight;
	unsigned int A, B, C, D;
	int itt = 6;

	for(i = 0; i < itt; i++)
	{
		Scale = ScaleA[i];
		w = Window_WA[i];
		h = Window_HA[i];
		normalize = normalizeA[i];
		if(w > (xmax - xmin) || h > (ymax - ymin))
		{
			continue;
		}
		// Find hands in the image for the current scale
		for(wy = ymin; wy < ymax - h; wy += Scale)
		{
			for(wx = xmin; wx < xmax - w; wx += Scale)
			{

				curr_threshold_index = 0;
				curr_feature_index = 0;
				sum_stage = 0;
				// Loop through a classifier stage
				for(i_stage = 0; i_stage < 20; i_stage++)
				{
					sum_stage = 0;
					for(i_trees = 0; i_trees < stageA[i_stage]; i_trees++)
					{
						Rectangle_sum = 0;
						for(i_feature = 0; i_feature < 3; i_feature++)
						{
							RectX = RectxA[curr_feature_index]*Scale + wx;
							RectY = RectyA[curr_feature_index]*Scale + wy;
							RectW = RectwA[curr_feature_index]*Scale;
							RectH = RecthA[curr_feature_index]*Scale;
							RectWeight = RectweightA[curr_feature_index];

							/******************************/
							D = *(ii_m + (RectY + RectH - 1)*image_width + RectX + RectW - 1);
							A = *(ii_m + (RectY - 1)*image_width + RectX - 1);
							B = *(ii_m + (RectY - 1)*image_width + RectX + RectW - 1);
							C = *(ii_m + (RectY + RectH - 1)*image_width + RectX- 1);
							/******************************/

							Rectangle_sum += (D + A - B - C) * RectWeight;

							curr_feature_index++;
						}
						Rectangle_sum = Rectangle_sum * normalize;
						
						// Get the values of the current haar-classifiers
						if(Rectangle_sum < thresholdA[curr_threshold_index])
							sum_stage += left_valA[curr_threshold_index];
						else
							sum_stage += right_valA[curr_threshold_index];
						curr_threshold_index++;
					}
					if(sum_stage > stage_thresholdA[i_stage])
						continue;

					else
						break;
				}
				if(i_stage == 20)
				{
					Hands[num_hand][0] = wx;
					Hands[num_hand][1] = wy;
					Hands[num_hand][2] = w;
					Hands[num_hand][3] = h;
					num_hand++;
				}
			}
		}
	}
	//return count;
}
