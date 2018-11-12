void skin_ycbcr
(
    const unsigned char     *y_data,    /* Luminence data        (Y')       */
    const unsigned char     *cb_data,   /* Blue color-difference (B'-Y')    */
    const unsigned char     *cr_data,   /* Red color-difference  (R'-Y')    */
    unsigned char *restrict rgb_data,  /* RGB 5:6:5 packed pixel output.   */ 	
    unsigned                num_pixels  /* # of luma pixels to process.     */
)
{
    unsigned short     i;                      /* Loop counter                     */
    unsigned char     y0;                 /* Individual Y components          		*/
    unsigned char     cb, cr;                 /* Color difference components      */

	/* -------------------------------------------------------------------- */
    /*  Iterate for num_pixels/2 iters, since we process pixels in pairs.   */
    /* -------------------------------------------------------------------- */
    i = num_pixels >> 1;
	//i = num_pixels;

    while (i-->0)
    {
        y0 = *y_data++;
		cb = *cb_data++;								
        cr = *cr_data++;
		//if((2*cr + cb) > 378 && (2*cr + cb) < 403
		 	//&& (cr - cb) > 1 && (cr - cb) < 20
		 	//&& (7*y0 + 5*(cr - cb)) < 1400 && (10*y0 - 3*(cr - cb)) > 850
			//&& (7*y0 + 5*(cr - cb)) < 800 && (10*y0 - 3*(cr - cb)) > 200)
        if(y0 > 45 && y0 < 235 && cb < 125 && cr > 130)
		{
			*rgb_data++ = 1;
			*rgb_data++ = 1;
			*y_data++;
		}
		else
		{
			*rgb_data++ = 0;
			*rgb_data++ = 0;
			*y_data++;
		}
    }

    return;
}

