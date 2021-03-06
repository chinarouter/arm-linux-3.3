

// =================== defines ===================
#define     ErrMsg_001  (0x00000001)    // 001: Input buffer size in GM_Init() <= 0
#define     ErrMsg_002  (0x00000002)    // 002: Input buffer size in GM_Init() should be multiple of 128
#define     ErrMsg_003  (0x00000003)    // 003: Input buffer size in GM_Init() should be great than 256
#define     ErrMsg_004  (0x00000004)    // 004:

#define NR_FRM_SZ_256		(256)
#define NR_FRM_OV_SZ_128	(128)

#define NR_FFT_SZ_256		(NR_FRM_SZ_256)

//#define swap(x,y)	(x)^=(y);(y)^=(x);(x)^=(y)
//#define abs(x)  	((x) < 0 ? -(x) : (x))  // get absolute value
#define sign(v)		((v) > 0 ? 1 : ((v) < 0 ? -1 : 0))

// Matlab Code:
//		A = round(hamming(256)*32767);
//		CStyle(floor(0.98*(max(A)/(max(A) + min(A)))*A));
const short hamming256[256] = {		// < Qu15, ONLY for Spectrum Subtraction overlap add
     2378,    2382,    2395,    2416,    2445,    2482,    2528,    2581,
    2643,    2713,    2792,    2878,    2971,    3074,    3184,    3302,
    3428,    3560,    3702,    3850,    4006,    4169,    4339,    4517,
    4701,    4892,    5090,    5295,    5507,    5723,    5948,    6178,
    6414,    6656,    6903,    7157,    7416,    7679,    7948,    8222,
    8500,    8784,    9072,    9363,    9659,    9959,   10262,   10569,
   10879,   11193,   11510,   11829,   12151,   12475,   12801,   13130,
   13459,   13791,   14124,   14458,   14793,   15130,   15465,   15803,
   16140,   16476,   16813,   17150,   17485,   17820,   18153,   18485,
   18817,   19146,   19474,   19798,   20121,   20442,   20759,   21075,
   21386,   21696,   22001,   22302,   22600,   22894,   23184,   23469,
   23750,   24026,   24298,   24564,   24825,   25081,   25332,   25577,
   25815,   26049,   26276,   26497,   26711,   26919,   27120,   27315,
   27503,   27684,   27858,   28025,   28184,   28336,   28481,   28618,
   28747,   28869,   28983,   29089,   29187,   29277,   29360,   29434,
   29500,   29558,   29608,   29648,   29682,   29706,   29724,   29732,
   29732,   29724,   29706,   29682,   29648,   29608,   29558,   29500,
   29434,   29360,   29277,   29187,   29089,   28983,   28869,   28747,
   28618,   28481,   28336,   28184,   28025,   27858,   27684,   27503,
   27315,   27120,   26919,   26711,   26497,   26276,   26049,   25815,
   25577,   25332,   25081,   24825,   24564,   24298,   24026,   23750,
   23469,   23184,   22894,   22600,   22302,   22001,   21696,   21386,
   21075,   20759,   20442,   20121,   19798,   19474,   19146,   18817,
   18485,   18153,   17820,   17485,   17150,   16813,   16476,   16140,
   15803,   15465,   15130,   14793,   14458,   14124,   13791,   13459,
   13130,   12801,   12475,   12151,   11829,   11510,   11193,   10879,
   10569,   10262,    9959,    9659,    9363,    9072,    8784,    8500,
    8222,    7948,    7679,    7416,    7157,    6903,    6656,    6414,
    6178,    5948,    5723,    5507,    5295,    5090,    4892,    4701,
    4517,    4339,    4169,    4006,    3850,    3702,    3560,    3428,
    3302,    3184,    3074,    2971,    2878,    2792,    2713,    2643,
    2581,    2528,    2482,    2445,    2416,    2395,    2382,    2378 };
