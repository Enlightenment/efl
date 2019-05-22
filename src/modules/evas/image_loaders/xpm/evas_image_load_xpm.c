#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

static int _evas_loader_xpm_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_loader_xpm_log_dom, __VA_ARGS__)

typedef struct
{
   unsigned short offset;
   unsigned char r, g, b;
} Color_Index;

static const Color_Index color_name_index[] =
{
{   0,  240, 248, 255},
{  11,  240, 248, 255},
{  21,  255, 239, 219},
{  35,  238, 223, 204},
{  49,  250, 235, 215},
{  63,  250, 235, 215},
{  76,  205, 192, 176},
{  90,  139, 131, 120},
{ 104,    0, 255, 255},
{ 109,  127, 255, 212},
{ 121,  127, 255, 212},
{ 132,  118, 238, 198},
{ 144,  102, 205, 170},
{ 156,   69, 139, 116},
{ 168,  240, 255, 255},
{ 175,  224, 238, 238},
{ 182,  240, 255, 255},
{ 188,  193, 205, 205},
{ 195,  131, 139, 139},
{ 202,  245, 245, 220},
{ 208,  255, 228, 196},
{ 216,  238, 213, 183},
{ 224,  255, 228, 196},
{ 231,  205, 183, 158},
{ 239,  139, 125, 107},
{ 247,    0,   0,   0},
{ 253,  255, 235, 205},
{ 269,  255, 235, 205},
{ 284,    0,   0, 255},
{ 289,    0,   0, 255},
{ 295,    0,   0, 238},
{ 301,    0,   0, 205},
{ 307,    0,   0, 139},
{ 313,  138,  43, 226},
{ 325,  138,  43, 226},
{ 336,  255,  64,  64},
{ 343,  165,  42,  42},
{ 349,  238,  59,  59},
{ 356,  205,  51,  51},
{ 363,  139,  35,  35},
{ 370,  255, 211, 155},
{ 381,  222, 184, 135},
{ 391,  238, 197, 145},
{ 402,  205, 170, 125},
{ 413,  139, 115,  85},
{ 424,  152, 245, 255},
{ 435,  142, 229, 238},
{ 446,  122, 197, 205},
{ 457,   83, 134, 139},
{ 468,   95, 158, 160},
{ 479,   95, 158, 160},
{ 489,  127, 255,   0},
{ 501,  127, 255,   0},
{ 512,  118, 238,   0},
{ 524,  102, 205,   0},
{ 536,   69, 139,   0},
{ 548,  255, 127,  36},
{ 559,  210, 105,  30},
{ 569,  238, 118,  33},
{ 580,  205, 102,  29},
{ 591,  139,  69,  19},
{ 602,  255, 114,  86},
{ 609,  238, 106,  80},
{ 616,  255, 127,  80},
{ 622,  205,  91,  69},
{ 629,  139,  62,  47},
{ 636,  100, 149, 237},
{ 652,  100, 149, 237},
{ 667,  255, 248, 220},
{ 677,  238, 232, 205},
{ 687,  255, 248, 220},
{ 696,  205, 200, 177},
{ 706,  139, 136, 120},
{ 716,  220,  20,  60},
{ 724,    0, 255, 255},
{ 729,    0, 255, 255},
{ 735,    0, 238, 238},
{ 741,    0, 205, 205},
{ 747,    0, 139, 139},
{ 753,    0,   0, 139},
{ 763,    0,   0, 139},
{ 772,    0, 139, 139},
{ 782,    0, 139, 139},
{ 791,  255, 185,  15},
{ 806,  184, 134,  11},
{ 821,  184, 134,  11},
{ 835,  238, 173,  14},
{ 850,  205, 149,  12},
{ 865,  139, 101,   8},
{ 880,  169, 169, 169},
{ 890,  169, 169, 169},
{ 899,    0, 100,   0},
{ 910,    0, 100,   0},
{ 920,  169, 169, 169},
{ 930,  169, 169, 169},
{ 939,  189, 183, 107},
{ 950,  189, 183, 107},
{ 960,  139,   0, 139},
{ 973,  139,   0, 139},
{ 985,  202, 255, 112},
{1001,  188, 238, 104},
{1017,  162, 205,  90},
{1033,  110, 139,  61},
{1049,   85, 107,  47},
{1066,   85, 107,  47},
{1081,  255, 127,   0},
{1093,  238, 118,   0},
{1105,  255, 140,   0},
{1117,  255, 140,   0},
{1128,  205, 102,   0},
{1140,  139,  69,   0},
{1152,  191,  62, 255},
{1164,  153,  50, 204},
{1176,  153,  50, 204},
{1187,  178,  58, 238},
{1199,  154,  50, 205},
{1211,  104,  34, 139},
{1223,  139,   0,   0},
{1232,  139,   0,   0},
{1240,  233, 150, 122},
{1252,  233, 150, 122},
{1263,  193, 255, 193},
{1277,  143, 188, 143},
{1292,  143, 188, 143},
{1305,  180, 238, 180},
{1319,  155, 205, 155},
{1333,  105, 139, 105},
{1347,   72,  61, 139},
{1363,   72,  61, 139},
{1377,  151, 255, 255},
{1392,  141, 238, 238},
{1407,  121, 205, 205},
{1422,   47,  79,  79},
{1438,   47,  79,  79},
{1452,   82, 139, 139},
{1467,   47,  79,  79},
{1483,   47,  79,  79},
{1497,    0, 206, 209},
{1512,    0, 206, 209},
{1526,  148,   0, 211},
{1538,  148,   0, 211},
{1549,  255,  20, 147},
{1559,  238,  18, 137},
{1569,  255,  20, 147},
{1579,  255,  20, 147},
{1588,  205,  16, 118},
{1598,  139,  10,  80},
{1608,    0, 191, 255},
{1622,    0, 191, 255},
{1634,    0, 191, 255},
{1647,    0, 178, 238},
{1660,    0, 154, 205},
{1673,    0, 104, 139},
{1686,  105, 105, 105},
{1695,  105, 105, 105},
{1703,  105, 105, 105},
{1712,  105, 105, 105},
{1720,   30, 144, 255},
{1732,   28, 134, 238},
{1744,   30, 144, 255},
{1756,   30, 144, 255},
{1767,   24, 116, 205},
{1779,   16,  78, 139},
{1791,  255,  48,  48},
{1802,  178,  34,  34},
{1812,  238,  44,  44},
{1823,  205,  38,  38},
{1834,  139,  26,  26},
{1845,  255, 250, 240},
{1858,  255, 250, 240},
{1870,   34, 139,  34},
{1883,   34, 139,  34},
{1895,  255,   0, 255},
{1903,  220, 220, 220},
{1913,  248, 248, 255},
{1925,  248, 248, 255},
{1936,  255, 215,   0},
{1942,  238, 201,   0},
{1948,  255, 215,   0},
{1953,  205, 173,   0},
{1959,  139, 117,   0},
{1965,  255, 193,  37},
{1976,  218, 165,  32},
{1986,  238, 180,  34},
{1997,  205, 155,  29},
{2008,  139, 105,  20},
{2019,    0,   0,   0},
{2025,  255, 255, 255},
{2033,   26,  26,  26},
{2040,   28,  28,  28},
{2047,   31,  31,  31},
{2054,    3,   3,   3},
{2060,   33,  33,  33},
{2067,   36,  36,  36},
{2074,   38,  38,  38},
{2081,   41,  41,  41},
{2088,   43,  43,  43},
{2095,   46,  46,  46},
{2102,  190, 190, 190},
{2107,   48,  48,  48},
{2114,   51,  51,  51},
{2121,   54,  54,  54},
{2128,   56,  56,  56},
{2135,   59,  59,  59},
{2142,   61,  61,  61},
{2149,    5,   5,   5},
{2155,   64,  64,  64},
{2162,   66,  66,  66},
{2169,   69,  69,  69},
{2176,   71,  71,  71},
{2183,   74,  74,  74},
{2190,   77,  77,  77},
{2197,   79,  79,  79},
{2204,   82,  82,  82},
{2211,   84,  84,  84},
{2218,   87,  87,  87},
{2225,   89,  89,  89},
{2232,   92,  92,  92},
{2239,   94,  94,  94},
{2246,    8,   8,   8},
{2252,   97,  97,  97},
{2259,   99,  99,  99},
{2266,  102, 102, 102},
{2273,   10,  10,  10},
{2279,  105, 105, 105},
{2286,  107, 107, 107},
{2293,  110, 110, 110},
{2300,  112, 112, 112},
{2307,  115, 115, 115},
{2314,  117, 117, 117},
{2321,  120, 120, 120},
{2328,  122, 122, 122},
{2335,  125, 125, 125},
{2342,  127, 127, 127},
{2349,  130, 130, 130},
{2356,   13,  13,  13},
{2362,  133, 133, 133},
{2369,  135, 135, 135},
{2376,  138, 138, 138},
{2383,  140, 140, 140},
{2390,  143, 143, 143},
{2397,  145, 145, 145},
{2404,  148, 148, 148},
{2411,  150, 150, 150},
{2418,  153, 153, 153},
{2425,  156, 156, 156},
{2432,   15,  15,  15},
{2438,  158, 158, 158},
{2445,  161, 161, 161},
{2452,  163, 163, 163},
{2459,  166, 166, 166},
{2466,  168, 168, 168},
{2473,  171, 171, 171},
{2480,  173, 173, 173},
{2487,  176, 176, 176},
{2494,  179, 179, 179},
{2501,  181, 181, 181},
{2508,   18,  18,  18},
{2514,  184, 184, 184},
{2521,  186, 186, 186},
{2528,  189, 189, 189},
{2535,  191, 191, 191},
{2542,  194, 194, 194},
{2549,  196, 196, 196},
{2556,  199, 199, 199},
{2563,  201, 201, 201},
{2570,  204, 204, 204},
{2577,  207, 207, 207},
{2584,   20,  20,  20},
{2590,  209, 209, 209},
{2597,  212, 212, 212},
{2604,  214, 214, 214},
{2611,  217, 217, 217},
{2618,  219, 219, 219},
{2625,  222, 222, 222},
{2632,  224, 224, 224},
{2639,  227, 227, 227},
{2646,  229, 229, 229},
{2653,  232, 232, 232},
{2660,  235, 235, 235},
{2667,   23,  23,  23},
{2673,  237, 237, 237},
{2680,  240, 240, 240},
{2687,  242, 242, 242},
{2694,  245, 245, 245},
{2701,  247, 247, 247},
{2708,  250, 250, 250},
{2715,  252, 252, 252},
{2722,    0, 255,   0},
{2728,    0, 255,   0},
{2735,    0, 238,   0},
{2742,    0, 205,   0},
{2749,    0, 139,   0},
{2756,  173, 255,  47},
{2769,  173, 255,  47},
{2781,    0,   0,   0},
{2787,  255, 255, 255},
{2795,   26,  26,  26},
{2802,   28,  28,  28},
{2809,   31,  31,  31},
{2816,    3,   3,   3},
{2822,   33,  33,  33},
{2829,   36,  36,  36},
{2836,   38,  38,  38},
{2843,   41,  41,  41},
{2850,   43,  43,  43},
{2857,   46,  46,  46},
{2864,  190, 190, 190},
{2869,   48,  48,  48},
{2876,   51,  51,  51},
{2883,   54,  54,  54},
{2890,   56,  56,  56},
{2897,   59,  59,  59},
{2904,   61,  61,  61},
{2911,    5,   5,   5},
{2917,   64,  64,  64},
{2924,   66,  66,  66},
{2931,   69,  69,  69},
{2938,   71,  71,  71},
{2945,   74,  74,  74},
{2952,   77,  77,  77},
{2959,   79,  79,  79},
{2966,   82,  82,  82},
{2973,   84,  84,  84},
{2980,   87,  87,  87},
{2987,   89,  89,  89},
{2994,   92,  92,  92},
{3001,   94,  94,  94},
{3008,    8,   8,   8},
{3014,   97,  97,  97},
{3021,   99,  99,  99},
{3028,  102, 102, 102},
{3035,   10,  10,  10},
{3041,  105, 105, 105},
{3048,  107, 107, 107},
{3055,  110, 110, 110},
{3062,  112, 112, 112},
{3069,  115, 115, 115},
{3076,  117, 117, 117},
{3083,  120, 120, 120},
{3090,  122, 122, 122},
{3097,  125, 125, 125},
{3104,  127, 127, 127},
{3111,  130, 130, 130},
{3118,   13,  13,  13},
{3124,  133, 133, 133},
{3131,  135, 135, 135},
{3138,  138, 138, 138},
{3145,  140, 140, 140},
{3152,  143, 143, 143},
{3159,  145, 145, 145},
{3166,  148, 148, 148},
{3173,  150, 150, 150},
{3180,  153, 153, 153},
{3187,  156, 156, 156},
{3194,   15,  15,  15},
{3200,  158, 158, 158},
{3207,  161, 161, 161},
{3214,  163, 163, 163},
{3221,  166, 166, 166},
{3228,  168, 168, 168},
{3235,  171, 171, 171},
{3242,  173, 173, 173},
{3249,  176, 176, 176},
{3256,  179, 179, 179},
{3263,  181, 181, 181},
{3270,   18,  18,  18},
{3276,  184, 184, 184},
{3283,  186, 186, 186},
{3290,  189, 189, 189},
{3297,  191, 191, 191},
{3304,  194, 194, 194},
{3311,  196, 196, 196},
{3318,  199, 199, 199},
{3325,  201, 201, 201},
{3332,  204, 204, 204},
{3339,  207, 207, 207},
{3346,   20,  20,  20},
{3352,  209, 209, 209},
{3359,  212, 212, 212},
{3366,  214, 214, 214},
{3373,  217, 217, 217},
{3380,  219, 219, 219},
{3387,  222, 222, 222},
{3394,  224, 224, 224},
{3401,  227, 227, 227},
{3408,  229, 229, 229},
{3415,  232, 232, 232},
{3422,  235, 235, 235},
{3429,   23,  23,  23},
{3435,  237, 237, 237},
{3442,  240, 240, 240},
{3449,  242, 242, 242},
{3456,  245, 245, 245},
{3463,  247, 247, 247},
{3470,  250, 250, 250},
{3477,  252, 252, 252},
{3484,  240, 255, 240},
{3494,  224, 238, 224},
{3504,  240, 255, 240},
{3513,  193, 205, 193},
{3523,  131, 139, 131},
{3533,  255, 110, 180},
{3542,  238, 106, 167},
{3551,  255, 105, 180},
{3560,  255, 105, 180},
{3568,  205,  96, 144},
{3577,  139,  58,  98},
{3586,  255, 106, 106},
{3597,  205,  92,  92},
{3608,  205,  92,  92},
{3618,  238,  99,  99},
{3629,  205,  85,  85},
{3640,  139,  58,  58},
{3651,   75,   0, 130},
{3658,  255, 255, 240},
{3665,  238, 238, 224},
{3672,  255, 255, 240},
{3678,  205, 205, 193},
{3685,  139, 139, 131},
{3692,  255, 246, 143},
{3699,  238, 230, 133},
{3706,  240, 230, 140},
{3712,  205, 198, 115},
{3719,  139, 134,  78},
{3726,  230, 230, 250},
{3735,  255, 240, 245},
{3750,  238, 224, 229},
{3765,  255, 240, 245},
{3780,  255, 240, 245},
{3794,  205, 193, 197},
{3809,  139, 131, 134},
{3824,  124, 252,   0},
{3835,  124, 252,   0},
{3845,  255, 250, 205},
{3859,  238, 233, 191},
{3873,  255, 250, 205},
{3887,  255, 250, 205},
{3900,  205, 201, 165},
{3914,  139, 137, 112},
{3928,  191, 239, 255},
{3939,  173, 216, 230},
{3950,  173, 216, 230},
{3960,  178, 223, 238},
{3971,  154, 192, 205},
{3982,  104, 131, 139},
{3993,  240, 128, 128},
{4005,  240, 128, 128},
{4016,  224, 255, 255},
{4027,  209, 238, 238},
{4038,  224, 255, 255},
{4049,  224, 255, 255},
{4059,  180, 205, 205},
{4070,  122, 139, 139},
{4081,  255, 236, 139},
{4097,  238, 220, 130},
{4113,  238, 221, 130},
{4129,  238, 221, 130},
{4144,  205, 190, 112},
{4160,  139, 129,  76},
{4176,  250, 250, 210},
{4199,  250, 250, 210},
{4220,  211, 211, 211},
{4231,  211, 211, 211},
{4241,  144, 238, 144},
{4253,  144, 238, 144},
{4264,  211, 211, 211},
{4275,  211, 211, 211},
{4285,  255, 174, 185},
{4296,  238, 162, 173},
{4307,  255, 182, 193},
{4318,  255, 182, 193},
{4328,  205, 140, 149},
{4339,  139,  95, 101},
{4350,  255, 160, 122},
{4363,  238, 149, 114},
{4376,  255, 160, 122},
{4389,  255, 160, 122},
{4401,  205, 129,  98},
{4414,  139,  87,  66},
{4427,   32, 178, 170},
{4443,   32, 178, 170},
{4457,  176, 226, 255},
{4471,  135, 206, 250},
{4486,  135, 206, 250},
{4499,  164, 211, 238},
{4513,  141, 182, 205},
{4527,   96, 123, 139},
{4541,  132, 112, 255},
{4558,  132, 112, 255},
{4573,  119, 136, 153},
{4590,  119, 136, 153},
{4605,  119, 136, 153},
{4622,  119, 136, 153},
{4637,  202, 225, 255},
{4653,  176, 196, 222},
{4670,  176, 196, 222},
{4685,  188, 210, 238},
{4701,  162, 181, 205},
{4717,  110, 123, 139},
{4733,  255, 255, 224},
{4746,  238, 238, 209},
{4759,  255, 255, 224},
{4772,  255, 255, 224},
{4784,  205, 205, 180},
{4797,  139, 139, 122},
{4810,    0, 255,   0},
{4815,   50, 205,  50},
{4826,   50, 205,  50},
{4836,  250, 240, 230},
{4842,  255,   0, 255},
{4851,  238,   0, 238},
{4860,  255,   0, 255},
{4868,  205,   0, 205},
{4877,  139,   0, 139},
{4886,  255,  52, 179},
{4894,  176,  48,  96},
{4901,  238,  48, 167},
{4909,  205,  41, 144},
{4917,  139,  28,  98},
{4925,  102, 205, 170},
{4943,  102, 205, 170},
{4960,    0,   0, 205},
{4972,    0,   0, 205},
{4983,  224, 102, 255},
{4997,  186,  85, 211},
{5011,  186,  85, 211},
{5024,  209,  95, 238},
{5038,  180,  82, 205},
{5052,  122,  55, 139},
{5066,  171, 130, 255},
{5080,  147, 112, 219},
{5094,  147, 112, 219},
{5107,  159, 121, 238},
{5121,  137, 104, 205},
{5135,   93,  71, 139},
{5149,   60, 179, 113},
{5166,   60, 179, 113},
{5181,  123, 104, 238},
{5199,  123, 104, 238},
{5215,    0, 250, 154},
{5235,    0, 250, 154},
{5253,   72, 209, 204},
{5270,   72, 209, 204},
{5286,  199,  21, 133},
{5304,  199,  21, 133},
{5320,   25,  25, 112},
{5334,   25,  25, 112},
{5347,  245, 255, 250},
{5358,  245, 255, 250},
{5368,  255, 228, 225},
{5379,  238, 213, 210},
{5390,  255, 228, 225},
{5401,  255, 228, 225},
{5411,  205, 183, 181},
{5422,  139, 125, 123},
{5433,  255, 228, 181},
{5442,  255, 222, 173},
{5455,  238, 207, 161},
{5468,  255, 222, 173},
{5481,  255, 222, 173},
{5493,  205, 179, 139},
{5506,  139, 121,  94},
{5519,    0,   0, 128},
{5524,    0,   0, 128},
{5534,    0,   0, 128},
{5543,  253, 245, 230},
{5552,  253, 245, 230},
{5560,  128, 128,   0},
{5566,  107, 142,  35},
{5577,  107, 142,  35},
{5587,  192, 255,  62},
{5598,  179, 238,  58},
{5609,  154, 205,  50},
{5620,  105, 139,  34},
{5631,  255, 165,   0},
{5639,  238, 154,   0},
{5647,  255, 165,   0},
{5654,  205, 133,   0},
{5662,  139,  90,   0},
{5670,  255,  69,   0},
{5681,  238,  64,   0},
{5692,  255,  69,   0},
{5703,  255,  69,   0},
{5713,  205,  55,   0},
{5724,  139,  37,   0},
{5735,  255, 131, 250},
{5743,  218, 112, 214},
{5750,  238, 122, 233},
{5758,  205, 105, 201},
{5766,  139,  71, 137},
{5774,  238, 232, 170},
{5789,  238, 232, 170},
{5803,  154, 255, 154},
{5814,  152, 251, 152},
{5825,  152, 251, 152},
{5835,  144, 238, 144},
{5846,  124, 205, 124},
{5857,   84, 139,  84},
{5868,  187, 255, 255},
{5883,  175, 238, 238},
{5898,  175, 238, 238},
{5912,  174, 238, 238},
{5927,  150, 205, 205},
{5942,  102, 139, 139},
{5957,  255, 130, 171},
{5972,  219, 112, 147},
{5988,  219, 112, 147},
{6002,  238, 121, 159},
{6017,  205, 104, 137},
{6032,  139,  71,  93},
{6047,  255, 239, 213},
{6059,  255, 239, 213},
{6070,  255, 218, 185},
{6081,  238, 203, 173},
{6092,  255, 218, 185},
{6103,  255, 218, 185},
{6113,  205, 175, 149},
{6124,  139, 119, 101},
{6135,  205, 133,  63},
{6140,  255, 181, 197},
{6146,  238, 169, 184},
{6152,  255, 192, 203},
{6157,  205, 145, 158},
{6163,  139,  99, 108},
{6169,  255, 187, 255},
{6175,  221, 160, 221},
{6180,  238, 174, 238},
{6186,  205, 150, 205},
{6192,  139, 102, 139},
{6198,  176, 224, 230},
{6210,  176, 224, 230},
{6221,  155,  48, 255},
{6229,  160,  32, 240},
{6236,  145,  44, 238},
{6244,  125,  38, 205},
{6252,   85,  26, 139},
{6260,  102,  51, 153},
{6275,  102,  51, 153},
{6289,  255,   0,   0},
{6294,  238,   0,   0},
{6299,  255,   0,   0},
{6303,  205,   0,   0},
{6308,  139,   0,   0},
{6313,  255, 193, 193},
{6324,  188, 143, 143},
{6335,  188, 143, 143},
{6345,  238, 180, 180},
{6356,  205, 155, 155},
{6367,  139, 105, 105},
{6378,   72, 118, 255},
{6389,   67, 110, 238},
{6400,   58,  95, 205},
{6411,   39,  64, 139},
{6422,   65, 105, 225},
{6433,   65, 105, 225},
{6443,  139,  69,  19},
{6456,  139,  69,  19},
{6468,  255, 140, 105},
{6476,  238, 130,  98},
{6484,  250, 128, 114},
{6491,  205, 112,  84},
{6499,  139,  76,  57},
{6507,  244, 164,  96},
{6519,  244, 164,  96},
{6530,   84, 255, 159},
{6540,   78, 238, 148},
{6550,   67, 205, 128},
{6560,   46, 139,  87},
{6570,   46, 139,  87},
{6580,   46, 139,  87},
{6589,  255, 245, 238},
{6599,  238, 229, 222},
{6609,  255, 245, 238},
{6618,  205, 197, 191},
{6628,  139, 134, 130},
{6638,  255, 130,  71},
{6646,  160,  82,  45},
{6653,  238, 121,  66},
{6661,  205, 104,  57},
{6669,  139,  71,  38},
{6677,  192, 192, 192},
{6684,  135, 206, 255},
{6693,  135, 206, 235},
{6702,  135, 206, 235},
{6710,  126, 192, 238},
{6719,  108, 166, 205},
{6728,   74, 112, 139},
{6737,  106,  90, 205},
{6748,  106,  90, 205},
{6758,  131, 111, 255},
{6769,  122, 103, 238},
{6780,  105,  89, 205},
{6791,   71,  60, 139},
{6802,  112, 128, 144},
{6813,  112, 128, 144},
{6823,  198, 226, 255},
{6834,  185, 211, 238},
{6845,  159, 182, 205},
{6856,  108, 123, 139},
{6867,  112, 128, 144},
{6878,  112, 128, 144},
{6888,  255, 250, 250},
{6894,  238, 233, 233},
{6900,  255, 250, 250},
{6905,  205, 201, 201},
{6911,  139, 137, 137},
{6917,    0, 255, 127},
{6930,    0, 255, 127},
{6942,    0, 255, 127},
{6955,    0, 238, 118},
{6968,    0, 205, 102},
{6981,    0, 139,  69},
{6994,   99, 184, 255},
{7005,   92, 172, 238},
{7016,   79, 148, 205},
{7027,   54, 100, 139},
{7038,   70, 130, 180},
{7049,   70, 130, 180},
{7059,  255, 165,  79},
{7064,  210, 180, 140},
{7068,  238, 154,  73},
{7073,  205, 133,  63},
{7078,  139,  90,  43},
{7083,    0, 128, 128},
{7088,  255, 225, 255},
{7097,  216, 191, 216},
{7105,  238, 210, 238},
{7114,  205, 181, 205},
{7123,  139, 123, 139},
{7132,  255,  99,  71},
{7140,  238,  92,  66},
{7148,  255,  99,  71},
{7155,  205,  79,  57},
{7163,  139,  54,  38},
{7171,    0, 245, 255},
{7182,    0, 229, 238},
{7193,    0, 197, 205},
{7204,    0, 134, 139},
{7215,   64, 224, 208},
{7225,  238, 130, 238},
{7232,  255,  62, 150},
{7243,  208,  32, 144},
{7254,  208,  32, 144},
{7264,  238,  58, 140},
{7275,  205,  50, 120},
{7286,  139,  34,  82},
{7297,  128, 128, 128},
{7306,  128, 128, 128},
{7314,    0, 128,   0},
{7324,    0, 128,   0},
{7333,  128, 128, 128},
{7342,  128, 128, 128},
{7350,  128,   0,   0},
{7361,  128,   0,   0},
{7371,  128,   0, 128},
{7382,  128,   0, 128},
{7392,  255, 231, 186},
{7399,  238, 216, 174},
{7406,  245, 222, 179},
{7412,  205, 186, 150},
{7419,  139, 126, 102},
{7426,  255, 255, 255},
{7432,  245, 245, 245},
{7444,  245, 245, 245},
{7455,  190, 190, 190},
{7464,  190, 190, 190},
{7472,    0, 255,   0},
{7482,    0, 255,   0},
{7491,  190, 190, 190},
{7500,  190, 190, 190},
{7508,  176,  48,  96},
{7519,  176,  48,  96},
{7529,  160,  32, 240},
{7540,  160,  32, 240},
{7550,  255, 255,   0},
{7558,  238, 238,   0},
{7566,  255, 255,   0},
{7573,  205, 205,   0},
{7581,  139, 139,   0},
{7589,  154, 205,  50},
{7602,  154, 205,  50}
};
static const char color_name_string[] =
"alice blue\0"
"aliceblue\0"
"antiquewhite1\0"
"antiquewhite2\0"
"antique white\0"
"antiquewhite\0"
"antiquewhite3\0"
"antiquewhite4\0"
"aqua\0"
"aquamarine1\0"
"aquamarine\0"
"aquamarine2\0"
"aquamarine3\0"
"aquamarine4\0"
"azure1\0"
"azure2\0"
"azure\0"
"azure3\0"
"azure4\0"
"beige\0"
"bisque1\0"
"bisque2\0"
"bisque\0"
"bisque3\0"
"bisque4\0"
"black\0"
"blanched almond\0"
"blanchedalmond\0"
"blue\0"
"blue1\0"
"blue2\0"
"blue3\0"
"blue4\0"
"blue violet\0"
"blueviolet\0"
"brown1\0"
"brown\0"
"brown2\0"
"brown3\0"
"brown4\0"
"burlywood1\0"
"burlywood\0"
"burlywood2\0"
"burlywood3\0"
"burlywood4\0"
"cadetblue1\0"
"cadetblue2\0"
"cadetblue3\0"
"cadetblue4\0"
"cadet blue\0"
"cadetblue\0"
"chartreuse1\0"
"chartreuse\0"
"chartreuse2\0"
"chartreuse3\0"
"chartreuse4\0"
"chocolate1\0"
"chocolate\0"
"chocolate2\0"
"chocolate3\0"
"chocolate4\0"
"coral1\0"
"coral2\0"
"coral\0"
"coral3\0"
"coral4\0"
"cornflower blue\0"
"cornflowerblue\0"
"cornsilk1\0"
"cornsilk2\0"
"cornsilk\0"
"cornsilk3\0"
"cornsilk4\0"
"crimson\0"
"cyan\0"
"cyan1\0"
"cyan2\0"
"cyan3\0"
"cyan4\0"
"dark blue\0"
"darkblue\0"
"dark cyan\0"
"darkcyan\0"
"darkgoldenrod1\0"
"dark goldenrod\0"
"darkgoldenrod\0"
"darkgoldenrod2\0"
"darkgoldenrod3\0"
"darkgoldenrod4\0"
"dark gray\0"
"darkgray\0"
"dark green\0"
"darkgreen\0"
"dark grey\0"
"darkgrey\0"
"dark khaki\0"
"darkkhaki\0"
"dark magenta\0"
"darkmagenta\0"
"darkolivegreen1\0"
"darkolivegreen2\0"
"darkolivegreen3\0"
"darkolivegreen4\0"
"dark olive green\0"
"darkolivegreen\0"
"darkorange1\0"
"darkorange2\0"
"dark orange\0"
"darkorange\0"
"darkorange3\0"
"darkorange4\0"
"darkorchid1\0"
"dark orchid\0"
"darkorchid\0"
"darkorchid2\0"
"darkorchid3\0"
"darkorchid4\0"
"dark red\0"
"darkred\0"
"dark salmon\0"
"darksalmon\0"
"darkseagreen1\0"
"dark sea green\0"
"darkseagreen\0"
"darkseagreen2\0"
"darkseagreen3\0"
"darkseagreen4\0"
"dark slate blue\0"
"darkslateblue\0"
"darkslategray1\0"
"darkslategray2\0"
"darkslategray3\0"
"dark slate gray\0"
"darkslategray\0"
"darkslategray4\0"
"dark slate grey\0"
"darkslategrey\0"
"dark turquoise\0"
"darkturquoise\0"
"dark violet\0"
"darkviolet\0"
"deeppink1\0"
"deeppink2\0"
"deep pink\0"
"deeppink\0"
"deeppink3\0"
"deeppink4\0"
"deep sky blue\0"
"deepskyblue\0"
"deepskyblue1\0"
"deepskyblue2\0"
"deepskyblue3\0"
"deepskyblue4\0"
"dim gray\0"
"dimgray\0"
"dim grey\0"
"dimgrey\0"
"dodgerblue1\0"
"dodgerblue2\0"
"dodger blue\0"
"dodgerblue\0"
"dodgerblue3\0"
"dodgerblue4\0"
"firebrick1\0"
"firebrick\0"
"firebrick2\0"
"firebrick3\0"
"firebrick4\0"
"floral white\0"
"floralwhite\0"
"forest green\0"
"forestgreen\0"
"fuchsia\0"
"gainsboro\0"
"ghost white\0"
"ghostwhite\0"
"gold1\0"
"gold2\0"
"gold\0"
"gold3\0"
"gold4\0"
"goldenrod1\0"
"goldenrod\0"
"goldenrod2\0"
"goldenrod3\0"
"goldenrod4\0"
"gray0\0"
"gray100\0"
"gray10\0"
"gray11\0"
"gray12\0"
"gray1\0"
"gray13\0"
"gray14\0"
"gray15\0"
"gray16\0"
"gray17\0"
"gray18\0"
"gray\0"
"gray19\0"
"gray20\0"
"gray21\0"
"gray22\0"
"gray23\0"
"gray24\0"
"gray2\0"
"gray25\0"
"gray26\0"
"gray27\0"
"gray28\0"
"gray29\0"
"gray30\0"
"gray31\0"
"gray32\0"
"gray33\0"
"gray34\0"
"gray35\0"
"gray36\0"
"gray37\0"
"gray3\0"
"gray38\0"
"gray39\0"
"gray40\0"
"gray4\0"
"gray41\0"
"gray42\0"
"gray43\0"
"gray44\0"
"gray45\0"
"gray46\0"
"gray47\0"
"gray48\0"
"gray49\0"
"gray50\0"
"gray51\0"
"gray5\0"
"gray52\0"
"gray53\0"
"gray54\0"
"gray55\0"
"gray56\0"
"gray57\0"
"gray58\0"
"gray59\0"
"gray60\0"
"gray61\0"
"gray6\0"
"gray62\0"
"gray63\0"
"gray64\0"
"gray65\0"
"gray66\0"
"gray67\0"
"gray68\0"
"gray69\0"
"gray70\0"
"gray71\0"
"gray7\0"
"gray72\0"
"gray73\0"
"gray74\0"
"gray75\0"
"gray76\0"
"gray77\0"
"gray78\0"
"gray79\0"
"gray80\0"
"gray81\0"
"gray8\0"
"gray82\0"
"gray83\0"
"gray84\0"
"gray85\0"
"gray86\0"
"gray87\0"
"gray88\0"
"gray89\0"
"gray90\0"
"gray91\0"
"gray92\0"
"gray9\0"
"gray93\0"
"gray94\0"
"gray95\0"
"gray96\0"
"gray97\0"
"gray98\0"
"gray99\0"
"green\0"
"green1\0"
"green2\0"
"green3\0"
"green4\0"
"green yellow\0"
"greenyellow\0"
"grey0\0"
"grey100\0"
"grey10\0"
"grey11\0"
"grey12\0"
"grey1\0"
"grey13\0"
"grey14\0"
"grey15\0"
"grey16\0"
"grey17\0"
"grey18\0"
"grey\0"
"grey19\0"
"grey20\0"
"grey21\0"
"grey22\0"
"grey23\0"
"grey24\0"
"grey2\0"
"grey25\0"
"grey26\0"
"grey27\0"
"grey28\0"
"grey29\0"
"grey30\0"
"grey31\0"
"grey32\0"
"grey33\0"
"grey34\0"
"grey35\0"
"grey36\0"
"grey37\0"
"grey3\0"
"grey38\0"
"grey39\0"
"grey40\0"
"grey4\0"
"grey41\0"
"grey42\0"
"grey43\0"
"grey44\0"
"grey45\0"
"grey46\0"
"grey47\0"
"grey48\0"
"grey49\0"
"grey50\0"
"grey51\0"
"grey5\0"
"grey52\0"
"grey53\0"
"grey54\0"
"grey55\0"
"grey56\0"
"grey57\0"
"grey58\0"
"grey59\0"
"grey60\0"
"grey61\0"
"grey6\0"
"grey62\0"
"grey63\0"
"grey64\0"
"grey65\0"
"grey66\0"
"grey67\0"
"grey68\0"
"grey69\0"
"grey70\0"
"grey71\0"
"grey7\0"
"grey72\0"
"grey73\0"
"grey74\0"
"grey75\0"
"grey76\0"
"grey77\0"
"grey78\0"
"grey79\0"
"grey80\0"
"grey81\0"
"grey8\0"
"grey82\0"
"grey83\0"
"grey84\0"
"grey85\0"
"grey86\0"
"grey87\0"
"grey88\0"
"grey89\0"
"grey90\0"
"grey91\0"
"grey92\0"
"grey9\0"
"grey93\0"
"grey94\0"
"grey95\0"
"grey96\0"
"grey97\0"
"grey98\0"
"grey99\0"
"honeydew1\0"
"honeydew2\0"
"honeydew\0"
"honeydew3\0"
"honeydew4\0"
"hotpink1\0"
"hotpink2\0"
"hot pink\0"
"hotpink\0"
"hotpink3\0"
"hotpink4\0"
"indianred1\0"
"indian red\0"
"indianred\0"
"indianred2\0"
"indianred3\0"
"indianred4\0"
"indigo\0"
"ivory1\0"
"ivory2\0"
"ivory\0"
"ivory3\0"
"ivory4\0"
"khaki1\0"
"khaki2\0"
"khaki\0"
"khaki3\0"
"khaki4\0"
"lavender\0"
"lavenderblush1\0"
"lavenderblush2\0"
"lavender blush\0"
"lavenderblush\0"
"lavenderblush3\0"
"lavenderblush4\0"
"lawn green\0"
"lawngreen\0"
"lemonchiffon1\0"
"lemonchiffon2\0"
"lemon chiffon\0"
"lemonchiffon\0"
"lemonchiffon3\0"
"lemonchiffon4\0"
"lightblue1\0"
"light blue\0"
"lightblue\0"
"lightblue2\0"
"lightblue3\0"
"lightblue4\0"
"light coral\0"
"lightcoral\0"
"lightcyan1\0"
"lightcyan2\0"
"light cyan\0"
"lightcyan\0"
"lightcyan3\0"
"lightcyan4\0"
"lightgoldenrod1\0"
"lightgoldenrod2\0"
"light goldenrod\0"
"lightgoldenrod\0"
"lightgoldenrod3\0"
"lightgoldenrod4\0"
"light goldenrod yellow\0"
"lightgoldenrodyellow\0"
"light gray\0"
"lightgray\0"
"light green\0"
"lightgreen\0"
"light grey\0"
"lightgrey\0"
"lightpink1\0"
"lightpink2\0"
"light pink\0"
"lightpink\0"
"lightpink3\0"
"lightpink4\0"
"lightsalmon1\0"
"lightsalmon2\0"
"light salmon\0"
"lightsalmon\0"
"lightsalmon3\0"
"lightsalmon4\0"
"light sea green\0"
"lightseagreen\0"
"lightskyblue1\0"
"light sky blue\0"
"lightskyblue\0"
"lightskyblue2\0"
"lightskyblue3\0"
"lightskyblue4\0"
"light slate blue\0"
"lightslateblue\0"
"light slate gray\0"
"lightslategray\0"
"light slate grey\0"
"lightslategrey\0"
"lightsteelblue1\0"
"light steel blue\0"
"lightsteelblue\0"
"lightsteelblue2\0"
"lightsteelblue3\0"
"lightsteelblue4\0"
"lightyellow1\0"
"lightyellow2\0"
"light yellow\0"
"lightyellow\0"
"lightyellow3\0"
"lightyellow4\0"
"lime\0"
"lime green\0"
"limegreen\0"
"linen\0"
"magenta1\0"
"magenta2\0"
"magenta\0"
"magenta3\0"
"magenta4\0"
"maroon1\0"
"maroon\0"
"maroon2\0"
"maroon3\0"
"maroon4\0"
"medium aquamarine\0"
"mediumaquamarine\0"
"medium blue\0"
"mediumblue\0"
"mediumorchid1\0"
"medium orchid\0"
"mediumorchid\0"
"mediumorchid2\0"
"mediumorchid3\0"
"mediumorchid4\0"
"mediumpurple1\0"
"medium purple\0"
"mediumpurple\0"
"mediumpurple2\0"
"mediumpurple3\0"
"mediumpurple4\0"
"medium sea green\0"
"mediumseagreen\0"
"medium slate blue\0"
"mediumslateblue\0"
"medium spring green\0"
"mediumspringgreen\0"
"medium turquoise\0"
"mediumturquoise\0"
"medium violet red\0"
"mediumvioletred\0"
"midnight blue\0"
"midnightblue\0"
"mint cream\0"
"mintcream\0"
"mistyrose1\0"
"mistyrose2\0"
"misty rose\0"
"mistyrose\0"
"mistyrose3\0"
"mistyrose4\0"
"moccasin\0"
"navajowhite1\0"
"navajowhite2\0"
"navajo white\0"
"navajowhite\0"
"navajowhite3\0"
"navajowhite4\0"
"navy\0"
"navy blue\0"
"navyblue\0"
"old lace\0"
"oldlace\0"
"olive\0"
"olive drab\0"
"olivedrab\0"
"olivedrab1\0"
"olivedrab2\0"
"olivedrab3\0"
"olivedrab4\0"
"orange1\0"
"orange2\0"
"orange\0"
"orange3\0"
"orange4\0"
"orangered1\0"
"orangered2\0"
"orange red\0"
"orangered\0"
"orangered3\0"
"orangered4\0"
"orchid1\0"
"orchid\0"
"orchid2\0"
"orchid3\0"
"orchid4\0"
"pale goldenrod\0"
"palegoldenrod\0"
"palegreen1\0"
"pale green\0"
"palegreen\0"
"palegreen2\0"
"palegreen3\0"
"palegreen4\0"
"paleturquoise1\0"
"pale turquoise\0"
"paleturquoise\0"
"paleturquoise2\0"
"paleturquoise3\0"
"paleturquoise4\0"
"palevioletred1\0"
"pale violet red\0"
"palevioletred\0"
"palevioletred2\0"
"palevioletred3\0"
"palevioletred4\0"
"papaya whip\0"
"papayawhip\0"
"peachpuff1\0"
"peachpuff2\0"
"peach puff\0"
"peachpuff\0"
"peachpuff3\0"
"peachpuff4\0"
"peru\0"
"pink1\0"
"pink2\0"
"pink\0"
"pink3\0"
"pink4\0"
"plum1\0"
"plum\0"
"plum2\0"
"plum3\0"
"plum4\0"
"powder blue\0"
"powderblue\0"
"purple1\0"
"purple\0"
"purple2\0"
"purple3\0"
"purple4\0"
"rebecca purple\0"
"rebeccapurple\0"
"red1\0"
"red2\0"
"red\0"
"red3\0"
"red4\0"
"rosybrown1\0"
"rosy brown\0"
"rosybrown\0"
"rosybrown2\0"
"rosybrown3\0"
"rosybrown4\0"
"royalblue1\0"
"royalblue2\0"
"royalblue3\0"
"royalblue4\0"
"royal blue\0"
"royalblue\0"
"saddle brown\0"
"saddlebrown\0"
"salmon1\0"
"salmon2\0"
"salmon\0"
"salmon3\0"
"salmon4\0"
"sandy brown\0"
"sandybrown\0"
"seagreen1\0"
"seagreen2\0"
"seagreen3\0"
"seagreen4\0"
"sea green\0"
"seagreen\0"
"seashell1\0"
"seashell2\0"
"seashell\0"
"seashell3\0"
"seashell4\0"
"sienna1\0"
"sienna\0"
"sienna2\0"
"sienna3\0"
"sienna4\0"
"silver\0"
"skyblue1\0"
"sky blue\0"
"skyblue\0"
"skyblue2\0"
"skyblue3\0"
"skyblue4\0"
"slate blue\0"
"slateblue\0"
"slateblue1\0"
"slateblue2\0"
"slateblue3\0"
"slateblue4\0"
"slate gray\0"
"slategray\0"
"slategray1\0"
"slategray2\0"
"slategray3\0"
"slategray4\0"
"slate grey\0"
"slategrey\0"
"snow1\0"
"snow2\0"
"snow\0"
"snow3\0"
"snow4\0"
"spring green\0"
"springgreen\0"
"springgreen1\0"
"springgreen2\0"
"springgreen3\0"
"springgreen4\0"
"steelblue1\0"
"steelblue2\0"
"steelblue3\0"
"steelblue4\0"
"steel blue\0"
"steelblue\0"
"tan1\0"
"tan\0"
"tan2\0"
"tan3\0"
"tan4\0"
"teal\0"
"thistle1\0"
"thistle\0"
"thistle2\0"
"thistle3\0"
"thistle4\0"
"tomato1\0"
"tomato2\0"
"tomato\0"
"tomato3\0"
"tomato4\0"
"turquoise1\0"
"turquoise2\0"
"turquoise3\0"
"turquoise4\0"
"turquoise\0"
"violet\0"
"violetred1\0"
"violet red\0"
"violetred\0"
"violetred2\0"
"violetred3\0"
"violetred4\0"
"web gray\0"
"webgray\0"
"web green\0"
"webgreen\0"
"web grey\0"
"webgrey\0"
"web maroon\0"
"webmaroon\0"
"web purple\0"
"webpurple\0"
"wheat1\0"
"wheat2\0"
"wheat\0"
"wheat3\0"
"wheat4\0"
"white\0"
"white smoke\0"
"whitesmoke\0"
"x11 gray\0"
"x11gray\0"
"x11 green\0"
"x11green\0"
"x11 grey\0"
"x11grey\0"
"x11 maroon\0"
"x11maroon\0"
"x11 purple\0"
"x11purple\0"
"yellow1\0"
"yellow2\0"
"yellow\0"
"yellow3\0"
"yellow4\0"
"yellow green\0"
"yellowgreen"
;

static int
_xpm_hexa_int(const char *s, int len)
{
   const char *hexa = "0123456789abcdef";
   const char *lookup;
   int i, c, r;

   for (r = 0, i = 0; i < len; i++)
     {
        c = s[i];
        lookup = strchr(hexa, tolower(c));
        r = (r << 4) | (lookup ? lookup - hexa : 0);
     }

   return r;
}

static void
xpm_parse_color(char *color, int *r, int *g, int *b)
{
   int i;

   /* is a #ff00ff like color */
   if (color[0] == '#')
     {
        int                 len;

        len = strlen(color) - 1;
        if (len < 13)
          {

             len /= 3;
             *r = _xpm_hexa_int(&(color[1 + (0 * len)]), len);
             *g = _xpm_hexa_int(&(color[1 + (1 * len)]), len);
             *b = _xpm_hexa_int(&(color[1 + (2 * len)]), len);
             if (len == 1)
               {
                  *r = (*r << 4) | *r;
                  *g = (*g << 4) | *g;
                  *b = (*b << 4) | *b;
               }
             else if (len > 2)
               {
                  *r >>= (len - 2) * 4;
                  *g >>= (len - 2) * 4;
                  *b >>= (len - 2) * 4;
               }
          }
        return;
     }

   /* look in rgb txt database */
   // yes this is a linear search. as it's sorted we could binary-search...
   // but come on. it's an xpm file. we do this only for the palette and
   // we're already forced to paars c src code text to load an image...
   // so does it matter?
   for (i = 0; i < 782; i++) // there are 786 entries in color index
     {
        if (!strcasecmp(color_name_string + color_name_index[i].offset, color))
          {
             *r = color_name_index[i].r;
             *g = color_name_index[i].g;
             *b = color_name_index[i].b;
             return;
          }
     }
}

typedef struct _CMap CMap;
struct _CMap {
   EINA_RBTREE;
   short         r, g, b;
   char          str[6];
   unsigned char transp;
};

Eina_Rbtree_Direction
_cmap_cmp_node_cb(const Eina_Rbtree *left, const Eina_Rbtree *right, void *data EINA_UNUSED)
{
   CMap *lcm;
   CMap *rcm;

   lcm = EINA_RBTREE_CONTAINER_GET(left, CMap);
   rcm = EINA_RBTREE_CONTAINER_GET(right, CMap);

   if (strcmp(lcm->str, rcm->str) < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

int
_cmap_cmp_key_cb(const Eina_Rbtree *node, const void *key, int length EINA_UNUSED, void *data EINA_UNUSED)
{
   CMap *root = EINA_RBTREE_CONTAINER_GET(node, CMap);

   return strcmp(root->str, key);
}

/** FIXME: clean this up and make more efficient  **/
static Eina_Bool
evas_image_load_file_xpm(Eina_File *f, Evas_Image_Property *prop, void *pixels, int load_data, int *error)
{
   DATA32      *ptr, *end, *head = NULL;
   const char  *map = NULL;
   size_t       length;
   size_t       position;

   int          pc, c, i, j, k, w, h, ncolors, cpp, comment, transp,
                quote, context, len, done, r, g, b, backslash, lu1, lu2;
   char        *line = NULL;
   char         s[256], tok[256], col[256], *tl;
   int          lsz = 256;
   CMap        *cmap = NULL;
   Eina_Rbtree *root = NULL;

   short        lookup[128 - 32][128 - 32];
   int          count, size;
   Eina_Bool    res = EINA_FALSE;

   done = 0;
   transp = 1;

   /* if immediate_load is 1, then dont delay image laoding as below, or */
   /* already data in this image - dont load it again */

   length = eina_file_size_get(f);
   position = 0;
   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (length < 9) goto on_error;

   map = eina_file_map_all(f, load_data ? EINA_FILE_WILLNEED : EINA_FILE_RANDOM);
   if (!map) goto on_error;

   if (strncmp("/* XPM */", map, 9))
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        goto on_error;
     }

   i = 0;
   j = 0;
   cmap = NULL;
   w = 10;
   h = 10;
   ptr = NULL;
   end = NULL;
   c = ' ';
   comment = 0;
   quote = 0;
   context = 0;
   size = 0;
   count = 0;
   line = malloc(lsz);
   if (!line)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        goto on_error;
     }

   backslash = 0;
   memset(lookup, 0, sizeof(lookup));
   while (!done)
     {
        pc = c;
        if (position == length) break ;
        c = (char) map[position++];
        if (!quote)
          {
             if ((pc == '/') && (c == '*')) comment = 1;
             else if ((pc == '*') && (c == '/') && (comment)) comment = 0;
          }
        if (!comment)
          {
             if ((!quote) && (c == '"'))
               {
                  quote = 1;
                  i = 0;
               }
             else if ((quote) && (c == '"'))
               {
                  line[i] = 0;
                  quote = 0;
                  if (context == 0)
                    {
                       /* Header */
                       if (sscanf(line, "%i %i %i %i", &w, &h, &ncolors, &cpp) != 4)
                         {
                            *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
                            goto on_error;
                         }
                       if ((ncolors > 32766) || (ncolors < 1))
                         {
                            ERR("XPM ERROR: XPM files with colors > 32766 or < 1 not supported");
                            *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                            goto on_error;
                         }
                       if ((cpp > 5) || (cpp < 1))
                         {
                            ERR("XPM ERROR: XPM files with characters per pixel > 5 or < 1not supported");
                            *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
                            goto on_error;
                         }
                       if ((w > IMG_MAX_SIZE) || (w < 1))
                         {
                            ERR("XPM ERROR: Image width > IMG_MAX_SIZE or < 1 pixels for file");
                            *error = EVAS_LOAD_ERROR_GENERIC;
                            goto on_error;
                         }
                       if ((h > IMG_MAX_SIZE) || (h < 1))
                         {
                            ERR("XPM ERROR: Image height > IMG_MAX_SIZE or < 1 pixels for file");
                            *error = EVAS_LOAD_ERROR_GENERIC;
                            goto on_error;
                         }
                       if (IMG_TOO_BIG(w, h))
                         {
                            ERR("XPM ERROR: Image just too big to ever allocate");
                            *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                            goto on_error;
                         }

                       if (!cmap)
                         {
                            cmap = malloc(sizeof(CMap) * ncolors);
                            if (!cmap)
                              {
                                 *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                                goto on_error;
                              }
                         }

                       if (!load_data)
                         {
                            prop->w = w;
                            prop->h = h;
                         }
                       else if ((int) prop->w != w ||
                                (int) prop->h != h)
                         {
                            *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                            goto on_error;
                         }


                       j = 0;
                       context++;
                    }
                  else if (context == 1)
                    {
                       /* Color Table */
                       if (j < ncolors)
                         {
                            int                 slen;
                            int                 hascolor, iscolor;

                            iscolor = 0;
                            hascolor = 0;
                            tok[0] = 0;
                            col[0] = 0;
                            s[0] = 0;
                            len = strlen(line);
                            strncpy(cmap[j].str, line, cpp);
                            cmap[j].str[cpp] = 0;
                            if (load_data) root = eina_rbtree_inline_insert(root, EINA_RBTREE_GET(&cmap[j]), _cmap_cmp_node_cb, NULL);
                            for (slen = 0; slen < cpp; slen++)
                              {
                                 /* fix the ascii of the  color string - if its < 32 - just limit to 32 */
                                 if (cmap[j].str[slen] < 32) cmap[j].str[slen] = 0;
                              }
                            cmap[j].r = -1;
                            cmap[j].transp = 0;
                            for (k = cpp; k < len; k++)
                              {
                                 if (line[k] != ' ')
                                   {
                                      const char *tmp = strchr(&line[k], ' ');
                                      slen = tmp ? tmp - &line[k]: 255;

                                      strncpy(s, &line[k], slen);
                                      s[slen] = 0;
                                      k += slen;
                                      if (slen == 1 && *s == 'c') iscolor = 1;
                                      if ((slen == 1 && ((s[0] == 'm') || (s[0] == 's') || (s[0] == 'g') || (s[0] == 'c'))) ||
                                          (slen == 2 && (s[0] == 'g') && (s[1] == '4')) ||
                                          (k >= len))
                                        {
                                           if (k >= len)
                                             {
                                                if (col[0])
                                                  {
                                                     if (strlen(col) < (sizeof(col) - 2))
                                                       strcat(col, " ");
                                                     else done = 1;
                                                  }
                                                if ((strlen(col) + strlen(s)) < (sizeof(col) - 1))
                                                  strcat(col, s);
                                             }
                                           if (col[0])
                                             {
                                                if (!strcasecmp(col, "none"))
                                                  {
                                                     transp = 1;
                                                     cmap[j].transp = 1;
                                                     cmap[j].r = 0;
                                                     cmap[j].g = 0;
                                                     cmap[j].b = 0;
                                                  }
                                                else
                                                  {
                                                     if ((((cmap[j].r < 0) || (!strcmp(tok, "c"))) && (!hascolor)))
                                                       {
                                                          r = g = b = 0;
                                                          xpm_parse_color(col, &r, &g, &b);
                                                          cmap[j].r = r;
                                                          cmap[j].g = g;
                                                          cmap[j].b = b;
                                                          if (iscolor) hascolor = 1;
                                                       }
                                                  }
                                             }
                                           strcpy(tok, s);
                                           col[0] = 0;
                                        }
                                      else
                                        {
                                           if (col[0])
                                             {
                                                if (strlen(col) < ( sizeof(col) - 2))
                                                  strcat(col, " ");
                                                else done = 1;
                                             }
                                           if ((strlen(col) + strlen(s)) < (sizeof(col) - 1))
                                             strcat(col, s);
                                        }
                                   }
                              }
                         }
                       j++;
                       if (load_data && j >= ncolors)
                         {
                            if (cpp == 1)
                              {
                                 for (i = 0; i < ncolors; i++)
                                   lookup[(int)cmap[i].str[0] - 32][0] = i;
                              }
                            if (cpp == 2)
                              {
                                 for (i = 0; i < ncolors; i++)
                                   lookup[(int)cmap[i].str[0] - 32][(int)cmap[i].str[1] - 32] = i;
                              }
                            context++;
                         }

                       if (transp) prop->alpha = 1;

                       if (load_data)
                         {
                            ptr = pixels;
                            head = ptr;
                            if (!ptr)
                              {
                                 *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                                 goto on_error;
                              }
                            size = w * h;
                            end = ptr + size;
                         }
                       else
                         {
                            *error = EVAS_LOAD_ERROR_NONE;
                            goto on_success;
                         }
                    }
                  else
                    {
                       /* Image Data */
                       i = 0;
                       if (cpp == 0)
                         {
                            /* Chars per pixel = 0? well u never know */
                         }
                       /* it's xpm - don't care about speed too much. still faster
                        * that most xpm loaders anyway */
                       else if (cpp == 1)
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      lu1 = (int)line[i] - 32;
                                      if (lu1 < 0) continue;
                                      if (cmap[lookup[lu1][0]].transp)
                                        {
                                           r = (unsigned char)cmap[lookup[lu1][0]].r;
                                           g = (unsigned char)cmap[lookup[lu1][0]].g;
                                           b = (unsigned char)cmap[lookup[lu1][0]].b;
                                           *ptr = RGB_JOIN(r, g, b);
                                           ptr++;
                                           count++;
                                        }
                                      else
                                        {
                                           r = (unsigned char)cmap[lookup[lu1][0]].r;
                                           g = (unsigned char)cmap[lookup[lu1][0]].g;
                                           b = (unsigned char)cmap[lookup[lu1][0]].b;
                                           *ptr = ARGB_JOIN(0xff, r, g, b);
                                           ptr++;
                                           count++;
                                        }
                                   }
                              }
                            else
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      lu1 = (int)line[i] - 32;
                                      if (lu1 < 0) continue;
                                      r = (unsigned char)cmap[lookup[lu1][0]].r;
                                      g = (unsigned char)cmap[lookup[lu1][0]].g;
                                      b = (unsigned char)cmap[lookup[lu1][0]].b;
                                      *ptr = ARGB_JOIN(0xff, r, g, b);
                                      ptr++;
                                      count++;
                                   }
                              }
                         }
                       else if (cpp == 2)
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      lu1 = (int)line[i] - 32;
                                      i++;
                                      lu2 = (int)line[i] - 32;
                                      if (lu1 < 0) continue;
                                      if (lu2 < 0) continue;
                                      if (cmap[lookup[lu1][lu2]].transp)
                                        {
                                           r = (unsigned char)cmap[lookup[lu1][lu2]].r;
                                           g = (unsigned char)cmap[lookup[lu1][lu2]].g;
                                           b = (unsigned char)cmap[lookup[lu1][lu2]].b;
                                           *ptr = RGB_JOIN(r, g, b);
                                           ptr++;
                                           count++;
                                        }
                                      else
                                        {
                                           r = (unsigned char)cmap[lookup[lu1][lu2]].r;
                                           g = (unsigned char)cmap[lookup[lu1][lu2]].g;
                                           b = (unsigned char)cmap[lookup[lu1][lu2]].b;
                                           *ptr = ARGB_JOIN(0xff, r, g, b);
                                           ptr++;
                                           count++;
                                        }
                                   }
                              }
                            else
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      lu1 = (int)line[i] - 32;
                                      i++;
                                      lu2 = (int)line[i] - 32;
                                      if (lu1 < 0) continue;
                                      if (lu2 < 0) continue;
                                      r = (unsigned char)cmap[lookup[lu1][lu2]].r;
                                      g = (unsigned char)cmap[lookup[lu1][lu2]].g;
                                      b = (unsigned char)cmap[lookup[lu1][lu2]].b;
                                      *ptr = ARGB_JOIN(0xff, r, g, b);
                                      ptr++;
                                      count++;
                                   }
                              }
                         }
                       else
                         {
                            if (transp)
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      Eina_Rbtree *l;

                                      for (j = 0; j < cpp; j++, i++)
                                        {
                                           col[j] = line[i];
                                           if (col[j] < 32) col[j] = 32;
                                        }
                                      col[j] = 0;
                                      i--;

                                      l = eina_rbtree_inline_lookup(root, col, j, _cmap_cmp_key_cb, NULL);
                                      if (l)
                                        {
                                           CMap *cm = EINA_RBTREE_CONTAINER_GET(l, CMap);

                                           r = (unsigned char)cm->r;
                                           g = (unsigned char)cm->g;
                                           b = (unsigned char)cm->b;
                                           if (cm->transp)
                                             *ptr = RGB_JOIN(r, g, b);
                                           else
                                             *ptr = ARGB_JOIN(0xff, r, g, b);

                                           ptr++;
                                           count++;
                                        }
                                   }
                              }
                            else
                              {
                                 for (i = 0;
                                      ((i < 65536) && (ptr < end) && (line[i]));
                                      i++)
                                   {
                                      Eina_Rbtree *l;

                                      for (j = 0; j < cpp; j++, i++)
                                        col[j] = line[i];
                                      col[j] = 0;
                                      i--;

                                      l = eina_rbtree_inline_lookup(root, col, 0, _cmap_cmp_key_cb, NULL);
                                      if (l)
                                        {
                                           CMap *cm = EINA_RBTREE_CONTAINER_GET(l, CMap);

                                           r = (unsigned char)cm->r;
                                           g = (unsigned char)cm->g;
                                           b = (unsigned char)cm->b;
                                           *ptr = ARGB_JOIN(0xff, r, g, b);
                                           ptr++;
                                           count++;
                                        }
                                   }
                              }
                         }
                    }
               }
          }
        /* Scan in line from XPM file */
        if ((!comment) && (quote) && (c != '"'))
          {
             if (c < 32) c = 32;
             else if (c > 127) c = 127;
             if (c =='\\')
               {
                  if (++backslash < 2) line[i++] = c;
                  else backslash = 0;
               }
             else
               {
                  backslash = 0;
                  line[i++] = c;
               }
          }
        if (i >= lsz)
          {
             lsz += 256;
             tl = realloc(line, lsz);
             if (!tl) break;
             line = tl;
          }
        if (((ptr) && ((ptr - head) >= (w * h))) ||
            ((context > 1) && (count >= size)))
          break;
     }

 on_success:
   *error = EVAS_LOAD_ERROR_NONE;
   res = EINA_TRUE;

 on_error:
   if (map) eina_file_map_free(f, (void*) map);
   free(cmap);
   free(line);
   return res;
}

static void *
evas_image_load_file_open_xpm(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts EINA_UNUSED,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error EINA_UNUSED)
{
  return f;
}

static void
evas_image_load_file_close_xpm(void *loader_data EINA_UNUSED)
{
}

static Eina_Bool
evas_image_load_file_head_xpm(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   Eina_File *f = loader_data;

   return evas_image_load_file_xpm(f, prop, NULL, 0, error);
}

static Eina_Bool
evas_image_load_file_data_xpm(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   Eina_File *f = loader_data;

   return evas_image_load_file_xpm(f, prop, pixels, 1, error);
}

static Evas_Image_Load_Func evas_image_load_xpm_func =
{
  evas_image_load_file_open_xpm,
  evas_image_load_file_close_xpm,
  evas_image_load_file_head_xpm,
  evas_image_load_file_data_xpm,
  NULL,
  EINA_FALSE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_loader_xpm_log_dom = eina_log_domain_register
     ("evas-xpm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_loader_xpm_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   em->functions = (void *)(&evas_image_load_xpm_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_loader_xpm_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_loader_xpm_log_dom);
        _evas_loader_xpm_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "xpm",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, xpm);

#ifndef EVAS_STATIC_BUILD_XPM
EVAS_EINA_MODULE_DEFINE(image_loader, xpm);
#endif
