/*
 *  Copyright 2004 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */
#include <std.h>
#include <tsk.h>
#include <sem.h>
#include <gio.h>

#include <csl_dat.h>
#include <csl_cache.h>


#include <fvid.h>
#include <edc.h>
#include <vport.h>
#include <vportcap.h>
#include <vportdis.h>
#include <saa7105.h>
#include <tvp51xx.h>

#include <evmdm642.h>

#include <evmdm642_capParamsSDTVDefault.h>
#include <evmdm642_disParamsVGADefault.h>

#include <stdio.h>
#include <VLIB_prototypes.h>

#include <def_val.h>
#include "profiler_timer2.h"

#define DDR2HEAP   		0
#define MINBLOBAREA 	1500
#define IMAGEWIDTH  	320
#define IMAGEHEIGHT 	240
#define IMAGESIZE 		76800
#define PACKEDSIZE 		9600

#define NONE		 	0
#define FACEDETECTED 	1
#define HANDDETECTED 	2

//Timer device number to be used for profiling (Use -1 for any timer device)
#define TIMER_NUM			1

//Ratio of (CPU CLK/TIMER CLK) - it is always 8 for 64xx with internal CLK
//  source for timer
#define CPUCLK_BY_TIMCLK	8

VPORT_PortParams EVMDM642_vCapParamsPort
  = EVMDM642_CAP_PARAMS_PORT_EMBEDDED_DEFAULT;
  
/* caputure configuration parameters */
/* embedded sync mode is recommended as it offers better re-sync capability */
static VPORTCAP_Params EVMDM642_vCapParamsChan
  = EVMDM642_CAP_PARAMS_CHAN_EMBEDDED_DEFAULT(NTSC640);

static TVP51XX_ConfParams EVMDM642_vCapParamsTVP51XX
  = EVMDM642_CAP_PARAMS_TVP51XX_EMBEDDED_DEFAULT(NTSC601, COMPOSITE, 1);

/* coefficients for color space conversion */
//static const short coeffs[5] = {0x2543, 0x3313, -0x0C8A, -0x1A04, 0x408D};

/* display configuration parameters */
VPORT_PortParams EVMDM642_vDisParamsPort
  = EVMDM642_DIS_PARAMS_PORT_DEFAULT;

/* The available display modes are: **
** VGA(640x480@60fps)               **
** SVGA(800x600@60fps)              **
** XGA(1024x768@60fps)              */

static VPORTDIS_Params EVMDM642_vDisParamsChan
  = EVMDM642_DIS_PARAMS_CHAN_RGB565_DEFAULT(XGA);

static SAA7105_ConfParams EVMDM642_vDisParamsSAA7105
  = EVMDM642_DIS_PARAMS_SAA7105_RGB565_DEFAULT(XGA);
                                               
/* coefficients for color space conversion */
static const short coeffs[5] = {0x2543, 0x3313, -0x0C8A, -0x1A04, 0x408D};
/* heap IDs defined in the BIOS configuration file */
extern Int EXTERNALHEAP;

#pragma DATA_SECTION(ii_m, ".EXTPROCBUFF"); 
#pragma DATA_SECTION(pLastLine, ".EXTPROCBUFF"); 
#pragma DATA_SECTION(skin_buf, ".EXTPROCBUFF");    
#pragma DATA_SECTION(y_buf, ".EXTPROCBUFF");
#pragma DATA_SECTION(cb_buf, ".EXTPROCBUFF");
#pragma DATA_SECTION(cr_buf, ".EXTPROCBUFF");
#pragma DATA_SECTION(skin_packed, ".EXTPROCBUFF");
#pragma DATA_SECTION(erode_dilate, ".EXTPROCBUFF");
//#pragma DATA_SECTION(CCMap, ".EXTPROCBUFF");
#pragma DATA_SECTION(backprojection_disp, ".EXTPROCBUFF"); 
#pragma DATA_SECTION(RGB, ".EXTPROCBUFF");
#pragma DATA_SECTION(internalBuffer, ".EXTPROCBUFF");
#pragma DATA_SECTION(target, ".EXTPROCBUFF");
#pragma DATA_SECTION(Hist, ".EXTPROCBUFF");
#pragma DATA_SECTION(internalH, ".EXTPROCBUFF");
#pragma DATA_SECTION(backprojection, ".EXTPROCBUFF");
#pragma DATA_ALIGN(ii_m, 64); 
#pragma DATA_ALIGN(pLastLine, 64); 
#pragma DATA_ALIGN(skin_buf, 64);    
#pragma DATA_ALIGN(y_buf, 64);
#pragma DATA_ALIGN(cb_buf, 64);
#pragma DATA_ALIGN(cr_buf, 64);
#pragma DATA_ALIGN(skin_packed, 64);
#pragma DATA_ALIGN(erode_dilate, 64);
//#pragma DATA_ALIGN(CCMap, 64);
#pragma DATA_ALIGN(backprojection_disp, 64);
#pragma DATA_ALIGN(RGB, 64);
#pragma DATA_ALIGN(internalBuffer, 64);
#pragma DATA_ALIGN(target, 64);
#pragma DATA_ALIGN(Hist, 64);
#pragma DATA_ALIGN(backprojection, 64);
//Integral
unsigned int ii_m[IMAGESIZE];
unsigned int pLastLine[IMAGEWIDTH];
//Skin segmentation
unsigned char skin_buf[IMAGESIZE];
unsigned char y_buf[100800]; // (WIDTH*HEIGHT)*21 / 64
unsigned char cb_buf[50400]; // (WIDTH*HEIGHT)*21 / 64
unsigned char cr_buf[50400]; // (WIDTH*HEIGHT)*21 / 64
unsigned int skin_packed[PACKEDSIZE];
//Connect Component
unsigned int erode_dilate[PACKEDSIZE];
//unsigned char CCMap[IMAGESIZE];
unsigned short backprojection_disp[IMAGESIZE];
//Camshift
unsigned short RGB[IMAGESIZE];
unsigned short target[IMAGESIZE];
unsigned char backprojection[IMAGESIZE];
unsigned short internalBuffer[4096];
unsigned short Hist[4096];
unsigned short internalH[4096];
unsigned short CamshiftWindow[4] = {0, 0, 0, 0};
unsigned short numB = 4096;
unsigned short binWeight = 1; 
        
/*
 ======== main ========
 */
main()
{
    /******************************************************/
    /* open CSL DAT module for fast copy                  */
    /******************************************************/
    CSL_init();
    CACHE_clean(CACHE_L2ALL, 0, 0);
    CACHE_setL2Mode(CACHE_256KCACHE);
    CACHE_enableCaching(CACHE_EMIFA_CE00);
    CACHE_enableCaching(CACHE_EMIFA_CE01);
    DAT_open(DAT_CHAANY, DAT_PRI_LOW, DAT_OPEN_2D);

}

/*
 * ======== tskVideoLoopback ========
 * video loopback function.
 */
void tskVideoLoopback()
{
	// Capture Device
	Int status, id;
	Int numLinesCap = (EVMDM642_vCapParamsChan.fldYStop1 -
       					EVMDM642_vCapParamsChan.fldYStrt1+1) * 2;
	FVID_Handle capChan;
	Int numPixels = EVMDM642_vCapParamsChan.fldXStop1 -
       				EVMDM642_vCapParamsChan.fldXStrt1+1;
	FVID_Frame *capFrameBuf;
	Int capLinePitch = EVMDM642_vCapParamsChan.fldXStop1 -
       					EVMDM642_vCapParamsChan.fldXStrt1+1;
	FVID_Handle disChan;
    FVID_Frame *disFrameBuf;
	Int disLinePitch = EVMDM642_vDisParamsChan.imgHSizeFld1;
    Int numLinesDis = EVMDM642_vDisParamsChan.imgVSizeFld1;
    Int numLines = (numLinesDis > numLinesCap) ? numLinesCap : numLinesDis;

	Int i, j;
	Int frames = 0;
	Int num_hand_final;
	Int max = 0;
	Int count = 0;
	Int flag = 0;
	Int cpu_cycles1;
	Int temp;

	void *pBuf;
	int  sizeOfCCHandle;
	int maxBytesRequired;
	int bytesRecommended;
    int  numCCs;

    VLIB_CCHandle * handle;
    VLIB_CC         vlibBlob;

	Int blob_width, blob_height;
	unsigned char detected = 0;

	unsigned short wX, wY, wW, wH, wX2, wY2;
	unsigned short *gesture;
	gesture = OK;

	image_width = IMAGEWIDTH;
	image_height = IMAGEHEIGHT;

    /******************************************************/
    /* allocate both capture and display frame buffers    */
    /* in external heap memory                            */
    /******************************************************/
    EVMDM642_vDisParamsChan.segId = EXTERNALHEAP;
    EVMDM642_vDisParamsSAA7105.hI2C = EVMDM642_I2C_hI2C;
	
	EVMDM642_vCapParamsChan.segId = EXTERNALHEAP;
    EVMDM642_vCapParamsTVP51XX.hI2C = EVMDM642_I2C_hI2C;

    /******************************************************/
    /* initialization of capture driver                   */
    /******************************************************/
    capChan = FVID_create("/VP0CAPTURE/A/0",
            IOM_INPUT, &status, (Ptr)&EVMDM642_vCapParamsChan, NULL);

    /******************************************************/
    /* initialization of display driver                   */
    /******************************************************/
    disChan = FVID_create("/VP2DISPLAY", IOM_OUTPUT,
        &status, (Ptr)&EVMDM642_vDisParamsChan, NULL);


    /******************************************************/
    /* configure video encoder & decoder                  */
    /******************************************************/
    FVID_control(disChan, VPORT_CMD_EDC_BASE + EDC_CONFIG,
        (Ptr)&EVMDM642_vDisParamsSAA7105);
		
	FVID_control(capChan, VPORT_CMD_EDC_BASE + EDC_CONFIG,
        (Ptr)&EVMDM642_vCapParamsTVP51XX);
	CACHE_clean(CACHE_L2ALL, 0, 0);
    /******************************************************/
    /* start capture & display operation                  */
    /******************************************************/
    FVID_control(disChan, VPORT_CMD_START, NULL);

	FVID_control(capChan, VPORT_CMD_START, NULL);
	
    /********************************************************/
    /* request a frame buffer from display & capture driver */
    /********************************************************/
    FVID_alloc(disChan, &disFrameBuf);

	FVID_alloc(capChan, &capFrameBuf);

	//Configure timer to be used for profiling
	if ((status = profile_timConfig(TIMER_NUM, CPUCLK_BY_TIMCLK)) != 0)
	{
		switch(status)
		{
			case ERR_INVDEVNUM :
				printf("\nERROR:Invalide timer device number input");
				break;
			case ERR_INVCLKRATIO :
				printf("\nERROR:Invalide CLK ratio input");
				break;
			case ERR_BADHANDLE :
				printf("\nERROR:Timer open failed with invalid handle");
				break;
			default:
				printf("\nERROR:Profiler configure failed");			
		}
		printf("\nTEST FAILED\n");
		exit(1);
	}

	while(1)
	{
		VLIB_imagePyramid8((unsigned char*)capFrameBuf->frame.iFrm.y1, numPixels, numLines, y_buf);
		VLIB_imagePyramid8((unsigned char*)capFrameBuf->frame.iFrm.cb1, (numPixels >> 1), numLines, cb_buf);
		VLIB_imagePyramid8((unsigned char*)capFrameBuf->frame.iFrm.cr1, (numPixels >> 1), numLines, cr_buf);
		
		// Convert YCBCR to RGB for display
		for(i = 0; i < numLines; i++) {
			yc2rgb16(coeffs, capFrameBuf->frame.iFrm.y1 + i * (capLinePitch),
					 capFrameBuf->frame.iFrm.cb1 + (capLinePitch >> 1) * i,
					 capFrameBuf->frame.iFrm.cr1 + (capLinePitch >> 1) * i,
					 disFrameBuf->frame.rpFrm.buf + i * (disLinePitch << 1),
					 numPixels);
		}
		
		//Convert YCBCR scaled for CAMSHIFT
		for(i = 0; i < IMAGEHEIGHT; i++) {
			yc2rgb16(coeffs, y_buf + i * (IMAGEWIDTH),
					 cb_buf + (IMAGEWIDTH >> 1) * i,
					 cr_buf + (IMAGEWIDTH >> 1) * i,
					 RGB + i * (IMAGEWIDTH),
					 IMAGEWIDTH);
		}
		CACHE_clean(CACHE_L2ALL, 0, 0);
		
		id = DAT_copy2d(DAT_1D2D, 
						OK, 
						(unsigned short*)disFrameBuf->frame.rpFrm.buf + 512000, 
						160 << 1, 
						160, 
						disLinePitch << 1);
		id = DAT_copy2d(DAT_1D2D, 
						Fist, 
						(unsigned short*)disFrameBuf->frame.rpFrm.buf + 512160, 
						160 << 1, 
						160, 
						disLinePitch << 1);
		id = DAT_copy2d(DAT_1D2D, 
						Palm, 
						(unsigned short*)disFrameBuf->frame.rpFrm.buf + 512320, 
						160 << 1, 
						160, 
						disLinePitch << 1);
		id = DAT_copy2d(DAT_1D2D, 
						Open, 
						(unsigned short*)disFrameBuf->frame.rpFrm.buf + 512480, 
						160 << 1, 
						160, 
						disLinePitch << 1);
		DAT_wait(id);
		
		//Calculate Integral Image for Hands Detection
	    memset(pLastLine, 0, IMAGEWIDTH << 2);
		VLIB_integralImage8(y_buf, IMAGEWIDTH, IMAGEHEIGHT, pLastLine, ii_m);

		if(detected == NONE)
		{
			num_hand = 0;
			face_detection(ii_m, 0, IMAGEWIDTH, 0, IMAGEHEIGHT);
			if(num_hand > 4)
			{
				num_hand_final = post_process(num_hand);
				/*
				for(i = 0; i < num_hand_final; i++)                                                                                          
		 	  	{                                                                                                                      
		  	  		draw_rectangle((unsigned short*)disFrameBuf->frame.rpFrm.buf, 
									(Hands_Combine[0][0] << 1), 
									(Hands_Combine[0][1] << 1), 
									(Hands_Combine[0][2] << 1), 
									(Hands_Combine[0][3] << 1), 
						  	  		color);
		  	  	}
				*/
				detected = FACEDETECTED;
				wH = Hands_Combine[0][3] >> 1;
				wW = Hands_Combine[0][2] >> 1;
				wY = Hands_Combine[0][1] + (Hands_Combine[0][3] >> 1);
				wX = Hands_Combine[0][0] + (Hands_Combine[0][2] >> 2);
				//Histogram
				memset(internalBuffer, 0, numB << 1);
			    VLIB_histogram_1D_Init_U16( binEdges, numB, internalBuffer );
			    memset(internalH, 0, numB << 1);
			    memset(Hist, 0, numB << 1);
			    for(i = 0; i < wH; i++)
				{
					for(j = 0; j < wW; j++)
					{
						target[i*wW + j] = RGB[(i + wY)*IMAGEWIDTH + (j + wX)] >> 4;
					}
				}
				VLIB_histogram_1D_U16(target, wW * wH, numB, binWeight, internalBuffer, internalH, Hist);
				for(i = 0; i < numB; i++)
				{
					max = (max < Hist[i]) ? Hist[i] : max;
				}

				for(i = 0; i < numB; i++)
				{
					Hist[i] = ((double)Hist[i] / max)*255;
				}
			}
		}
		if(detected != NONE)
		{
			//Start profiler
			//profile_begin();
			for(i = 1; i < IMAGEHEIGHT; i += 2)
			{
				for(j = 1; j < IMAGEWIDTH; j += 2)
				{
					backprojection[i*IMAGEWIDTH + j] = Hist[RGB[i*IMAGEWIDTH + j] >> 4];
					backprojection[(i - 1)*IMAGEWIDTH + j] = Hist[RGB[(i - 1)*IMAGEWIDTH + j] >> 4];
					backprojection[i*IMAGEWIDTH + j - 1] = Hist[RGB[i*IMAGEWIDTH + j - 1] >> 4];
					backprojection[(i - 1)*IMAGEWIDTH + j - 1] = Hist[RGB[(i - 1)*IMAGEWIDTH + j - 1] >> 4];
				}
			}
			//Stop the profiler
			//cpu_cycles1 = profile_end();
			//printf("CPU cycles : %d\n",cpu_cycles1);
			//Close the profiler just before returning from main()
			//profile_timClose();
			
			if(detected == HANDDETECTED)
			{
				//Start profiler
				//profile_begin();
				meanshift(backprojection, CamshiftWindow, IMAGEWIDTH, IMAGEHEIGHT);
				
				if(CamshiftWindow[2] < 35 || CamshiftWindow[3] < 35 || count > 10)
				{
					detected = FACEDETECTED;
					num_hand_final = 0;
					num_hand = 0;
					count = 0;
				}
				if(detected == HANDDETECTED && frames > 5)
				{
					frames = 0;
					flag = 0;
					count++;
					temp = CamshiftWindow[0] - (CamshiftWindow[2] >> 2);
					wX = (temp > 0) ? temp : 0;
					temp = CamshiftWindow[1] - (CamshiftWindow[3] >> 1); 
					wY = (temp > 0) ? temp : 0;
					temp = wX + CamshiftWindow[2] + (CamshiftWindow[2] >> 1);
					wX2 = (temp > IMAGEWIDTH) ? temp : IMAGEWIDTH;
					temp = wY + CamshiftWindow[3] + (CamshiftWindow[3] >> 1);
					wY2 = (temp > IMAGEHEIGHT) ? temp : IMAGEHEIGHT;
					num_hand = 0;
				  	hands_detectionA(ii_m, wX, wX2, wY, wY2);
					if(num_hand > 2){flag = 1; gesture = Fist;}
					if(flag == 0)
					{
						num_hand = 0;
				  		hands_detectionB(ii_m, wX, wX2, wY, wY2);
						if(num_hand > 2) {flag = 1; gesture = Palm;}
					}
					if(flag == 0)
					{
						num_hand = 0;
				  		hands_detection5(ii_m, wX, wX2, wY, wY2);
						if(num_hand > 2) {flag = 1; gesture = Open;}
					}
					if(flag == 0)
					{
						num_hand = 0;
				  		hands_detectionOK(ii_m, wX, wX2, wY, wY2);
						if(num_hand > 2) {flag = 1; gesture = OK;}
					}
					if(flag == 1)
					{
						num_hand_final = post_process(num_hand);
						CamshiftWindow[3] = Hands_Combine[0][3];
						CamshiftWindow[2] = Hands_Combine[0][2];
						CamshiftWindow[1] = Hands_Combine[0][1];
						CamshiftWindow[0] = Hands_Combine[0][0];
						count = 0;
					}
				
				}
				//frames++;
				//Stop the profiler
				//cpu_cycles1 = profile_end();
				//printf("CPU cycles : %d\n",cpu_cycles1);
				//Close the profiler just before returning from main()
				//profile_timClose();
				
			}

			if(detected == FACEDETECTED && frames > 5)
			{
				frames = 0;
				/*
				IMG_pix_expand(IMAGESIZE, backprojection, backprojection_disp);
				for(i = 0; i < IMAGEHEIGHT; i++)
				{
					DAT_copy(backprojection_disp + i*IMAGEWIDTH, 
							(unsigned short*)disFrameBuf->frame.rpFrm.buf + i*disLinePitch + numPixels, 
							IMAGEWIDTH<<1);
				}
				*/
				IMG_thr_gt2thr(backprojection, skin_buf, IMAGEWIDTH, IMAGEHEIGHT, 1);
				
				//Packed to 1 bit-per-pixel
				VLIB_packMask32(skin_buf, skin_packed, IMAGESIZE);

				//Erode & dilate
				VLIB_erode_bin_cross((const unsigned char *)skin_packed, (unsigned char *)erode_dilate + 40, 75520, IMAGEWIDTH);
				VLIB_dilate_bin_square((const unsigned char *)erode_dilate, (unsigned char *)skin_packed + 40, 74880, IMAGEWIDTH);
				VLIB_dilate_bin_square((const unsigned char *)skin_packed, (unsigned char *)erode_dilate + 40, 74240, IMAGEWIDTH);
				
				//Connect Component Labeling
				VLIB_calcConnectedComponentsMaxBufferSize(IMAGEWIDTH,IMAGEHEIGHT,MINBLOBAREA,&maxBytesRequired);
				bytesRecommended = maxBytesRequired;

				sizeOfCCHandle =  VLIB_GetSizeOfCCHandle();  
				handle = (VLIB_CCHandle *) MEM_alloc(DDR2HEAP, sizeOfCCHandle, 8);
				
				// Set-up Memory Buffers
				pBuf	= (void *)  MEM_alloc(DDR2HEAP, bytesRecommended, 8);
				status	= VLIB_initConnectedComponentsList(handle, pBuf, bytesRecommended);
				status = VLIB_createConnectedComponentsList(handle,
			        IMAGEWIDTH,
			        IMAGEHEIGHT,
			        (int*)erode_dilate,
			        MINBLOBAREA,
			        1);
				
				VLIB_getNumCCs(handle, &numCCs);			
				for (i=0; i < numCCs; i++)
			    {
			        VLIB_getCCFeatures(handle, &vlibBlob, i);
					blob_width = vlibBlob.xmax - vlibBlob.xmin;
					blob_height = vlibBlob.ymax - vlibBlob.ymin;
					if(blob_width < 35 || blob_height < 35) continue;
					vlibBlob.xmin = ((vlibBlob.xmin - (blob_width >> 2)) > 0) ? (vlibBlob.xmin - (blob_width >> 2)) : 0;
					vlibBlob.xmax = ((vlibBlob.xmax + (blob_width >> 2)) < IMAGEWIDTH) ? (vlibBlob.xmax + (blob_width >> 2)) : IMAGEWIDTH;
					vlibBlob.ymin = ((vlibBlob.ymin - (blob_height >> 2)) > 0) ? (vlibBlob.ymin - (blob_height >> 2)) : 0;
					vlibBlob.ymax = ((vlibBlob.ymax + (blob_height >> 2)) < IMAGEHEIGHT) ? (vlibBlob.ymax + (blob_height >> 2)) : IMAGEHEIGHT;
					blob_width = vlibBlob.xmax - vlibBlob.xmin;
					blob_height = vlibBlob.ymax - vlibBlob.ymin;
					num_hand = 0;
				  	hands_detectionA(ii_m, vlibBlob.xmin, vlibBlob.xmax, vlibBlob.ymin, vlibBlob.ymax);
					if(num_hand > 2){detected = HANDDETECTED; gesture = Fist; break;}
					num_hand = 0;
				  	hands_detectionB(ii_m, vlibBlob.xmin, vlibBlob.xmax, vlibBlob.ymin, vlibBlob.ymax);
					if(num_hand > 2) {detected = HANDDETECTED; gesture = Palm; break;}
					num_hand = 0;
					hands_detection5(ii_m, vlibBlob.xmin, vlibBlob.xmax, vlibBlob.ymin, vlibBlob.ymax);
					if(num_hand > 2){detected = HANDDETECTED; gesture = Open; break;}
					num_hand = 0;
					hands_detectionOK(ii_m, vlibBlob.xmin, vlibBlob.xmax, vlibBlob.ymin, vlibBlob.ymax);
					if(num_hand > 2){detected = HANDDETECTED; gesture = OK; break;}
			    }
				MEM_free(DDR2HEAP, pBuf, maxBytesRequired);
				MEM_free(DDR2HEAP, handle, sizeOfCCHandle);
				if(detected == HANDDETECTED)
				{
					num_hand_final = post_process(num_hand);
					CamshiftWindow[3] = Hands_Combine[0][3];
					CamshiftWindow[2] = Hands_Combine[0][2];
					CamshiftWindow[1] = Hands_Combine[0][1];
					CamshiftWindow[0] = Hands_Combine[0][0];
				}
			}
			
			//num_hand_final = post_process(num_hand);
			
			if(num_hand_final)                                                                                          
	 	  	{
				temp = CamshiftWindow[0] - (CamshiftWindow[2] >> 2);
				wX = (temp > 0) ? temp : 0;
				temp = CamshiftWindow[1] - (CamshiftWindow[3] >> 1); 
				wY = (temp > 0) ? temp : 0;
				temp = wX + CamshiftWindow[2] + (CamshiftWindow[2] >> 1);	 	  		                                                                                                                    
	  	  		draw_rectangle((unsigned short*)disFrameBuf->frame.rpFrm.buf, 
								//(CamshiftWindow[0] - (CamshiftWindow[2] >> 2))<< 1, 
								//(CamshiftWindow[1] - (CamshiftWindow[3] >> 1))<< 1, 
								wX << 1,
								wY << 1,
								(CamshiftWindow[2] + (CamshiftWindow[2] >> 1))<< 1, 
								(CamshiftWindow[3] + (CamshiftWindow[3] >> 1))<< 1,
					  	  		63488);
	  	  	}
			
			id = DAT_copy2d(DAT_1D2D, 
							gesture, 
							(unsigned short*)disFrameBuf->frame.rpFrm.buf + 640, 
							160 << 1, 
							160, 
							disLinePitch << 1);
			DAT_wait(id);
		}
		
		frames++;
		FVID_exchange(capChan, &capFrameBuf);
		//CACHE_clean(CACHE_L2ALL, 0, 0);
	    FVID_exchange(disChan, &disFrameBuf);
	}
}
