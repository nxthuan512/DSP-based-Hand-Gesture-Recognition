;
;  Copyright 2003 by Texas Instruments Incorporated.
;  All rights reserved. Property of Texas Instruments Incorporated.
;  Restricted rights to use, duplicate or disclose this code are
;  granted through contract.
;  
;
; "@(#) DDK 1.10.00.21 06-26-03 (ddk-b10)"
* ========================================================================= *
*                                                                           *
*   USAGE                                                                   *
*       This function is C callable, and is called according to this        *
*       C prototype:                                                        *
*                                                                           *
*       void ycbcr422pl_to_rgb565                                           *
*       (                                                                   *
*           const short         coeff[5],  -- Matrix coefficients.          *
*           const unsigned char *y_data,   -- Luminence data  (Y')          *
*           const unsigned char *cb_data,  -- Blue color-diff (B'-Y')       *
*           const unsigned char *cr_data,  -- Red color-diff  (R'-Y')       *
*           unsigned short      *rgb_data, -- RGB 5:6:5 packed pixel out.   *
*           unsigned            num_pixels -- # of luma pixels to process.  *
*       )                                                                   *
*                                                                           *
*       The 'coeff[]' array contains the color-space-conversion matrix      *
*       coefficients.  The 'y_data', 'cb_data' and 'cr_data' pointers       *
*       point to the separate input image planes.  The 'rgb_data' pointer   *
*       points to the output image buffer, and must be word aligned.        *
*                                                                           *
*       The kernel is designed to process arbitrary amounts of 4:2:2        *
*       image data, although 4:2:0 image data may be processed as well.     *
*       For 4:2:2 input data, the 'y_data', 'cb_data' and 'cr_data'         *
*       arrays may hold an arbitrary amount of image data.  For 4:2:0       *
*       input data, only a single scan-line (or portion thereof) may be     *
*       processed at a time.                                                *
*                                                                           *
*       The coefficients in the coeff array must be in signed Q13 form.     *
*       These coefficients correspond to the following matrix equation:     *
*                                                                           *
*           [ Y' -  16 ]   [ coeff[0] 0.0000   coeff[1] ]     [ R']         *
*           [ Cb - 128 ] * [ coeff[0] coeff[2] coeff[3] ]  =  [ G']         *
*           [ Cr - 128 ]   [ coeff[0] coeff[4] 0.0000   ]     [ B']         *
*                                                                           *
*   DESCRIPTION                                                             *
*       This function runs for 46 + (num_pixels * 3) cycles, including      *
*       6 cycles of function-call overhead.  Interrupts are masked for      *
*       37 + (num_pixels * 3) cycles.  Code size is 512 bytes.              *
*                                                                           *
*       This kernel performs Y'CbCr to RGB conversion.  From the Color      *
*       FAQ, http://home.inforamp.net/~poynton/ColorFAQ.html :              *
*                                                                           *
*           Various scale factors are applied to (B'-Y') and (R'-Y')        *
*           for different applications.  The Y'PbPr scale factors are       *
*           optimized for component analog video.  The Y'CbCr scaling       *
*           is appropriate for component digital video, JPEG and MPEG.      *
*           Kodak's PhotoYCC(tm) uses scale factors optimized for the       *
*           gamut of film colors.  Y'UV scaling is appropriate as an        *
*           intermediate step in the formation of composite NTSC or PAL     *
*           video signals, but is not appropriate when the components       *
*           are keps separate.  Y'UV nomenclature is now used rather        *
*           loosely, and it sometimes denotes any scaling of (B'-Y')        *
*           and (R'-Y').  Y'IQ coding is obsolete.                          *
*                                                                           *
*       This code can perform various flavors of Y'CbCr to RGB conversion   *
*       as long as the offsets on Y, Cb, and Cr are -16, -128, and -128,    *
*       respectively, and the coefficients match the pattern shown.         *
*                                                                           *
*       The kernel implements the following matrix form, which involves 5   *
*       unique coefficients:                                                *
*                                                                           *
*           [ Y' -  16 ]   [ coeff[0] 0.0000   coeff[1] ]     [ R']         *
*           [ Cb - 128 ] * [ coeff[0] coeff[2] coeff[3] ]  =  [ G']         *
*           [ Cr - 128 ]   [ coeff[0] coeff[4] 0.0000   ]     [ B']         *
*                                                                           *
*                                                                           *
*       Below are some common coefficient sets, along with the matrix       *
*       equation that they correspond to.   Coefficients are in signed      *
*       Q13 notation, which gives a suitable balance between precision      *
*       and range.                                                          *
*                                                                           *
*       1.  Y'CbCr -> RGB conversion with RGB levels that correspond to     *
*           the 219-level range of Y'.  Expected ranges are [16..235] for   *
*           Y' and [16..240] for Cb and Cr.                                 *
*                                                                           *
*           coeff[] = { 0x2000, 0x2BDD, -0x0AC5, -0x1658, 0x3770 };         *
*                                                                           *
*           [ Y' -  16 ]   [ 1.0000    0.0000    1.3707 ]     [ R']         *
*           [ Cb - 128 ] * [ 1.0000   -0.3365   -0.6982 ]  =  [ G']         *
*           [ Cr - 128 ]   [ 1.0000    1.7324    0.0000 ]     [ B']         *
*                                                                           *
*       2.  Y'CbCr -> RGB conversion with the 219-level range of Y'         *
*           expanded to fill the full RGB dynamic range.  (The matrix has   *
*           been scaled by 255/219.)  Expected ranges are [16..235] for Y'  *
*           and [16..240] for Cb and Cr.                                    *
*                                                                           *
*           coeff[] = { 0x2543, 0x3313, -0x0C8A, -0x1A04, 0x408D };         *
*                                                                           *
*           [ Y' -  16 ]   [ 1.1644    0.0000    1.5960 ]     [ R']         *
*           [ Cb - 128 ] * [ 1.1644   -0.3918   -0.8130 ]  =  [ G']         *
*           [ Cr - 128 ]   [ 1.1644    2.0172    0.0000 ]     [ B']         *
*                                                                           *
*       Other scalings of the color differences (B'-Y') and (R'-Y')         *
*       (sometimes incorrectly referred to as U and V) are supported, as    *
*       long as the color differences are unsigned values centered around   *
*       128 rather than signed values centered around 0, as noted above.    *
*                                                                           *
*       In addition to performing plain color-space conversion, color       *
*       saturation can be adjusted by scaling coeff[1] through coeff[4].    *
*       Similarly, brightness can be adjusted by scaling coeff[0].          *
*       General hue adjustment can not be performed, however, due to the    *
*       two zeros hard-coded in the matrix.                                 *
*                                                                           *
*   TECHNIQUES                                                              *
*       Pixel replication is performed implicitly on chroma data to         *
*       reduce the total number of multiplies required.  The chroma         *
*       portion of the matrix is calculated once for each Cb, Cr pair,      *
*       and the result is added to both Y' samples.                         *
*                                                                           *
*       Luma is biased downwards to produce R, G, and B values that are     *
*       signed quantities centered around zero, rather than unsigned qtys.  *
*       This allows us to use SSHL to perform saturation, followed by a     *
*       quick XOR to correct the sign bits in the final packed pixels.      *
*       The required downward bias is 128 shifted left by the Q-point, 13.  *
*                                                                           *
*       To save two instructions, I transformed "(y0-16)*luma - (128<<13)"  *
*       to the slightly more cryptic "y0*luma - (16*luma + (128<<13))".     *
*       This gives me the non-obvious but effective y_bias value            *
*       -((128 << 13) + 16*luma).  The transformation allows me to fit in   *
*       a 6 cycle loop.                                                     *
*                                                                           *
*       Twin pointers are used for the stack and coeff[] arrays for speed.  *
*                                                                           *
*       Because the loop accesses four different arrays at three different  *
*       strides, no memory accesses are allowed to parallelize in the       *
*       loop.  No bank conflicts occur, as a result.                        *
*                                                                           *
*       Creatively constructed multiplies are used to avoid a bottleneck    *
*       on shifts in the loop.  In particular, the 5-bit mask 0xF8000000    *
*       doubles as a right-shift constant that happens to negate while      *
*       shifting.  This negation is reversed by merging the bits with a     *
*       SUB instead of an ADD or OR.                                        *
*                                                                           *
*       Prolog and epilog collapsing have been performed, with only a       *
*       partial stage of prolog and epilog left uncollapsed.  The partial   *
*       stages are interscheduled with the rest of the code for speed.      *
*                                                                           *
*       The stack pointer is saved in IRP to allow all 32 registers to      *
*       be used in the loop.  This enabled prolog collapsing by freeing     *
*       up a predicate register.  The prolog collapse counter is            *
*       implemented as a MPY which shifts a constant left by 3 bits each    *
*       iteration.  The counter is initialized from one of the other        *
*       constant registers, thereby reducing the S-unit bottleneck in the   *
*       setup code.                                                         *
*                                                                           *
*       Instructions have been scheduled to minimize fetch-packet padding   *
*       NOPs.  Only 3 padding NOPs and 1 explicit NOP remain.               *
*                                                                           *
*   ASSUMPTIONS                                                             *
*       An even number of luma samples needs to be processed.               *
*                                                                           *
*       The output image must be word aligned.                              *
*                                                                           *
*   NOTES                                                                   *
*       No bank conflicts occur.                                            *
*                                                                           *
*       Codesize is 512 bytes.                                              *
*                                                                           *
*       On average, one bank per cycle is accessed on a C6201 in the loop,  *
*       with 1 cycle of 6 accessing no banks, and 1 cycle accessing two.    *
*                                                                           *
*       The kernel requires 14 words of stack space.                        *
*                                                                           *
*   SOURCE                                                                  *
*       Poynton, Charles et al.  "The Color FAQ,"  1999.                    *
*           http://home.inforamp.net/~poynton/ColorFAQ.html                 *
*                                                                           *
* ------------------------------------------------------------------------- *
*             Copyright (c) 1999 Texas Instruments, Incorporated.           *
*                            All Rights Reserved.                           *
* ========================================================================= *

                .sect ".data:copyright_h"
_Copyright:     .string "Copyright (C) 1999 Texas Instruments Incorporated. "
                .string "All Rights Reserved.",0
                .sect ".text:hand"
                .global _yc2rgb16
_yc2rgb16:
; =============== SYMBOLIC REGISTER ASSIGNMENTS: ARGUMENTS ================ ;
        .asg            A4,         A_coef      ; Coefficients table
        .asg            B4,         B_y_data    ; Pointer to luma
        .asg            A6,         A_cb_data   ; Pointer to B-Y
        .asg            B6,         B_cr_data   ; Pointer to R-Y
        .asg            A8,         A_rgb_data  ; Pointer to RGB output
        .asg            B8,         B_num_pix   ; # of pixels to process
; ================= SYMBOLIC REGISTER ASSIGNMENTS: SETUP ================== ;
        .asg            B15,        B_SP        ; Stack pointer, B datapath
        .asg            A3,         A_SP        ; Stack pointer, A datapath
        .asg            B0,         B_csr       ; CSR's value
        .asg            B1,         B_noGIE     ; CSR w/ GIE bit cleared
        .asg            B2,         B_irp       ; IRP's value
        .asg            A0,         A_csr       ; Copy of CSR's value
        .asg            B3,         B_ret       ; Return address
        .asg            B7,         B_coef      ; Twin coefficients ptr.
        .asg            A13,        A_rcr       ; Cr's contribution to Red
        .asg            B14,        B_bcb       ; Cb's contribution to Blu
        .asg            A5,         A_gcr_      ; Cr's contribution to Grn
        .asg            A5,         A_gcr       ; A_gcr_ << 16
        .asg            B5,         B_gcb_      ; Cb's contribution to Grn
        .asg            B5,         B_gcb       ; B_gcb_ << 16
        .asg            A1,         A_lneg      ; luma coeff[0] < 0
; ================= SYMBOLIC REGISTER ASSIGNMENTS: KERNEL ================= ;
        .asg            B0,         B_p         ; Prolog collapse counter
        .asg            A2,         A_i         ; Loop trip counter
        .asg            A10,        A_y_ptr     ; Luma data pointer
        .asg            B15,        B_cb_ptr    ; B-Y data pointer
        .asg            B6,         B_cr_ptr    ; R-Y data pointer
        .asg            B11,        B_rgb_ptr   ; RGB output data pointer
        .asg            B12,        B_k32_k128  ; Constant 0x00200080
        .asg            A11,        A_k32_k128  ; Constant 0x00200080
        .asg            A12,        A_one_lum   ; Constant 1 packed w/coeff[0]
        .asg            A13,        A_gcr_rcr   ; coeff[3], coeff[1] packed
        .asg            B14,        B_gcb_bcb   ; coeff[2], coeff[4] packed
        .asg            B10,        B_y_bias    ; -((128<<13) + 16*coeff[0])
        .asg            B13,        B_ms5       ; Mask:  upper 5 bits
        .asg            A14,        A_ms6       ; Mask:  upper 6 bits
        .asg            A15,        A_sflip     ; Sign-flip const 0x84108410
        .asg            A0,         A_y0        ; y0 value from y_data[]
        .asg            B4,         B_y1        ; y1 value from y_data[]
        .asg            B1,         B_cb_       ; cb value prior to level shift
        .asg            A3,         A_cr_       ; cr value prior to level shift
        .asg            B3,         B_cb        ; level-shifted cb value.
        .asg            A4,         A_cr        ; level-shifted cr value
        .asg            B5,         B_y1t_      ; scaled y1, before level shift
        .asg            A3,         A_y0t_      ; scaled y0, before level shift
        .asg            B9,         B_y1t       ; scaled, level-shifted y1
        .asg            A5,         A_y0t       ; scaled, level-shifted y0
        .asg            B3,         B_bt        ; Scaled blue color-diff
        .asg            B1,         B_gt_       ; Scaled green color-diff (a)
        .asg            A8,         A_gt_       ; Scaled green color-diff (b)
        .asg            A6,         A_gt        ; Scaled green color-diff
        .asg            A9,         A_rt        ; Scaled red color-diff
        .asg            B1,         B_r1        ; Pixel 1 red  (16Q16)
        .asg            B3,         B_g1        ; Pixel 1 grn  (17Q15)
        .asg            B4,         B_b1        ; Pixel 1 blu  (16Q16)
        .asg            A3,         A_r0        ; Pixel 0 red  (16Q16)
        .asg            A5,         A_g0        ; Pixel 0 grn  (17Q15)
        .asg            A0,         A_b0        ; Pixel 0 blu  (16Q16)
        .asg            B5,         B_r1s       ; Saturated pixel 1 red (5Q27)
        .asg            B4,         B_g1s       ; Saturated pixel 1 grn (6Q26)
        .asg            B5,         B_b1s       ; Saturated pixel 1 blu (5Q27)
        .asg            A1,         A_r0s       ; Saturated pixel 0 red (5Q27)
        .asg            A4,         A_g0s       ; Saturated pixel 0 grn (6Q26)
        .asg            A4,         A_b0s       ; Saturated pixel 0 blu (5Q27)
        .asg            B8,         B_r1t       ; Truncated pixel 1 red
        .asg            B7,         B_g1t       ; Truncated pixel 1 grn
        .asg            B2,         B_b1t       ; Truncated pixel 1 blu
        .asg            A7,         A_r0t       ; Truncated pixel 0 red
        .asg            A4,         A_g0t       ; Truncated pixel 0 grn
        .asg            A5,         A_b0t       ; Truncated pixel 0 blu
        .asg            B2,         B_g1f       ; Pixel 1 grn in final position
        .asg            B1,         B_b1f       ; Pixel 1 blu in final position
        .asg            B8,         B_r_b1      ; Pixel 1 red, blue merged
        .asg            B4,         B_rgb1      ; Pixel 1 red, grn, blu merged
        .asg            A3,         A_g0f       ; Pixel 0 grn in final position
        .asg            A6,         A_b0f       ; Pixel 0 blu in final position
        .asg            A7,         A_r_b0      ; Pixel 0 red, blue merged
        .asg            A9,         A_rgb0_     ; Pixel 0 red, grn, blu merged
        .asg            A6,         A_rgb0      ; Pixel 0 in low half word
        .asg            B5,         B_rgb_      ; Combined pixels pre-sign-fix
        .asg            B7,         B_rgb       ; Combined pixels w/ sign-fix
; ========================================================================= ;
        ; Stack frame.  14 words:  A10..A15, B10..B14, B3, CSR, IRP
;-
        STW     .D2T1   A15,        *B_SP--[14]         ; Save A15, get stack
||      MVC     .S2     CSR,        B_csr               ; Capture CSR's state
||      MV      .L2X    A_coef,     B_coef              ; Twin coef pointer
||      MVK     .S1     0xFFFF8410, A_sflip             ; Sign-flip cst, low

        MV      .S1X    B_SP,       A_SP                ; Twin Stack Pointer
||      AND     .L2     B_csr,      -2,         B_noGIE ; Clear GIE
||      LDHU    .D1T2   *A_coef[2], B_gcb_              ; gcb = coeff[2]
||      LDHU    .D2T1   *B_coef[3], A_gcr_              ; gcb = coeff[3]
;-
        STW     .D1T1   A14,        *+A_SP[12]          ; Save A14
||      STW     .D2T2   B14,        *+B_SP[11]          ; Save B14
||      MVC     .S2     B_noGIE,    CSR                 ; Disable interrupts
||      ZERO    .L1     A_ms6                           ; Mask 6, low
; ===== Interrupts masked here =====
        STW     .D1T1   A13,        *+A_SP[10]          ; Save A13
||      STW     .D2T2   B13,        *+B_SP[ 9]          ; Save B13
||      MVC     .S2     IRP,        B_irp               ; Capture IRP's state
||      ZERO    .L2     B_ms5                           ; Mask 5, low
;-
        STW     .D1T1   A12,        *+A_SP[ 8]          ; Save A12
||      STW     .D2T2   B12,        *+B_SP[ 7]          ; Save B12
||      MVC     .S2     B_SP,       IRP                 ; Save SP in IRP
||      MVKLH   .S1     0xFC00,     A_ms6               ; Mask 6, high

        LDH     .D1T1   *A_coef[0], A_one_lum           ; lum = coeff[0]
||      MV      .L1X    B_csr,      A_csr               ; Partitioning MV

        STW     .D1T1   A11,        *+A_SP[ 6]          ; Save A11
||      STW     .D2T2   B11,        *+B_SP[ 5]          ; Save B11
;-
        LDHU    .D2T1   *B_coef[1], A_rcr               ; rcr = coeff[1]
||      LDHU    .D1T2   *A_coef[4], B_bcb               ; rcr = coeff[2]

        STW     .D1T1   A10,        *+A_SP[ 4]          ; Save A10
||      STW     .D2T2   B10,        *+B_SP[ 3]          ; Save B10
||      MV      .L1X    B_y_data,   A_y_ptr             ; Partitioning MV

        STW     .D1T1   A_csr,      *+A_SP[ 2]          ; Save CSR
||      STW     .D2T2   B_ret,      *+B_SP[ 1]          ; Save return address
||      MVK     .S2     128,        B_k32_k128          ; Constant: 128
;-
; =========================== PIPE LOOP PROLOG ============================ ;
        LDBU    .D2T1   *B_cr_ptr++,            A_cr_   ;[ 1,1] cr = *cr_ptr++
||      AND     .L1X    B_num_pix,  -2,         A_i     ; Make num_pix even
||      MV      .L2X    A_cb_data,  B_cb_ptr            ; Partitioning MV
||      MVKLH   .S1     1,          A_one_lum           ; Constant: 1
||      MVKLH   .S2     32,         B_k32_k128          ; Constant: 32
||      MPY     .M2     B_k32_k128, 1,          B_p     ; Prolog collapse count
||      MPYH    .M1     A_one_lum,  A_one_lum,  A_lneg  ; lneg = coeff[0] < 0
;-
        LDBU    .D1T1   *A_y_ptr++[2],          A_y0    ;[ 2,1] y0 = *y_ptr++
||      SHL     .S2X    A_one_lum,  4,          B_y_bias; ((128<<13)+16*luma)
||      MVKH    .S1     0x84108410, A_sflip             ; Sign-flip cst, high

        LDBU    .D2T2   *B_cb_ptr++,            B_cb_   ;[ 3,1] cb = *cb_ptr++
||      ADD     .D1     A_i,        2,          A_i     ; Adjust for para iter
||      SHL     .S1     A_lneg,     20,         A_lneg  ; Handle luma < 0
||      MV      .L1X    B_k32_k128, A_k32_k128          ; Twin constant reg.
||      MV      .L2X    A_rgb_data, B_rgb_ptr           ; Partitioning MV
;-
        LDBU    .D1T2   *-A_y_ptr[1],           B_y1    ;[ 4,1] y1 = *y_ptr++
||      SHL     .S1     A_gcr_,     16,         A_gcr   ; Put gcr in high half
||      SHL     .S2     B_gcb_,     16,         B_gcb   ; Put gcb in high half
||      SUB     .L2X    B_y_bias,   A_lneg,     B_y_bias; Sign bit, coeff[0]<0

        STW     .D1T2   B_irp,      *+A_SP[13]          ; Save IRP
||      ADD     .L1     A_gcr,      A_rcr,      A_gcr_rcr ; Merge gcr, rcr
||      ADD     .L2     B_gcb,      B_bcb,      B_gcb_bcb ; Merge gcb, rcb
||      MVKLH   .S2     0xF800,     B_ms5               ; Mask 5, high
;-
; =========================== PIPE LOOP KERNEL ============================ ;
conv_loop:
  [ A_i]B       .S1     conv_loop                       ;[24,1] while (i)
||      ADD     .L2X    B_rgb1,     A_rgb0,     B_rgb_  ;[24,1] merge pix 0, 1
||      MPYHUS  .M1X    A_g0t,      B_ms5,      A_g0f   ;[18,2] >> 5 and negate
||      SSHL    .S2     B_g1,       11,         B_g1s   ;[18,2] g1s = sat(g1)
||      ADD     .D1     A_y0t,      A_rt,       A_r0    ;[12,3] r0  = y0t + rt
||      SUB     .D2     B_y1t_,     B_y_bias,   B_y1t   ;[12,3] y1t-= y_bias
||      SUB     .L1     A_cr_,      A_k32_k128, A_cr    ;[ 6,4] cr -= 128
||      MPYUS   .M2     B_p,        8,          B_p     ; prolog collapse count
;-
        ADD     .D1     A_r0t,      A_b0f,      A_r_b0  ;[19,2] Merge r0, b0
||      MPYHU   .M2     B_b1t,      B_k32_k128, B_b1f   ;[19,2] >> 11
||      AND     .S2X    B_g1s,      A_ms6,      B_g1t   ;[19,2] g1t = g1s & ms6
||      SSHL    .S1     A_r0,       11,         A_r0s   ;[13,3] r0s = sat(r0)
||      ADD     .L2     B_y1t,      B_bt,       B_b1    ;[13,3] b1  = y1t + bt
||      ADD     .L1X    B_gt_,      A_gt_,      A_gt    ;[13,3]gt=gcr*cr+gcb*cb
||      MPYLH   .M1     A_cr,       A_gcr_rcr,  A_gt_   ;[ 7,4] gcr *c r
||      LDBU    .D2T1   *B_cr_ptr++,            A_cr_   ;[ 1,5] cr  = *cr_ptr++
;-
        XOR     .L2X    B_rgb_,     A_sflip,    B_rgb   ;[26,1] Fix sign bits
||      MPYHUS  .M2     B_g1t,      B_ms5,      B_g1f   ;[20,2] >> 5 and negate
||      SSHL    .S2     B_b1,       11,         B_b1s   ;[14,3] b1s = sat(b1)
||      ADD     .L1X    A_y0t,      B_bt,       A_b0    ;[14,3] b0  = y0t + bt
||      ADD     .S1     A_y0t,      A_gt,       A_g0    ;[14,3] g0  = y0t + gt
||      MPY     .M1     A_y0,       A_one_lum,  A_y0t_  ;[ 8,4] y0t = y0 * luma
||      SUB     .D2     B_cb_,      B_k32_k128, B_cb    ;[ 8,4] cb -= 128
||      LDBU    .D1T1   *A_y_ptr++[2],          A_y0    ;[ 2,5] y0  = *y_ptr++
;-
        SUB     .D1     A_r_b0,     A_g0f,      A_rgb0_ ;[21,2] merge r0,g0,b0
||      ADD     .L2     B_r1t,      B_b1f,      B_r_b1  ;[21,2] merge r1, b1
||      AND     .L1X    A_r0s,      B_ms5,      A_r0t   ;[15,3] r0s = r0t & ms5
||      SSHL    .S1     A_b0,       11,         A_b0s   ;[15,3] b0s = sat(b0)
||      ADD     .S2X    B_y1t,      A_rt,       B_r1    ;[15,3] r1  = y1t + rt
||      MPY     .M1     A_cr,       A_gcr_rcr,  A_rt    ;[ 9,4] rt  = rcr * cr
||      MPYLH   .M2     B_cb,       B_gcb_bcb,  B_gt_   ;[ 9,4] gcb * cb
||      LDBU    .D2T2   *B_cb_ptr++,            B_cb_   ;[ 3,5] cb  = *cb_ptr++
;-
        MPYHU   .M1     A_rgb0_,    A_one_lum,  A_rgb0  ;[22,2] rgb0 in lo half
||      SUB     .D2     B_r_b1,     B_g1f,      B_rgb1  ;[22,2] merge r1,g1,b1
||      AND     .L1X    A_b0s,      B_ms5,      A_b0t   ;[16,3] b0t = b0s & ms5
||      AND     .L2     B_b1s,      B_ms5,      B_b1t   ;[16,3] b1t = b1s & ms5
||      SSHL    .S1     A_g0,       11,         A_g0s   ;[16,3] g0s = sat(g0)
||      SSHL    .S2     B_r1,       11,         B_r1s   ;[16,3] r1s = sat(r1)
||      MPY     .M2X    B_y1,       A_one_lum,  B_y1t_  ;[10,4] y1t = y1 * luma
||      LDBU    .D1T2   *-A_y_ptr[1],           B_y1    ;[ 4,5] y1  = *y_ptr++
;-
  [!B_p]STW     .D2T2   B_rgb,      *B_rgb_ptr++        ;[29,1] *rgb_ptr++=rgb
||      SUB     .D1     A_i,        2,          A_i     ;[23,2] i -= 2
||      MPYHU   .M1     A_b0t,      A_k32_k128, A_b0f   ;[17,3] >> 11
||      AND     .L1     A_g0s,      A_ms6,      A_g0t   ;[17,3] g0t = g0s & ms6
||      AND     .L2     B_r1s,      B_ms5,      B_r1t   ;[17,3] r1t = r1s & ms5
||      ADD     .S2X    B_y1t,      A_gt,       B_g1    ;[17,3] g1  = y1t + gt
||      MPY     .M2     B_cb,       B_gcb_bcb,  B_bt    ;[11,4] bt  = bcb * cb
||      SUB     .S1X    A_y0t_,     B_y_bias,   A_y0t   ;[11,4] y0t-= y_bias
; =========================== PIPE LOOP EPILOG ============================ ;

; ================ SYMBOLIC REGISTER ASSIGNMENTS: CLEANUP ================= ;
        .asg            B15,        B_SP                ; Stack ptr, B side
        .asg            A3,         A_SP                ; Stack ptr, A side
        .asg            A0,         A_csr               ; CSR value
        .asg            B0,         B_irp               ; IRP value
        .asg            B3,         B_ret               ; Return address
; ========================================================================= ;
;-
        MVC     .S2     IRP,        B_SP                ; Restore stack ptr
||      ADD     .L2X    B_rgb1,     A_rgb0,     B_rgb_  ;[24,5] merge pix 0, 1

        MV      .L1X    B_SP,       A_SP                ; Twin Stack Pointer
||      LDW     .D2T2   *+B_SP[13], B_irp               ; Get IRP's value

        LDW     .D1T2   *+A_SP[ 1], B_ret               ; Get return address
||      LDW     .D2T1   *+B_SP[ 2], A_csr               ; Get CSR's value

        LDW     .D1T2   *+A_SP[ 3], B10                 ; Restore B10
||      LDW     .D2T1   *+B_SP[ 4], A10                 ; Restore A10
;-
        LDW     .D1T2   *+A_SP[11], B14                 ; Restore B14
||      LDW     .D2T1   *+B_SP[12], A14                 ; Restore A14
||      XOR     .L2X    B_rgb_,     A_sflip,    B_rgb   ;[26,5] fix sign bits

        LDW     .D1T2   *+A_SP[ 7], B12                 ; Restore B12
||      LDW     .D2T1   *+B_SP[ 8], A12                 ; Restore A12

        LDW     .D1T2   *+A_SP[ 9], B13                 ; Restore B13
||      LDW     .D2T1   *+B_SP[10], A13                 ; Restore A13
||      MVC     .S2     B_irp,      IRP                 ; Restore IRP
;-
        LDW     .D1T2   *+A_SP[ 5], B11                 ; Restore B11
||      LDW     .D2T1   *+B_SP[ 6], A11                 ; Restore A11
||      B       .S2     B_ret                           ; Return to caller

        MVC     .S2X    A_csr,      CSR                 ; Restore CSR
||      LDW     .D2T1   *++B_SP[14],A15                 ; Restore A15
; ===== Interruptibility state (GIE) restored here =====

        STW     .D2T2   B_rgb,      *B_rgb_ptr          ;[29,5] *rgb_ptr++=rgb

        NOP             3

; ===== Branch occurs =====
; ===== Interrupts may occur here =====

* ========================================================================= *
*   End of file:  ycbcr422pl_to_rgb565_h.asm                                *
* ------------------------------------------------------------------------- *
*             Copyright (c) 1999 Texas Instruments, Incorporated.           *
*                            All Rights Reserved.                           *
* ========================================================================= *
