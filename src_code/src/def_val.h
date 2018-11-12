#ifndef _DEF_VAL_H_
#define _DEF_VAL_H_
              
extern unsigned short image_height;
extern unsigned short image_width;


//Coordinate hands detected
extern unsigned int Hands[30][4];
extern unsigned int Hands_Combine[30][4];
extern unsigned int num_hand;

// Haar Cascade
//A
extern unsigned char ScaleA[11];
extern unsigned short Window_WA[11];
extern unsigned short Window_HA[11];
extern unsigned short normalizeA[11];
extern unsigned char stageA[20];
extern int thresholdA[635];
extern int left_valA[635];
extern int right_valA[635];
extern int stage_thresholdA[20];
extern char RectweightA[1905];
extern unsigned char RectxA[1905];
extern unsigned char RectyA[1905];
extern unsigned char RectwA[1905];
extern unsigned char RecthA[1905];
//5
extern unsigned char Scale5[11];
extern unsigned short Window_W5[11];
extern unsigned short Window_H5[11];
extern unsigned short normalize5[11];
extern unsigned char stage5[20];
extern int threshold5[599];
extern int left_val5[599];
extern int right_val5[599];
extern int stage_threshold5[20];
extern char Rectweight5[1797];
extern unsigned char Rectx5[1797];
extern unsigned char Recty5[1797];
extern unsigned char Rectw5[1797];
extern unsigned char Recth5[1797];
//B
extern unsigned char ScaleB[8];
extern unsigned short Window_WB[8];
extern unsigned short Window_HB[8];
extern unsigned short normalizeB[8];
extern unsigned char stageB[20];
extern int thresholdB[437];
extern int left_valB[437];
extern int right_valB[437];
extern int stage_thresholdB[20];
extern char RectweightB[1311];
extern unsigned char RectxB[1311];
extern unsigned char RectyB[1311];
extern unsigned char RectwB[1311];
extern unsigned char RecthB[1311];
//OK
extern unsigned char ScaleOK[9];
extern unsigned short Window_WOK[9];
extern unsigned short Window_HOK[9];
extern unsigned short normalizeOK[9];
extern unsigned char stageOK[20];
extern int thresholdOK[593];
extern int left_valOK[593];
extern int right_valOK[593];
extern int stage_thresholdOK[20];
extern char RectweightOK[1779];
extern unsigned char RectxOK[1779];
extern unsigned char RectyOK[1779];
extern unsigned char RectwOK[1779];
extern unsigned char RecthOK[1779];

//Face
extern unsigned char ScaleFace[7];
extern unsigned short Window_WFace[7];
extern unsigned short Window_HFace[7];
extern unsigned short normalizeFace[7];
extern unsigned char stageFace[22];
extern int thresholdFace[2135];
extern int left_valFace[2135];
extern int right_valFace[2135];
extern int stage_thresholdFace[22];
extern char RectweightFace[6405];
extern unsigned char RectxFace[6405];
extern unsigned char RectyFace[6405];
extern unsigned char RectwFace[6405];
extern unsigned char RecthFace[6405];

//Histogram
extern unsigned short div_table[510];
extern unsigned short binEdges[4096];

//Gestures image
extern unsigned short Fist[25600];
extern unsigned short OK[25600];
extern unsigned short Open[25600];
extern unsigned short Palm[25600];
#endif
