/* This file has a different license to the rest of the uGFX system.
 * You can copy, modify and distribute this file as you see fit.
 * You do not need to publish your source modifications to this file.
 * The only thing you are not permitted to do is to relicense it
 * under a different license.
 */

/**
 * Copy this file into your project directory and rename it as gfxconf.h
 * Edit your copy to turn on the uGFX features you want to use.
 * The values below are the defaults.
 *
 * Only remove the comments from lines where you want to change the
 * default value. This allows definitions to be included from
 * driver makefiles when required and provides the best future
 * compatibility for your project.
 *
 * Please use spaces instead of tabs in this file.
 */

#ifndef _GFXCONF_H
#define _GFXCONF_H

#define GDISP_USE_DMA                               TRUE
///////////////////////////////////////////////////////////////////////////
// GOS - One of these must be defined, preferably in your Makefile       //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_OS_CHIBIOS                           TRUE


///////////////////////////////////////////////////////////////////////////
// GDISP                                                                 //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GDISP                               TRUE
//#define GDISP_NEED_AUTOFLUSH                         FALSE
//#define GDISP_NEED_TIMERFLUSH                        FALSE
#define GDISP_NEED_VALIDATION                        TRUE
#define GDISP_NEED_CLIP                              TRUE
//#define GDISP_NEED_CIRCLE                            FALSE
//#define GDISP_NEED_ELLIPSE                           FALSE
//#define GDISP_NEED_ARC                               FALSE
//#define GDISP_NEED_CONVEX_POLYGON                    FALSE
//#define GDISP_NEED_SCROLL                            FALSE
//#define GDISP_NEED_PIXELREAD                         FALSE
#define GDISP_NEED_CONTROL                           TRUE
//#define GDISP_NEED_QUERY                             FALSE
#define GDISP_NEED_MULTITHREAD                       TRUE
//#define GDISP_NEED_STREAMING                         FALSE
#define GDISP_NEED_TEXT                              	TRUE
#define GDISP_NEED_ANTIALIAS                     		FALSE
#define GDISP_NEED_UTF8                          		TRUE
//    #define GDISP_NEED_TEXT_KERNING                  FALSE
#define GDISP_INCLUDE_FONT_UI1                   		TRUE
#define GDISP_INCLUDE_FONT_UI2                   		TRUE		// The smallest preferred font.
#define GDISP_INCLUDE_FONT_LARGENUMBERS          		TRUE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS10          FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS12          FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS16          FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS24          FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS32          FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANSBOLD12      FALSE
//    #define GDISP_INCLUDE_FONT_FIXED_10X20           FALSE
//    #define GDISP_INCLUDE_FONT_FIXED_7X14            FALSE
#define GDISP_INCLUDE_FONT_FIXED_5X8             		TRUE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS12_AA       FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS16_AA       FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS24_AA       FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANS32_AA       FALSE
//    #define GDISP_INCLUDE_FONT_DEJAVUSANSBOLD12_AA   FALSE
//    #define GDISP_INCLUDE_USER_FONTS                 FALSE
#define GDISP_DEFAULT_ORIENTATION                    	GDISP_ROTATE_90

#define GDISP_NEED_IMAGE                             	TRUE
//#define GDISP_NEED_IMAGE_NATIVE                  		TRUE
//    #define GDISP_NEED_IMAGE_GIF                     FALSE
#define GDISP_NEED_IMAGE_BMP                     		TRUE
//        #define GDISP_NEED_IMAGE_BMP_1               FALSE
//        #define GDISP_NEED_IMAGE_BMP_4               FALSE
//        #define GDISP_NEED_IMAGE_BMP_4_RLE           FALSE
//        #define GDISP_NEED_IMAGE_BMP_8               FALSE
//        #define GDISP_NEED_IMAGE_BMP_8_RLE           FALSE
#define GDISP_NEED_IMAGE_BMP_16              			TRUE
//        #define GDISP_NEED_IMAGE_BMP_24              FALSE
//        #define GDISP_NEED_IMAGE_BMP_32              FALSE
//    #define GDISP_NEED_IMAGE_JPG                     FALSE
//#define GDISP_NEED_IMAGE_PNG                     		TRUE
//    #define GDISP_NEED_IMAGE_ACCOUNTING              FALSE

#define GDISP_NEED_STARTUP_LOGO                      FALSE

//#define GDISP_DEFAULT_ORIENTATION                    GDISP_ROTATE_LANDSCAPE    // If not defined the native hardware orientation is used.
//#define GDISP_LINEBUF_SIZE                           128

//#define GDISP_USE_GFXNET                             FALSE
//    #define GDISP_GFXNET_PORT                        13001
//    #define GDISP_GFXNET_CUSTOM_LWIP_STARTUP         FALSE
//    #define GDISP_DONT_WAIT_FOR_NET_DISPLAY          FALSE
//    #define GDISP_GFXNET_UNSAFE_SOCKETS              FALSE


///////////////////////////////////////////////////////////////////////////
// GWIN                                                                  //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GWIN                                 	TRUE

#define GWIN_NEED_WINDOWMANAGER                      	TRUE

#define GWIN_NEED_CONSOLE                            	TRUE
//    #define GWIN_CONSOLE_USE_HISTORY                 FALSE
//        #define GWIN_CONSOLE_HISTORY_AVERAGING       FALSE
//        #define GWIN_CONSOLE_HISTORY_ATCREATE        FALSE
//    #define GWIN_CONSOLE_ESCSEQ                      FALSE
//    #define GWIN_CONSOLE_USE_BASESTREAM              FALSE
//    #define GWIN_CONSOLE_USE_FLOAT                   FALSE
//#define GWIN_NEED_GRAPH                              FALSE

#define GWIN_NEED_WIDGET                             	TRUE
#define GWIN_NEED_LABEL                          		TRUE
//        #define GWIN_LABEL_ATTRIBUTE                 FALSE
#define GWIN_NEED_BUTTON                         		TRUE
#define GWIN_BUTTON_LAZY_RELEASE             			TRUE
#define GWIN_NEED_SLIDER                         		TRUE
//    #define GWIN_NEED_CHECKBOX                       FALSE
#define GWIN_NEED_IMAGE                          		TRUE
//        #define GWIN_NEED_IMAGE_ANIMATION            FALSE
//    #define GWIN_NEED_RADIO                          FALSE
//    #define GWIN_NEED_LIST                           FALSE
//        #define GWIN_NEED_LIST_IMAGES                FALSE
//    #define GWIN_NEED_PROGRESSBAR                    FALSE
//        #define GWIN_PROGRESSBAR_AUTO                FALSE
//    #define GWIN_FLAT_STYLING                        FALSE

//#define GWIN_NEED_CONTAINERS                         FALSE
//    #define GWIN_NEED_CONTAINER                      FALSE
//    #define GWIN_NEED_FRAME                          FALSE


///////////////////////////////////////////////////////////////////////////
// GEVENT                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GEVENT                               TRUE

//#define GEVENT_ASSERT_NO_RESOURCE                    FALSE
//#define GEVENT_MAXIMUM_SIZE                          32
//#define GEVENT_MAX_SOURCE_LISTENERS                  32


///////////////////////////////////////////////////////////////////////////
// GTIMER                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GTIMER                               TRUE

//#define GTIMER_THREAD_PRIORITY                       HIGH_PRIORITY
//#define GTIMER_THREAD_WORKAREA_SIZE                  2048


///////////////////////////////////////////////////////////////////////////
// GQUEUE                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GQUEUE                               	TRUE

#define GQUEUE_NEED_ASYNC                            	TRUE
//#define GQUEUE_NEED_GSYNC                            FALSE
//#define GQUEUE_NEED_FSYNC                            FALSE
//#define GQUEUE_NEED_BUFFERS                          FALSE

///////////////////////////////////////////////////////////////////////////
// GINPUT                                                                //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GINPUT                               	TRUE

#define GINPUT_NEED_MOUSE                            	TRUE
//#define GINPUT_TOUCH_STARTRAW							TRUE
//#define GINPUT_TOUCH_NOCALIBRATE_GUI             		FALSE
//#define GINPUT_TOUCH_USER_CALIBRATION_SAVE				TRUE

///////////////////////////////////////////////////////////////////////////
// GFILE                                                                 //
///////////////////////////////////////////////////////////////////////////
#define GFX_USE_GFILE                                	TRUE

//#define GFILE_NEED_PRINTG                            FALSE
//#define GFILE_NEED_SCANG                             FALSE
//#define GFILE_NEED_STRINGS                           FALSE
//#define GFILE_NEED_STDIO                             FALSE
//    #define GFILE_ALLOW_FLOATS                       FALSE
#define GFILE_ALLOW_DEVICESPECIFIC               		TRUE
//    #define GFILE_MAX_GFILES                         3

#define GFILE_NEED_MEMFS                             	TRUE
#define GFILE_NEED_ROMFS                             	TRUE
//#define GFILE_NEED_RAMFS                             FALSE
//#define GFILE_NEED_FATFS                             FALSE
//#define GFILE_NEED_NATIVEFS                          	TRUE
//#define GFILE_NEED_CHBIOSFS                          FALSE


///////////////////////////////////////////////////////////////////////////
// GADC                                                                  //
///////////////////////////////////////////////////////////////////////////
//#define GFX_USE_GADC                                 FALSE

//#define GADC_MAX_LOWSPEED_DEVICES                    4


///////////////////////////////////////////////////////////////////////////
// GAUDIO                                                                //
///////////////////////////////////////////////////////////////////////////
//#define GFX_USE_GAUDIO                               FALSE
//    #define GAUDIO_NEED_PLAY                         FALSE
//    #define GAUDIO_NEED_RECORD                       FALSE


///////////////////////////////////////////////////////////////////////////
// GMISC                                                                 //
///////////////////////////////////////////////////////////////////////////
//#define GFX_USE_GMISC                                FALSE

//#define GMISC_NEED_ARRAYOPS                          FALSE
//#define GMISC_NEED_FASTTRIG                          FALSE
//#define GMISC_NEED_FIXEDTRIG                         FALSE
//#define GMISC_NEED_INVSQRT                           FALSE
//    #define GMISC_INVSQRT_MIXED_ENDIAN               FALSE
//    #define GMISC_INVSQRT_REAL_SLOW                  FALSE

#endif /* _GFXCONF_H */
