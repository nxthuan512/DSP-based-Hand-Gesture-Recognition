//#include <def_val.h>
void draw_rectangle(unsigned short *RGB, unsigned int wx, unsigned int wy, unsigned int w, unsigned int h, unsigned short color)
{
	unsigned short i, j;
	unsigned short disLinePitch = 1024;
	unsigned short wx_w = (wx + w > 640) ? 640 : wx + w;
	unsigned short wy_h = (wy + h > 480) ? 480 : wy + h;
	for(i = wx; i < wx_w; i++)
	{
		*(RGB + wy*disLinePitch + i) = color;
		*(RGB + wy*disLinePitch + i + 1) = color;
		*(RGB + wy_h*disLinePitch + i) = color;
		*(RGB + wy_h*disLinePitch + i + 1) = color;
	}
	for(j = wy; j < wy_h; j++)
	{
		*(RGB + j*disLinePitch + wx) = color;
		*(RGB + (j + 1)*disLinePitch + wx) = color;
		*(RGB + j*disLinePitch + wx_w) = color;
		*(RGB + (j + 1)*disLinePitch + wx_w) = color;
	}
}
