// File: rg_etc1.cpp - Fast, high quality ETC1 block packer/unpacker - Rich Geldreich <richgel99@gmail.com>
// Please see ZLIB license at the end of rg_etc1.h.
//
// For more information Ericsson Texture Compression (ETC/ETC1), see:
// http://www.khronos.org/registry/gles/extensions/OES/OES_compressed_ETC1_RGB8_texture.txt
//
// v1.04 - 5/15/14 - Fix signed vs. unsigned subtraction problem (noticed when compiled with gcc) in pack_etc1_block_init().
//         This issue would cause an assert when this func. was called in debug. (Note this module was developed/testing with MSVC,
//         I still need to test it throughly when compiled with gcc.)
//
// v1.03 - 5/12/13 - Initial public release

#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "Eina.h"

#include "rg_etc1.h"

#if defined(_DEBUG) || defined(DEBUG)
#define RG_ETC1_BUILD_DEBUG
#endif

typedef unsigned long long uint64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef unsigned int uint32;
typedef unsigned char DATA8;

#define MIN(A, B) ((A < B) ? A : B)
#define MAX(A, B) ((A > B) ? A : B)
#define CLAMP(Value, Low, High) ((Value < Low) ? Low : ((Value > High) ? High : Value))
#define SQUARE(Value) (Value * Value)

#ifndef UINT_MAX
#define UINT_MAX  4294967295U
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

#define cUINT32_MAX UINT_MAX
#define cUINT64_MAX ULLONG_MAX
#define RG_ETC1_ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

// Some configuration defines

// Disable this constrained function, it produces artifacts (in black areas mostly)
#define RG_ETC1_CONSTRAINED_SUBBLOCK 0
// Disable dithering. It uses invalid RGBA order and isn't great visually
// Dithering should happen AFTER the color selection, not before
#define RG_ETC1_DITHERING 0

enum RG_Etc_Constants
  {
    cETC1BytesPerBlock = 8U,

    cETC1SelectorBits = 2U,
    cETC1SelectorValues = 1U << cETC1SelectorBits,
    cETC1SelectorMask = cETC1SelectorValues - 1U,

    cETC1BlockShift = 2U,
    cETC1BlockSize = 1U << cETC1BlockShift,

    cETC1LSBSelectorIndicesBitOffset = 0,
    cETC1MSBSelectorIndicesBitOffset = 16,

    cETC1FlipBitOffset = 32,
    cETC1DiffBitOffset = 33,

    cETC1IntenModifierNumBits = 3,
    cETC1IntenModifierValues = 1 << cETC1IntenModifierNumBits,
    cETC1RightIntenModifierTableBitOffset = 34,
    cETC1LeftIntenModifierTableBitOffset = 37,

    // Base+Delta encoding (5 bit bases, 3 bit delta)
    cETC1BaseColorCompNumBits = 5,
    cETC1BaseColorCompMax = 1 << cETC1BaseColorCompNumBits,

    cETC1DeltaColorCompNumBits = 3,
    cETC1DeltaColorComp = 1 << cETC1DeltaColorCompNumBits,
    cETC1DeltaColorCompMax = 1 << cETC1DeltaColorCompNumBits,

    cETC1BaseColor5RBitOffset = 59,
    cETC1BaseColor5GBitOffset = 51,
    cETC1BaseColor5BBitOffset = 43,

    cETC1DeltaColor3RBitOffset = 56,
    cETC1DeltaColor3GBitOffset = 48,
    cETC1DeltaColor3BBitOffset = 40,

    // Absolute (non-delta) encoding (two 4-bit per component bases)
    cETC1AbsColorCompNumBits = 4,
    cETC1AbsColorCompMax = 1 << cETC1AbsColorCompNumBits,

    cETC1AbsColor4R1BitOffset = 60,
    cETC1AbsColor4G1BitOffset = 52,
    cETC1AbsColor4B1BitOffset = 44,

    cETC1AbsColor4R2BitOffset = 56,
    cETC1AbsColor4G2BitOffset = 48,
    cETC1AbsColor4B2BitOffset = 40,

    cETC1ColorDeltaMin = -4,
    cETC1ColorDeltaMax = 3,

    // Delta3:
    // 0   1   2   3   4   5   6   7
    // 000 001 010 011 100 101 110 111
    // 0   1   2   3   -4  -3  -2  -1
  };

/*
 * IMPORTANT NOTE:
 *
 * rg_etc1 originally works only on R,G,B,A data
 * evas works on B,G,R,A data
 *
 * ARGB_JOIN() is used for unpacking, so it will directly produce BGRA.
 *
 * Upon packing, we convert BGRA to RGBA so we can use the precomputed tables,
 * so we must use the X_VAL_GET() macros.
 * Upon unpacking, we directly output BGRA data using ARGB_JOIN() and X_VAL_SET()
 *
 * Yes, this is a mess. Maybe a clear BGRA API is needed
 */

#ifndef WORDS_BIGENDIAN
// BGRA
#define A_VAL_SET(p) (((DATA8 *)(p))[3])
#define R_VAL_SET(p) (((DATA8 *)(p))[2])
#define G_VAL_SET(p) (((DATA8 *)(p))[1])
#define B_VAL_SET(p) (((DATA8 *)(p))[0])
// RGBA
#define A_VAL_GET(p) (((DATA8 *)(p))[3])
#define R_VAL_GET(p) (((DATA8 *)(p))[0])
#define G_VAL_GET(p) (((DATA8 *)(p))[1])
#define B_VAL_GET(p) (((DATA8 *)(p))[2])
#else
// BIGENDIAN is untested
#define A_VAL_SET(p) (((DATA8 *)(p))[0])
#define R_VAL_SET(p) (((DATA8 *)(p))[1])
#define G_VAL_SET(p) (((DATA8 *)(p))[2])
#define B_VAL_SET(p) (((DATA8 *)(p))[3])
#define A_VAL_GET(p) (((DATA8 *)(p))[0])
#define R_VAL_GET(p) (((DATA8 *)(p))[3])
#define G_VAL_GET(p) (((DATA8 *)(p))[2])
#define B_VAL_GET(p) (((DATA8 *)(p))[1])
#endif

#define A_MASK (0xFFul << 24)

// For unpacking and writing BGRA output data
#define ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

static unsigned char rg_etc_quant5_tab[256 + 16];

static const int rg_etc1_inten_tables[cETC1IntenModifierValues][cETC1SelectorValues] = {
  { -8,   -2,   2,   8 },
  { -17,  -5,   5,  17 },
  { -29,  -9,   9,  29 },
  { -42,  -13, 13,  42 },
  { -60,  -18, 18,  60 },
  { -80,  -24, 24,  80 },
  { -106, -33, 33, 106 },
  { -183, -47, 47, 183 }
};

static const unsigned char rg_etc1_to_selector_index[cETC1SelectorValues] = { 2, 3, 1, 0 };
static const unsigned char rg_etc_selector_index_to_etc1[cETC1SelectorValues] = { 3, 2, 0, 1 };

// Given an ETC1 diff/inten_table/selector, and an 8-bit desired color, this table encodes the best packed_color in the low byte, and the abs error in the high byte.
static unsigned short rg_etc1_inverse_lookup[2*8*4][256]; // [diff/inten_table/selector][desired_color]

// rg_color8_to_etc_block_config[color][table_index] = Supplies for each 8-bit color value a list of packed ETC1 diff/intensity table/selectors/packed_colors that map to that color.
// To pack: diff | (inten << 1) | (selector << 4) | (packed_c << 8)
static const unsigned short rg_etc_color8_to_etc_block_config_0_255[2][33] = {
  {
    0x0000, 0x0010, 0x0002, 0x0012, 0x0004, 0x0014, 0x0006, 0x0016, 0x0008,
    0x0018, 0x000A, 0x001A, 0x000C, 0x001C, 0x000E, 0x001E, 0x0001, 0x0011,
    0x0003, 0x0013, 0x0005, 0x0015, 0x0007, 0x0017, 0x0009, 0x0019, 0x000B,
    0x001B, 0x000D, 0x001D, 0x000F, 0x001F, 0xFFFF
  },
  {
    0x0F20, 0x0F30, 0x0E32, 0x0F22, 0x0E34, 0x0F24, 0x0D36, 0x0F26, 0x0C38,
    0x0E28, 0x0B3A, 0x0E2A, 0x093C, 0x0E2C, 0x053E, 0x0D2E, 0x1E31, 0x1F21,
    0x1D33, 0x1F23, 0x1C35, 0x1E25, 0x1A37, 0x1E27, 0x1839, 0x1D29, 0x163B,
    0x1C2B,  0x133D,  0x1B2D,  0x093F,  0x1A2F, 0xFFFF
  },
};

// Really only [254][11].
static const unsigned short rg_etc_color8_to_etc_block_config_1_to_254[254][12] = {
  { 0x021C, 0x0D0D, 0xFFFF },
  { 0x0020, 0x0021, 0x0A0B, 0x061F, 0xFFFF },
  { 0x0113, 0x0217, 0xFFFF },
  { 0x0116, 0x031E, 0x0B0E, 0x0405, 0xFFFF },
  { 0x0022, 0x0204, 0x050A, 0x0023, 0xFFFF },
  { 0x0111, 0x0319, 0x0809, 0x170F, 0xFFFF },
  { 0x0303, 0x0215, 0x0607, 0xFFFF },
  { 0x0030, 0x0114, 0x0408, 0x0031, 0x0201, 0x051D, 0xFFFF },
  { 0x0100, 0x0024, 0x0306, 0x0025, 0x041B, 0x0E0D, 0xFFFF },
  { 0x021A, 0x0121, 0x0B0B, 0x071F, 0xFFFF },
  { 0x0213, 0x0317, 0xFFFF },
  { 0x0112, 0x0505, 0xFFFF },
  { 0x0026, 0x070C, 0x0123, 0x0027, 0xFFFF },
  { 0x0211, 0x0909, 0xFFFF },
  { 0x0110, 0x0315, 0x0707, 0x0419, 0x180F, 0xFFFF },
  { 0x0218, 0x0131, 0x0301, 0x0403, 0x061D, 0xFFFF },
  { 0x0032, 0x0202, 0x0033, 0x0125, 0x051B, 0x0F0D, 0xFFFF },
  { 0x0028, 0x031C, 0x0221, 0x0029, 0xFFFF },
  { 0x0120, 0x0313, 0x0C0B, 0x081F, 0xFFFF },
  { 0x0605, 0x0417, 0xFFFF },
  { 0x0216, 0x041E, 0x0C0E, 0x0223, 0x0127, 0xFFFF },
  { 0x0122, 0x0304, 0x060A, 0x0311, 0x0A09, 0xFFFF },
  { 0x0519, 0x190F, 0xFFFF },
  { 0x002A, 0x0231, 0x0503, 0x0415, 0x0807, 0x002B, 0x071D, 0xFFFF },
  { 0x0130, 0x0214, 0x0508, 0x0401, 0x0133, 0x0225, 0x061B, 0xFFFF },
  { 0x0200, 0x0124, 0x0406, 0x0321, 0x0129, 0x100D, 0xFFFF },
  { 0x031A, 0x0D0B, 0x091F, 0xFFFF },
  { 0x0413, 0x0705, 0x0517, 0xFFFF },
  { 0x0212, 0x0034, 0x0323, 0x0035, 0x0227, 0xFFFF },
  { 0x0126, 0x080C, 0x0B09, 0xFFFF },
  { 0x0411, 0x0619, 0x1A0F, 0xFFFF },
  { 0x0210, 0x0331, 0x0603, 0x0515, 0x0907, 0x012B, 0xFFFF },
  { 0x0318, 0x002C, 0x0501, 0x0233, 0x0325, 0x071B, 0x002D, 0x081D, 0xFFFF },
  { 0x0132, 0x0302, 0x0229, 0x110D, 0xFFFF },
  { 0x0128, 0x041C, 0x0421, 0x0E0B, 0x0A1F, 0xFFFF },
  { 0x0220, 0x0513, 0x0617, 0xFFFF },
  { 0x0135, 0x0805, 0x0327, 0xFFFF },
  { 0x0316, 0x051E, 0x0D0E, 0x0423, 0xFFFF },
  { 0x0222, 0x0404, 0x070A, 0x0511, 0x0719, 0x0C09, 0x1B0F, 0xFFFF },
  { 0x0703, 0x0615, 0x0A07, 0x022B, 0xFFFF },
  { 0x012A, 0x0431, 0x0601, 0x0333, 0x012D, 0x091D, 0xFFFF },
  { 0x0230, 0x0314, 0x0036, 0x0608, 0x0425, 0x0037, 0x0329, 0x081B, 0x120D, 0xFFFF },
  { 0x0300, 0x0224, 0x0506, 0x0521, 0x0F0B, 0x0B1F, 0xFFFF },
  { 0x041A, 0x0613, 0x0717, 0xFFFF },
  { 0x0235, 0x0905, 0xFFFF },
  { 0x0312, 0x0134, 0x0523, 0x0427, 0xFFFF },
  { 0x0226, 0x090C, 0x002E, 0x0611, 0x0D09, 0x002F, 0xFFFF },
  { 0x0715, 0x0B07, 0x0819, 0x032B, 0x1C0F, 0xFFFF },
  { 0x0310, 0x0531, 0x0701, 0x0803, 0x022D, 0x0A1D, 0xFFFF },
  { 0x0418, 0x012C, 0x0433, 0x0525, 0x0137, 0x091B, 0x130D, 0xFFFF },
  { 0x0232, 0x0402, 0x0621, 0x0429, 0xFFFF },
  { 0x0228, 0x051C, 0x0713, 0x100B, 0x0C1F, 0xFFFF },
  { 0x0320, 0x0335, 0x0A05, 0x0817, 0xFFFF },
  { 0x0623, 0x0527, 0xFFFF },
  { 0x0416, 0x061E, 0x0E0E, 0x0711, 0x0E09, 0x012F, 0xFFFF },
  { 0x0322, 0x0504, 0x080A, 0x0919, 0x1D0F, 0xFFFF },
  { 0x0631, 0x0903, 0x0815, 0x0C07, 0x042B, 0x032D, 0x0B1D, 0xFFFF },
  { 0x022A, 0x0801, 0x0533, 0x0625, 0x0237, 0x0A1B, 0xFFFF },
  { 0x0330, 0x0414, 0x0136, 0x0708, 0x0721, 0x0529, 0x140D, 0xFFFF },
  { 0x0400, 0x0324, 0x0606, 0x0038, 0x0039, 0x110B, 0x0D1F, 0xFFFF },
  { 0x051A, 0x0813, 0x0B05, 0x0917, 0xFFFF },
  { 0x0723, 0x0435, 0x0627, 0xFFFF },
  { 0x0412, 0x0234, 0x0F09, 0x022F, 0xFFFF },
  { 0x0326, 0x0A0C, 0x012E, 0x0811, 0x0A19, 0x1E0F, 0xFFFF },
  { 0x0731, 0x0A03, 0x0915, 0x0D07, 0x052B, 0xFFFF },
  { 0x0410, 0x0901, 0x0633, 0x0725, 0x0337, 0x0B1B, 0x042D, 0x0C1D, 0xFFFF },
  { 0x0518, 0x022C, 0x0629, 0x150D, 0xFFFF },
  { 0x0332, 0x0502, 0x0821, 0x0139, 0x120B, 0x0E1F, 0xFFFF },
  { 0x0328, 0x061C, 0x0913, 0x0A17, 0xFFFF },
  { 0x0420, 0x0535, 0x0C05, 0x0727, 0xFFFF },
  { 0x0823, 0x032F, 0xFFFF },
  { 0x0516, 0x071E, 0x0F0E, 0x0911, 0x0B19, 0x1009, 0x1F0F, 0xFFFF },
  { 0x0422, 0x0604, 0x090A, 0x0B03, 0x0A15, 0x0E07, 0x062B, 0xFFFF },
  { 0x0831, 0x0A01, 0x0733, 0x052D, 0x0D1D, 0xFFFF },
  { 0x032A, 0x0825, 0x0437, 0x0729, 0x0C1B, 0x160D, 0xFFFF },
  { 0x0430, 0x0514, 0x0236, 0x0808, 0x0921, 0x0239, 0x130B, 0x0F1F, 0xFFFF },
  { 0x0500, 0x0424, 0x0706, 0x0138, 0x0A13, 0x0B17, 0xFFFF },
  { 0x061A, 0x0635, 0x0D05, 0xFFFF },
  { 0x0923, 0x0827, 0xFFFF },
  { 0x0512, 0x0334, 0x003A, 0x0A11, 0x1109, 0x003B, 0x042F, 0xFFFF },
  { 0x0426, 0x0B0C, 0x022E, 0x0B15, 0x0F07, 0x0C19, 0x072B, 0xFFFF },
  { 0x0931, 0x0B01, 0x0C03, 0x062D, 0x0E1D, 0xFFFF },
  { 0x0510, 0x0833, 0x0925, 0x0537, 0x0D1B, 0x170D, 0xFFFF },
  { 0x0618, 0x032C, 0x0A21, 0x0339, 0x0829, 0xFFFF },
  { 0x0432, 0x0602, 0x0B13, 0x140B, 0x101F, 0xFFFF },
  { 0x0428, 0x071C, 0x0735, 0x0E05, 0x0C17, 0xFFFF },
  { 0x0520, 0x0A23, 0x0927, 0xFFFF },
  { 0x0B11, 0x1209, 0x013B, 0x052F, 0xFFFF },
  { 0x0616, 0x081E, 0x0D19, 0xFFFF },
  { 0x0522, 0x0704, 0x0A0A, 0x0A31, 0x0D03, 0x0C15, 0x1007, 0x082B, 0x072D, 0x0F1D, 0xFFFF },
  { 0x0C01, 0x0933, 0x0A25, 0x0637, 0x0E1B, 0xFFFF },
  { 0x042A, 0x0B21, 0x0929, 0x180D, 0xFFFF },
  { 0x0530, 0x0614, 0x0336, 0x0908, 0x0439, 0x150B, 0x111F, 0xFFFF },
  { 0x0600, 0x0524, 0x0806, 0x0238, 0x0C13, 0x0F05, 0x0D17, 0xFFFF },
  { 0x071A, 0x0B23, 0x0835, 0x0A27, 0xFFFF },
  { 0x1309, 0x023B, 0x062F, 0xFFFF },
  { 0x0612, 0x0434, 0x013A, 0x0C11, 0x0E19, 0xFFFF },
  { 0x0526, 0x0C0C, 0x032E, 0x0B31, 0x0E03, 0x0D15, 0x1107, 0x092B, 0xFFFF },
  { 0x0D01, 0x0A33, 0x0B25, 0x0737, 0x0F1B, 0x082D, 0x101D, 0xFFFF },
  { 0x0610, 0x0A29, 0x190D, 0xFFFF },
  { 0x0718, 0x042C, 0x0C21, 0x0539, 0x160B, 0x121F, 0xFFFF },
  { 0x0532, 0x0702, 0x0D13, 0x0E17, 0xFFFF },
  { 0x0528, 0x081C, 0x0935, 0x1005, 0x0B27, 0xFFFF },
  { 0x0620, 0x0C23, 0x033B, 0x072F, 0xFFFF },
  { 0x0D11, 0x0F19, 0x1409, 0xFFFF },
  { 0x0716, 0x003C, 0x091E, 0x0F03, 0x0E15, 0x1207, 0x0A2B, 0x003D, 0xFFFF },
  { 0x0622, 0x0804, 0x0B0A, 0x0C31, 0x0E01, 0x0B33, 0x092D, 0x111D, 0xFFFF },
  { 0x0C25, 0x0837, 0x0B29, 0x101B, 0x1A0D, 0xFFFF },
  { 0x052A, 0x0D21, 0x0639, 0x170B, 0x131F, 0xFFFF },
  { 0x0630, 0x0714, 0x0436, 0x0A08, 0x0E13, 0x0F17, 0xFFFF },
  { 0x0700, 0x0624, 0x0906, 0x0338, 0x0A35, 0x1105, 0xFFFF },
  { 0x081A, 0x0D23, 0x0C27, 0xFFFF },
  { 0x0E11, 0x1509, 0x043B, 0x082F, 0xFFFF },
  { 0x0712, 0x0534, 0x023A, 0x0F15, 0x1307, 0x1019, 0x0B2B, 0x013D, 0xFFFF },
  { 0x0626, 0x0D0C, 0x042E, 0x0D31, 0x0F01, 0x1003, 0x0A2D, 0x121D, 0xFFFF },
  { 0x0C33, 0x0D25, 0x0937, 0x111B, 0x1B0D, 0xFFFF },
  { 0x0710, 0x0E21, 0x0739, 0x0C29, 0xFFFF },
  { 0x0818, 0x052C, 0x0F13, 0x180B, 0x141F, 0xFFFF },
  { 0x0632, 0x0802, 0x0B35, 0x1205, 0x1017, 0xFFFF },
  { 0x0628, 0x091C, 0x0E23, 0x0D27, 0xFFFF },
  { 0x0720, 0x0F11, 0x1609, 0x053B, 0x092F, 0xFFFF },
  { 0x1119, 0x023D, 0xFFFF },
  { 0x0816, 0x013C, 0x0A1E, 0x0E31, 0x1103, 0x1015, 0x1407, 0x0C2B, 0x0B2D, 0x131D, 0xFFFF },
  { 0x0722, 0x0904, 0x0C0A, 0x1001, 0x0D33, 0x0E25, 0x0A37, 0x121B, 0xFFFF },
  { 0x0F21, 0x0D29, 0x1C0D, 0xFFFF },
  { 0x062A, 0x0839, 0x190B, 0x151F, 0xFFFF },
  { 0x0730, 0x0814, 0x0536, 0x0B08, 0x1013, 0x1305, 0x1117, 0xFFFF },
  { 0x0800, 0x0724, 0x0A06, 0x0438, 0x0F23, 0x0C35, 0x0E27, 0xFFFF },
  { 0x091A, 0x1709, 0x063B, 0x0A2F, 0xFFFF },
  { 0x1011, 0x1219, 0x033D, 0xFFFF },
  { 0x0812, 0x0634, 0x033A, 0x0F31, 0x1203, 0x1115, 0x1507, 0x0D2B, 0xFFFF },
  { 0x0726, 0x0E0C, 0x052E, 0x1101, 0x0E33, 0x0F25, 0x0B37, 0x131B, 0x0C2D, 0x141D, 0xFFFF },
  { 0x0E29, 0x1D0D, 0xFFFF },
  { 0x0810, 0x1021, 0x0939, 0x1A0B, 0x161F, 0xFFFF },
  { 0x0918, 0x062C, 0x1113, 0x1217, 0xFFFF },
  { 0x0732, 0x0902, 0x0D35, 0x1405, 0x0F27, 0xFFFF },
  { 0x0728, 0x0A1C, 0x1023, 0x073B, 0x0B2F, 0xFFFF },
  { 0x0820, 0x1111, 0x1319, 0x1809, 0xFFFF },
  { 0x1303, 0x1215, 0x1607, 0x0E2B, 0x043D, 0xFFFF },
  { 0x0916, 0x023C, 0x0B1E, 0x1031, 0x1201, 0x0F33, 0x0D2D, 0x151D, 0xFFFF },
  { 0x0822, 0x0A04, 0x0D0A, 0x1025, 0x0C37, 0x0F29, 0x141B, 0x1E0D, 0xFFFF },
  { 0x1121, 0x0A39, 0x1B0B, 0x171F, 0xFFFF },
  { 0x072A, 0x1213, 0x1317, 0xFFFF },
  { 0x0830, 0x0914, 0x0636, 0x0C08, 0x0E35, 0x1505, 0xFFFF },
  { 0x0900, 0x0824, 0x0B06, 0x0538, 0x1123, 0x1027, 0xFFFF },
  { 0x0A1A, 0x1211, 0x1909, 0x083B, 0x0C2F, 0xFFFF },
  { 0x1315, 0x1707, 0x1419, 0x0F2B, 0x053D, 0xFFFF },
  { 0x0912, 0x0734, 0x043A, 0x1131, 0x1301, 0x1403, 0x0E2D, 0x161D, 0xFFFF },
  { 0x0826, 0x0F0C, 0x062E, 0x1033, 0x1125, 0x0D37, 0x151B, 0x1F0D, 0xFFFF },
  { 0x1221, 0x0B39, 0x1029, 0xFFFF },
  { 0x0910, 0x1313, 0x1C0B, 0x181F, 0xFFFF },
  { 0x0A18, 0x072C, 0x0F35, 0x1605, 0x1417, 0xFFFF },
  { 0x0832, 0x0A02, 0x1223, 0x1127, 0xFFFF },
  { 0x0828, 0x0B1C, 0x1311, 0x1A09, 0x093B, 0x0D2F, 0xFFFF },
  { 0x0920, 0x1519, 0x063D, 0xFFFF },
  { 0x1231, 0x1503, 0x1415, 0x1807, 0x102B, 0x0F2D, 0x171D, 0xFFFF },
  { 0x0A16, 0x033C, 0x0C1E, 0x1401, 0x1133, 0x1225, 0x0E37, 0x161B, 0xFFFF },
  { 0x0922, 0x0B04, 0x0E0A, 0x1321, 0x1129, 0xFFFF },
  { 0x0C39, 0x1D0B, 0x191F, 0xFFFF },
  { 0x082A, 0x1413, 0x1705, 0x1517, 0xFFFF },
  { 0x0930, 0x0A14, 0x0736, 0x0D08, 0x1323, 0x1035, 0x1227, 0xFFFF },
  { 0x0A00, 0x0924, 0x0C06, 0x0638, 0x1B09, 0x0A3B, 0x0E2F, 0xFFFF },
  { 0x0B1A, 0x1411, 0x1619, 0x073D, 0xFFFF },
  { 0x1331, 0x1603, 0x1515, 0x1907, 0x112B, 0xFFFF },
  { 0x0A12, 0x0834, 0x053A, 0x1501, 0x1233, 0x1325, 0x0F37, 0x171B, 0x102D, 0x181D, 0xFFFF },
  { 0x0926, 0x072E, 0x1229, 0xFFFF },
  { 0x1421, 0x0D39, 0x1E0B, 0x1A1F, 0xFFFF },
  { 0x0A10, 0x1513, 0x1617, 0xFFFF },
  { 0x0B18, 0x082C, 0x1135, 0x1805, 0x1327, 0xFFFF },
  { 0x0932, 0x0B02, 0x1423, 0x0B3B, 0x0F2F, 0xFFFF },
  { 0x0928, 0x0C1C, 0x1511, 0x1719, 0x1C09, 0xFFFF },
  { 0x0A20, 0x1703, 0x1615, 0x1A07, 0x122B, 0x083D, 0xFFFF },
  { 0x1431, 0x1601, 0x1333, 0x112D, 0x191D, 0xFFFF },
  { 0x0B16, 0x043C, 0x0D1E, 0x1425, 0x1037, 0x1329, 0x181B, 0xFFFF },
  { 0x0A22, 0x0C04, 0x0F0A, 0x1521, 0x0E39, 0x1F0B, 0x1B1F, 0xFFFF },
  { 0x1613, 0x1717, 0xFFFF },
  { 0x092A, 0x1235, 0x1905, 0xFFFF },
  { 0x0A30, 0x0B14, 0x0836, 0x0E08, 0x1523, 0x1427, 0xFFFF },
  { 0x0B00, 0x0A24, 0x0D06, 0x0738, 0x1611, 0x1D09, 0x0C3B, 0x102F, 0xFFFF },
  { 0x0C1A, 0x1715, 0x1B07, 0x1819, 0x132B, 0x093D, 0xFFFF },
  { 0x1531, 0x1701, 0x1803, 0x122D, 0x1A1D, 0xFFFF },
  { 0x0B12, 0x0934, 0x063A, 0x1433, 0x1525, 0x1137, 0x191B, 0xFFFF },
  { 0x0A26, 0x003E, 0x082E, 0x1621, 0x0F39, 0x1429, 0x003F, 0xFFFF },
  { 0x1713, 0x1C1F, 0xFFFF },
  { 0x0B10, 0x1335, 0x1A05, 0x1817, 0xFFFF },
  { 0x0C18, 0x092C, 0x1623, 0x1527, 0xFFFF },
  { 0x0A32, 0x0C02, 0x1711, 0x1E09, 0x0D3B, 0x112F, 0xFFFF },
  { 0x0A28, 0x0D1C, 0x1919, 0x0A3D, 0xFFFF },
  { 0x0B20, 0x1631, 0x1903, 0x1815, 0x1C07, 0x142B, 0x132D, 0x1B1D, 0xFFFF },
  { 0x1801, 0x1533, 0x1625, 0x1237, 0x1A1B, 0xFFFF },
  { 0x0C16, 0x053C, 0x0E1E, 0x1721, 0x1529, 0x013F, 0xFFFF },
  { 0x0B22, 0x0D04, 0x1039, 0x1D1F, 0xFFFF },
  { 0x1813, 0x1B05, 0x1917, 0xFFFF },
  { 0x0A2A, 0x1723, 0x1435, 0x1627, 0xFFFF },
  { 0x0B30, 0x0C14, 0x0936, 0x0F08, 0x1F09, 0x0E3B, 0x122F, 0xFFFF },
  { 0x0C00, 0x0B24, 0x0E06, 0x0838, 0x1811, 0x1A19, 0x0B3D, 0xFFFF },
  { 0x0D1A, 0x1731, 0x1A03, 0x1915, 0x1D07, 0x152B, 0xFFFF },
  { 0x1901, 0x1633, 0x1725, 0x1337, 0x1B1B, 0x142D, 0x1C1D, 0xFFFF },
  { 0x0C12, 0x0A34, 0x073A, 0x1629, 0x023F, 0xFFFF },
  { 0x0B26, 0x013E, 0x092E, 0x1821, 0x1139, 0x1E1F, 0xFFFF },
  { 0x1913, 0x1A17, 0xFFFF },
  { 0x0C10, 0x1535, 0x1C05, 0x1727, 0xFFFF },
  { 0x0D18, 0x0A2C, 0x1823, 0x0F3B, 0x132F, 0xFFFF },
  { 0x0B32, 0x0D02, 0x1911, 0x1B19, 0xFFFF },
  { 0x0B28, 0x0E1C, 0x1B03, 0x1A15, 0x1E07, 0x162B, 0x0C3D, 0xFFFF },
  { 0x0C20, 0x1831, 0x1A01, 0x1733, 0x152D, 0x1D1D, 0xFFFF },
  { 0x1825, 0x1437, 0x1729, 0x1C1B, 0x033F, 0xFFFF },
  { 0x0D16, 0x063C, 0x0F1E, 0x1921, 0x1239, 0x1F1F, 0xFFFF },
  { 0x0C22, 0x0E04, 0x1A13, 0x1B17, 0xFFFF },
  { 0x1635, 0x1D05, 0xFFFF },
  { 0x0B2A, 0x1923, 0x1827, 0xFFFF },
  { 0x0C30, 0x0D14, 0x0A36, 0x1A11, 0x103B, 0x142F, 0xFFFF },
  { 0x0D00, 0x0C24, 0x0F06, 0x0938, 0x1B15, 0x1F07, 0x1C19, 0x172B, 0x0D3D, 0xFFFF },
  { 0x0E1A, 0x1931, 0x1B01, 0x1C03, 0x162D, 0x1E1D, 0xFFFF },
  { 0x1833, 0x1925, 0x1537, 0x1D1B, 0xFFFF },
  { 0x0D12, 0x0B34, 0x083A, 0x1A21, 0x1339, 0x1829, 0x043F, 0xFFFF },
  { 0x0C26, 0x023E, 0x0A2E, 0x1B13, 0xFFFF },
  { 0x1735, 0x1E05, 0x1C17, 0xFFFF },
  { 0x0D10, 0x1A23, 0x1927, 0xFFFF },
  { 0x0E18, 0x0B2C, 0x1B11, 0x113B, 0x152F, 0xFFFF },
  { 0x0C32, 0x0E02, 0x1D19, 0x0E3D, 0xFFFF },
  { 0x0C28, 0x0F1C, 0x1A31, 0x1D03, 0x1C15, 0x182B, 0x172D, 0x1F1D, 0xFFFF },
  { 0x0D20, 0x1C01, 0x1933, 0x1A25, 0x1637, 0x1E1B, 0xFFFF },
  { 0x1B21, 0x1929, 0x053F, 0xFFFF },
  { 0x0E16, 0x073C, 0x1439, 0xFFFF },
  { 0x0D22, 0x0F04, 0x1C13, 0x1F05, 0x1D17, 0xFFFF },
  { 0x1B23, 0x1835, 0x1A27, 0xFFFF },
  { 0x0C2A, 0x123B, 0x162F, 0xFFFF },
  { 0x0D30, 0x0E14, 0x0B36, 0x1C11, 0x1E19, 0x0F3D, 0xFFFF },
  { 0x0E00, 0x0D24, 0x0A38, 0x1B31, 0x1E03, 0x1D15, 0x192B, 0xFFFF },
  { 0x0F1A, 0x1D01, 0x1A33, 0x1B25, 0x1737, 0x1F1B, 0x182D, 0xFFFF },
  { 0x1A29, 0x063F, 0xFFFF },
  { 0x0E12, 0x0C34, 0x093A, 0x1C21, 0x1539, 0xFFFF },
  { 0x0D26, 0x033E, 0x0B2E, 0x1D13, 0x1E17, 0xFFFF },
  { 0x1935, 0x1B27, 0xFFFF },
  { 0x0E10, 0x1C23, 0x133B, 0x172F, 0xFFFF },
  { 0x0F18, 0x0C2C, 0x1D11, 0x1F19, 0xFFFF },
  { 0x0D32, 0x0F02, 0x1F03, 0x1E15, 0x1A2B, 0x103D, 0xFFFF },
  { 0x0D28, 0x1C31, 0x1E01, 0x1B33, 0x192D, 0xFFFF },
  { 0x0E20, 0x1C25, 0x1837, 0x1B29, 0x073F, 0xFFFF },
  { 0x1D21, 0x1639, 0xFFFF },
  { 0x0F16, 0x083C, 0x1E13, 0x1F17, 0xFFFF },
  { 0x0E22, 0x1A35, 0xFFFF },
  { 0x1D23, 0x1C27, 0xFFFF },
  { 0x0D2A, 0x1E11, 0x143B, 0x182F, 0xFFFF },
  { 0x0E30, 0x0F14, 0x0C36, 0x1F15, 0x1B2B, 0x113D, 0xFFFF },
  { 0x0F00, 0x0E24, 0x0B38, 0x1D31, 0x1F01, 0x1A2D, 0xFFFF },
  { 0x1C33, 0x1D25, 0x1937, 0xFFFF },
  { 0x1E21, 0x1739, 0x1C29, 0x083F, 0xFFFF },
  { 0x0F12, 0x0D34, 0x0A3A, 0x1F13, 0xFFFF },
  { 0x0E26, 0x043E, 0x0C2E, 0x1B35, 0xFFFF },
  { 0x1E23, 0x1D27, 0xFFFF },
  { 0x0F10, 0x1F11, 0x153B, 0x192F, 0xFFFF },
  { 0x0D2C, 0x123D, 0xFFFF },
};

typedef union
{
   struct comp {
      unsigned char r;
      unsigned char g;
      unsigned char b;
      unsigned char a;
   } comp;

   unsigned int m_u32;
} color_quad_u8;

static inline int
rg_etc1_color_quad_u8_clamp(int v)
{
   /* FIXME: (From Wikipedia)
    * "In C, the result of right-shifting a negative value is implementation-defined"
    * The following code assumes right-shift will duplicate the sign bit.
    */
   if (v & 0xFFFFFF00U)
     v = ((~v) >> 31) & 0xFF;
   return v;
}

static inline void
rg_etc1_color_quad_u8_init(color_quad_u8 *color, int r, int g, int b, int alpha)
{
   color->comp.r = rg_etc1_color_quad_u8_clamp(r);
   color->comp.g = rg_etc1_color_quad_u8_clamp(g);
   color->comp.b = rg_etc1_color_quad_u8_clamp(b);
   color->comp.a = rg_etc1_color_quad_u8_clamp(alpha);
}
static inline void
rg_etc1_color_quad_u8_copy(color_quad_u8 *dst, const color_quad_u8 *src)
{
   dst->m_u32 = src->m_u32;
}

static inline void
rg_etc1_color_quad_u8_clear(color_quad_u8 *color)
{
   color->m_u32 = 0;
}

static inline unsigned int
rg_etc1_color_quad_u8_rgb_squared_distance(color_quad_u8 color1, color_quad_u8 color2)
{
   return SQUARE((color1.comp.r - color2.comp.r)) + SQUARE((color1.comp.g - color2.comp.g)) + SQUARE((color1.comp.b - color2.comp.b));
}

#if RG_ETC1_CONSTRAINED_SUBBLOCK
static inline void
rg_etc1_color_quad_u8_component_set(color_quad_u8 *color, unsigned char idx, unsigned char value)
{
   switch (idx)
     {
      case 0: color->comp.r = value; break;
      case 1: color->comp.g = value; break;
      case 2: color->comp.b = value; break;
      case 3: color->comp.a = value; break;
      default: abort();
     }
}
#endif

#if 0
static inline unsigned int
rg_etc1_color_quad_duplicate_init(unsigned char y, unsigned char alpha)
{
   return ARGB_JOIN(alpha, y, y, y);
}
#endif

static inline unsigned int
rg_etc1_color_quad_init(unsigned char r, unsigned char g, unsigned char b, unsigned char alpha)
{
   return ARGB_JOIN(alpha, r, g, b);
}

static inline unsigned int
rg_etc1_color_quad_set(unsigned int old_color, unsigned int new_color)
{
   return (new_color & ~A_MASK) | (old_color & A_MASK);
}

static inline void
rg_etc1_color_quad_get(unsigned int color, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *alpha)
{
   // Used for PACKING
   if (r) *r = R_VAL_GET(&color);
   if (g) *g = G_VAL_GET(&color);
   if (b) *b = B_VAL_GET(&color);
   if (alpha) *alpha = A_VAL_GET(&color);
}

#if RG_ETC1_CONSTRAINED_SUBBLOCK
static inline unsigned char
rg_etc1_color_quad_component_get(unsigned int color, unsigned char idx)
{
   switch (idx)
     {
      // FIXME: Untested code (RGBA vs BGRA)
      case 0: return R_VAL_GET(&color);
      case 1: return G_VAL_GET(&color);
      case 2: return B_VAL_GET(&color);
      case 3: return A_VAL_GET(&color);
      default: abort();
     }
   return 0;
}
#endif

#if 0
static inline unsigned int
rg_etc1_color_quad_component_set(unsigned int color, unsigned char idx, unsigned char value)
{
   unsigned char r, g, b, a;

   rg_etc1_color_quad_get(color, &r, &g, &b, &a);

   switch (idx)
     {
      case 0: r = value; break;
      case 1: g = value; break;
      case 2: b = value; break;
      case 3: a = value; break;
      default: abort();
     }

   return rg_etc1_color_quad_init(r, g, b, a);
}

static inline unsigned int
rg_etc1_color_quad_grayscale_set(unsigned int color, unsigned char l)
{
   unsigned char a;

   a = A_VAL_SET(&color);

   return rg_etc1_color_quad_init(l, l, l, a);
}

static inline unsigned int
rg_etc1_color_quad_clamp(unsigned int color, unsigned int low, unsigned high)
{
   unsigned char *c = (unsigned char *)&color;
   unsigned char *l = (unsigned char *)&low;
   unsigned char *h = (unsigned char *)&high;
   unsigned int i;

   for (i = 0; i < 4; i++)
     c[i] = CLAMP(c[i], l[i], h[i]);

   return color;
}

static inline unsigned int
rg_etc1_color_quad_component_clamp(unsigned int color, unsigned int low, unsigned high)
{
   unsigned char *c = (unsigned char *)&color;
   unsigned int i;

   for (i = 0; i < 4; i++)
     c[i] = CLAMP(c[i], low, high);

   return color;
}

// Returns CCIR 601 luma (consistent with color_utils::RGB_To_Y).
static inline unsigned char
rg_etc1_color_quad_luma601_get(unsigned int color)
{
   unsigned char r, g, b;
   rg_etc1_color_quad_get(color, &r, &g, &b, NULL);
   return ((19595U * r + 38470U * g + 7471U * b + 32768U) >> 16U);
}

// Returns REC 709 luma.
static inline unsigned char
rg_etc1_color_quad_luma709_get(unsigned int color)
{
   unsigned char r, g, b;
   rg_etc1_color_quad_get(color, &r, &g, &b, NULL);
   return ((13938U * r + 46869U * g + 4729U * b + 32768U) >> 16U);
}

static inline unsigned int
rg_etc1_color_quad_rgb_squared_distance(unsigned int color1, unsigned int color2)
{
   unsigned char r1, g1, b1;
   unsigned char r2, g2, b2;

   rg_etc1_color_quad_get(color1, &r1, &g1, &b1, NULL);
   rg_etc1_color_quad_get(color2, &r2, &g2, &b2, NULL);

   return SQUARE(r1 - r2) + SQUARE(g1 - g2) + SQUARE(b1 - b2);
}

static inline unsigned int
rg_etc1_color_quad_argb_squared_distance(unsigned int color1, unsigned int color2)
{
   unsigned char r1, g1, b1, a1;
   unsigned char r2, g2, b2, a2;

   rg_etc1_color_quad_get(color1, &r1, &g1, &b1, &a1);
   rg_etc1_color_quad_get(color2, &r2, &g2, &b2, &a2);

   return SQUARE(r1 - r2) + SQUARE(g1 - g2) + SQUARE(b1 - b2) + SQUARE(a1 - a1);
}

static inline unsigned char
rg_etc1_color_quad_rgb_equals(unsigned int color1, unsigned int color2)
{
   A_VAL_SET(&color1) = 0;
   A_VAL_SET(&color2) = 0;

   return color1 == color2;
}

static inline unsigned int
rg_etc1_color_quad_add(unsigned int color1, unsigned int color2)
{
   unsigned char *c1 = (unsigned char *)&color1;
   unsigned char *c2 = (unsigned char *)&color2;
   unsigned int i;

   for (i = 0; i < 4; i++)
     {
        unsigned short t;

        t = c1[i] + c2[i];
        c1[i] = (unsigned char) (MIN(t, 255));
     }

   return color1;
}

static inline unsigned int
rg_etc1_color_quad_del(unsigned int color1, unsigned int color2)
{
   unsigned char *c1 = (unsigned char *)&color1;
   unsigned char *c2 = (unsigned char *)&color2;
   unsigned int i;

   for (i = 0; i < 4; i++)
     {
        short t;

        t = c1[i] - c2[i];
        c1[i] = (unsigned char) (CLAMP(t, 0, 255));
     }

   return color1;
}
#endif

static inline void
rg_etc1_vec_init(float v[3], float s)
{
   v[0] = s; v[1] = s; v[2] = s;
}

static inline void
rg_etc1_vec_set(float v[3], float x, float y, float z)
{
   v[0] = x; v[1] = y; v[2] = z;
}

static inline void
rg_etc1_vec_copy(float dst[3], float src[3])
{
   dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
}

static inline void
rg_etc1_vec_add(float r[3], float a[3])
{
   unsigned int i;

   //for (i = 0; i < RG_ETC1_ARRAY_SIZE(r); i++)
   for (i = 0; i < 3; i++)
     r[i] += a[i];
}

static inline void
rg_etc1_vec_scale(float r[3], float s)
{
   unsigned int i;

   //for (i = 0; i < RG_ETC1_ARRAY_SIZE(r); i++)
   for (i = 0; i < 3; i++)
     r[i] *= s;
}

static inline unsigned char
rg_etc1_block_byte_bits_get(const unsigned char bytes[8], unsigned char offset, unsigned char num)
{
   const unsigned char byte_offset = 7 - (offset >> 3);
   const unsigned char byte_bit_offset = offset & 7;

   return (bytes[byte_offset] >> byte_bit_offset) & ((1 << num) - 1);
}

static inline void
rg_etc1_block_byte_bits_set(unsigned char bytes[8], unsigned char offset, unsigned char num, unsigned char bits)
{
   const unsigned char byte_offset = 7 - (offset >> 3);
   const unsigned char byte_bit_offset = offset & 7;
   const unsigned char mask = (1 << num) - 1;
   bytes[byte_offset] &= ~(mask << byte_bit_offset);
   bytes[byte_offset] |= (((!!bits) & 0x1) << byte_bit_offset);
}

static inline unsigned char
rg_etc1_block_flip_bit_get(const unsigned char bytes[8])
{
   return (bytes[3] & 1) != 0;
}

static inline void
rg_etc1_block_flip_bit_set(unsigned char bytes[8], unsigned char flip)
{
   bytes[3] &= ~1;
   bytes[3] |= (!!flip) & 0x1;
}

static inline unsigned char
rg_etc1_block_diff_bit_get(const unsigned char bytes[8])
{
   return (bytes[3] & 2) != 0;
}

static inline void
rg_etc1_block_diff_bit_set(unsigned char bytes[8], unsigned char diff)
{
   bytes[3] &= ~2;
   bytes[3] |= ((!!diff) & 0x1) << 1;
}

static inline void
rg_etc1_block_clear(unsigned char bytes[8])
{
   memset(bytes, 0, sizeof (unsigned char) * 8);
}

// Returns intensity modifier table (0-7) used by subblock subblock_id.
// subblock_id=0 left/top (CW 1), 1=right/bottom (CW 2)
static inline unsigned char
rg_etc1_block_inten_table_get(const unsigned char bytes[8], unsigned char subblock_id)
{
   const unsigned char offset = subblock_id ? 2 : 5;
   if (!(subblock_id < 2)) return 0; // ERROR CASE NO ASSERT IN EVAS CODE
   return (bytes[3] >> offset) & 7;
}

// Sets intensity modifier table (0-7) used by subblock subblock_id (0 or 1)
static inline void
rg_etc1_block_inten_table_set(unsigned char bytes[8], unsigned char subblock_id, unsigned char t)
{
   const unsigned char offset = subblock_id ? 2 : 5;
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(subblock_id < 2) || !(t < 8)) return ;
   bytes[3] &= ~(7 << offset);
   bytes[3] |= (t << offset);
}

// Returned selector value ranges from 0-3 and is a direct index into g_etc1_inten_tables.
static inline unsigned char
rg_etc1_block_selector_get(const unsigned char bytes[8], unsigned char x, unsigned char y)
{
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!((x | y) < 4)) return 0;

   const unsigned char bit_index = x * 4 + y;
   const unsigned char byte_bit_offset = bit_index & 7;
   const unsigned char *p = &bytes[7 - (bit_index >> 3)];
   const unsigned char lsb = (p[0] >> byte_bit_offset) & 1;
   const unsigned char msb = (p[-2] >> byte_bit_offset) & 1;
   const unsigned char val = lsb | (msb << 1);

   return rg_etc1_to_selector_index[val];
}

// Selector "val" ranges from 0-3 and is a direct index into g_etc1_inten_tables.
static inline void
rg_etc1_block_selector_set(unsigned char bytes[8], unsigned char x, unsigned char y, unsigned char val)
{
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!((x | y) < 4)) return ;

   const unsigned char bit_index = x * 4 + y;

   unsigned char *p = &bytes[7 - (bit_index >> 3)];

   const unsigned char byte_bit_offsets = bit_index & 7;
   const unsigned char mask = 1 << byte_bit_offsets;
   const unsigned char etc1_val = rg_etc_selector_index_to_etc1[val];

   const unsigned char lsb = etc1_val & 1;
   const unsigned char msb = etc1_val >> 1;

   p[0] &= ~mask;
   p[0] |= (lsb << byte_bit_offsets);

   p[-2] &= ~mask;
   p[-2] |= (msb << byte_bit_offsets);
}

static inline unsigned short
rg_etc_block_base4_color_get(const unsigned char bytes[8], unsigned char idx)
{
   unsigned short r, g, b;

   if (idx)
     {
        r = (bytes[0]) & ((1 << 4) - 1);
        g = (bytes[1]) & ((1 << 4) - 1);
        b = (bytes[2]) & ((1 << 4) - 1);
     }
   else
     {
        r = (bytes[0] >> 4) & ((1 << 4) - 1);
        g = (bytes[1] >> 4) & ((1 << 4) - 1);
        b = (bytes[2] >> 4) & ((1 << 4) - 1);
     }

   return b | (g << 4) | (r << 8);
}

static inline void
rg_etc1_block_base4_color_set(unsigned char bytes[8], unsigned char idx, unsigned short c)
{
   if (idx)
     {
        rg_etc1_block_byte_bits_set(bytes, cETC1AbsColor4R2BitOffset, 4, (c >> 8) & 15);
        rg_etc1_block_byte_bits_set(bytes, cETC1AbsColor4G2BitOffset, 4, (c >> 4) & 15);
        rg_etc1_block_byte_bits_set(bytes, cETC1AbsColor4B2BitOffset, 4, c & 15);
     }
   else
     {
        rg_etc1_block_byte_bits_set(bytes, cETC1AbsColor4R1BitOffset, 4, (c >> 8) & 15);
        rg_etc1_block_byte_bits_set(bytes, cETC1AbsColor4G1BitOffset, 4, (c >> 4) & 15);
        rg_etc1_block_byte_bits_set(bytes, cETC1AbsColor4B1BitOffset, 4, c & 15);
     }
}

static inline unsigned short
rg_etc1_block_base5_color_get(const unsigned char bytes[8])
{
   unsigned short r, g, b;

   r = (bytes[0] >> 3) & ((1 << 5) - 1);
   g = (bytes[1] >> 3) & ((1 << 5) - 1);
   b = (bytes[2] >> 3) & ((1 << 5) - 1);

   return b | (g << 5) | (r << 10);
}

static inline void
rg_etc1_block_base5_color_set(unsigned char bytes[8], unsigned short c)
{
   rg_etc1_block_byte_bits_set(bytes, cETC1BaseColor5RBitOffset, 5, (c >> 10) & 31);
   rg_etc1_block_byte_bits_set(bytes, cETC1BaseColor5GBitOffset, 5, (c >> 5) & 31);
   rg_etc1_block_byte_bits_set(bytes, cETC1BaseColor5BBitOffset, 5, c & 31);
}

static inline unsigned short
rg_etc1_block_delta3_color_get(const unsigned char bytes[8])
{
   unsigned short r, g, b;

   r = (bytes[0]) & ((1 << 3) - 1);
   g = (bytes[1]) & ((1 << 3) - 1);
   b = (bytes[2]) & ((1 << 3) - 1);

   return b | (g << 3) | (r << 6);
}

static inline void
rg_etc1_block_delta3_color_set(unsigned char bytes[8], unsigned short c)
{
   rg_etc1_block_byte_bits_set(bytes, cETC1DeltaColor3RBitOffset, 3, (c >> 6) & 7);
   rg_etc1_block_byte_bits_set(bytes, cETC1DeltaColor3GBitOffset, 3, (c >> 3) & 7);
   rg_etc1_block_byte_bits_set(bytes, cETC1DeltaColor3BBitOffset, 3, c & 7);
}

static inline unsigned short
rg_etc1_block_color5_component_pack(unsigned char r, unsigned char g, unsigned char b,
                                    unsigned char scaled, unsigned char bias)
{
   if (scaled)
     {
        r = (r * 31 + bias) / 255U;
        g = (g * 31 + bias) / 255U;
        b = (b * 31 + bias) / 255U;
     }

   r = MIN(r, 31);
   g = MIN(g, 31);
   b = MIN(b, 31);

   return b | (g << 5) | (r << 10);
}

static inline unsigned short
rg_etc1_block_color5_pack(unsigned int c, unsigned char scaled, unsigned char bias)
{
   unsigned char r, g, b;

   rg_etc1_color_quad_get(c, &r, &g, &b, NULL);
   return rg_etc1_block_color5_component_pack(r, g, b, scaled, bias);
}

static inline void
rg_etc1_block_color5_component_unpack(unsigned char *r, unsigned char *g, unsigned char *b,
                                      unsigned short packed_color5, unsigned scaled)
{
   *r = (packed_color5 >> 10) & 31;
   *g = (packed_color5 >> 5) & 31;
   *b = packed_color5 & 31;

   if (scaled)
     {
        *b = (*b << 3) | (*b >> 2);
        *g = (*g << 3) | (*g >> 2);
        *r = (*r << 3) | (*r >> 2);
     }
}

static inline unsigned int
rg_etc1_block_color5_unpack(unsigned short packed_color5, unsigned char scaled, unsigned char alpha)
{
   unsigned char r, g, b;

   rg_etc1_block_color5_component_unpack(&r, &g, &b, packed_color5, scaled);

   return rg_etc1_color_quad_init(r, g, b, MIN(alpha, 255));
}

// Inputs range from -4 to 3 (cETC1ColorDeltaMin to cETC1ColorDeltaMax)
static inline unsigned int
rg_etc1_block_delta3_pack(char r, char g, char b)
{
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!((r >= cETC1ColorDeltaMin) && (r <= cETC1ColorDeltaMax))) return 0;
   if (!((g >= cETC1ColorDeltaMin) && (g <= cETC1ColorDeltaMax))) return 0;
   if (!((b >= cETC1ColorDeltaMin) && (b <= cETC1ColorDeltaMax))) return 0;

   if (r < 0) r += 8;
   if (g < 0) g += 8;
   if (b < 0) b += 8;

   return b | (g << 3) | (r << 6);
}

// Results range from -4 to 3 (cETC1ColorDeltaMin to cETC1ColorDeltaMax)
static inline void
rg_etc1_block_delta3_unpack(char *r, char *g, char *b, unsigned short packed_delta3)
{
   *r = (packed_delta3 >> 6) & 7;
   *g = (packed_delta3 >> 3) & 7;
   *b = packed_delta3 & 7;

   if (*r >= 4) *r -= 8;
   if (*g >= 4) *g -= 8;
   if (*b >= 4) *b -= 8;
}

static inline unsigned short
rg_etc1_block_color4_component_pack(unsigned char r, unsigned char g, unsigned char b,
                                    unsigned char scaled, unsigned char bias)
{
   if (scaled)
     {
        r = (r * 15 + bias) / 255;
        g = (g * 15 + bias) / 255;
        b = (b * 15 + bias) / 255;
     }

   r = MIN(r, 15);
   g = MIN(g, 15);
   b = MIN(b, 15);

   return b | (g << 4) | (r << 8);
}

static inline unsigned short
rg_etc1_block_color4_pack(unsigned int color, unsigned char scaled, unsigned char bias)
{
   unsigned char r, g, b;

   rg_etc1_color_quad_get(color, &r, &g, &b, NULL);
   return rg_etc1_block_color4_component_pack(r, g, b, scaled, bias);
}

static inline void
rg_etc1_block_color4_component_unpack(unsigned char *r, unsigned char *g, unsigned char *b,
                                      unsigned short packed_color4, bool scaled)
{
   *r = (packed_color4 >> 8) & 15;
   *g = (packed_color4 >> 4) & 15;
   *b = packed_color4 & 15;

   if (scaled)
     {
        *b = (*b << 4) | *b;
        *g = (*g << 4) | *g;
        *r = (*r << 4) | *r;
     }
}

static inline unsigned int
rg_etc1_block_color4_unpack(unsigned short packed_color4, unsigned char scaled, unsigned char alpha)
{
   unsigned char r, g, b;

   rg_etc1_block_color4_component_unpack(&r, &g, &b, packed_color4, scaled);

   return rg_etc1_color_quad_init(r, g, b, alpha);
}


static inline unsigned char
rg_etc1_block_color5_delta3_component_unpack(unsigned char *r, unsigned char *g, unsigned char *b,
                                             unsigned short packed_color5, unsigned short packed_delta3, bool scaled)
{
   unsigned char success = 1;
   char dc_r, dc_g, dc_b;

   rg_etc1_block_delta3_unpack(&dc_r, &dc_g, &dc_b, packed_delta3);

   *r = ((packed_color5 >> 10) & 31) + dc_r;
   *g = ((packed_color5 >> 5) & 31) + dc_g;
   *b = (packed_color5 & 31) + dc_b;

   if ((*r | *g | *b) > 31)
     {
        success = 0;
        *r = MIN(*r, 31);
        *g = MIN(*g, 31);
        *b = MIN(*b, 31);
     }

   if (scaled)
     {
        *r = (*r << 3) | (*r >> 2);
        *g = (*g << 3) | (*g >> 2);
        *b = (*b << 3) | (*b >> 2);
     }

   return success;
}


static inline unsigned char
rg_etc1_block_color5_delta3_unpack(unsigned int *result,
                                   unsigned short packed_color5, unsigned short packed_delta3,
                                   bool scaled, unsigned char alpha)
{
   unsigned char success;
   unsigned char r, g, b;

   success = rg_etc1_block_color5_delta3_component_unpack(&r, &g, &b, packed_color5, packed_delta3, scaled);

   *result = rg_etc1_color_quad_init(r, g, b, alpha);
   return success;
}

static inline void
rg_etc1_block_sublock_diff(unsigned int dst[4], const int *pInten_modifer_table,
                           unsigned char r, unsigned char g, unsigned char b)
{
   int i;

   for (i = 0; i < 4; i++)
     {
        int y = pInten_modifer_table[i];

        dst[i] = rg_etc1_color_quad_init(CLAMP((int)r + y, 0, 255),
                                         CLAMP((int) g + y,0, 255),
                                         CLAMP((int) b + y,0, 255),
                                         255);
     }
}

static inline void
rg_etc1_block_subblock_color5_diff_get(unsigned int dst[4], unsigned short packed_color5, unsigned char table_idx)
{
   const int *pInten_modifer_table;
   unsigned char r, g, b;

   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(table_idx < cETC1IntenModifierValues)) return ;

   rg_etc1_block_color5_component_unpack(&r, &g, &b, packed_color5, 1);

   pInten_modifer_table = &rg_etc1_inten_tables[table_idx][0];
   rg_etc1_block_sublock_diff(dst, pInten_modifer_table, r, g, b);
}

static inline unsigned char
rg_etc1_block_subblock_color5_delta3_diff_get(unsigned int dst[4],
                                              unsigned short packed_color5, unsigned short packed_delta3,
                                              unsigned char table_idx)
{
   const int *pInten_modifer_table;
   unsigned char r, g, b;
   unsigned char success;

   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(table_idx < cETC1IntenModifierValues))
     {
        fprintf(stderr, "table_idx %i < %i\n", table_idx, cETC1IntenModifierValues);
        return 0;
     }

   success = rg_etc1_block_color5_delta3_component_unpack(&r, &g, &b, packed_color5, packed_delta3, 1);

   pInten_modifer_table = &rg_etc1_inten_tables[table_idx][0];
   rg_etc1_block_sublock_diff(dst, pInten_modifer_table, r, g, b);

   return success;
}

static inline void
rg_etc1_block_subblock_color4_abs_get(unsigned int dst[4], unsigned short packed_color4, unsigned char table_idx)
{
   const int *pInten_modifer_table;
   unsigned char r, g, b;

   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(table_idx < cETC1IntenModifierValues)) return ;

   rg_etc1_block_color4_component_unpack(&r, &g, &b, packed_color4, 1);

   pInten_modifer_table = &rg_etc1_inten_tables[table_idx][0];
   rg_etc1_block_sublock_diff(dst, pInten_modifer_table, r, g, b);
}

// This is the exported function to unpack a block
bool
rg_etc1_unpack_block(const void *ETC1_block, unsigned int *pDst_pixels_BGRA, bool preserve_alpha)
{
   unsigned char diff_flag, flip_flag, table_index0, table_index1;
   unsigned int subblock_colors0[4] = { 0 };
   unsigned int subblock_colors1[4] = { 0 };
   unsigned char success = 1;
   const unsigned char *bytes;
   bytes = (unsigned char *)ETC1_block;

   diff_flag = rg_etc1_block_diff_bit_get(ETC1_block);
   flip_flag = rg_etc1_block_flip_bit_get(ETC1_block);
   table_index0 = (bytes[3] >> 5) & 7;
   table_index1 = (bytes[3] >> 2) & 7;

   if (diff_flag)
     {
        unsigned short base_color5, delta_color3;

        base_color5 = rg_etc1_block_base5_color_get(ETC1_block);
        delta_color3 = rg_etc1_block_delta3_color_get(ETC1_block);

        rg_etc1_block_subblock_color5_diff_get(subblock_colors0, base_color5, table_index0);
        success = rg_etc1_block_subblock_color5_delta3_diff_get(subblock_colors1,
                                                                base_color5, delta_color3,
                                                                table_index1);
     }
   else
     {
        unsigned short base_color4_0, base_color4_1;

        base_color4_0 = rg_etc_block_base4_color_get(ETC1_block, 0);
        base_color4_1 = rg_etc_block_base4_color_get(ETC1_block, 1);

        rg_etc1_block_subblock_color4_abs_get(subblock_colors0, base_color4_0, table_index0);
        rg_etc1_block_subblock_color4_abs_get(subblock_colors1, base_color4_1, table_index1);
     }

   // FIXME: preserve_alpha and continue
   // Block is either :
   //  0000
   //  0000
   //  1111
   //  1111
   // or :
   //  0011
   //  0011
   //  0011
   //  0011
   unsigned char val0 = (bytes[7] & 1) | ((bytes[5] & 1) << 1);
   unsigned char val1 = ((bytes[7] >> 4) & 1) | (((bytes[5] >> 4) & 1) << 1);
   unsigned char val2 = (bytes[6] & 1) | ((bytes[4] & 1) << 1);
   unsigned char val3 = ((bytes[6] >> 4) & 1) | (((bytes[4] >> 4) & 1) << 1);
   unsigned char val4 = ((bytes[7] >> 1) & 1) | (((bytes[5] >> 1) & 1) << 1);
   unsigned char val5 = ((bytes[7] >> 5) & 1) | (((bytes[5] >> 5) & 1) << 1);
   unsigned char val6 = ((bytes[6] >> 1) & 1) | (((bytes[4] >> 1) & 1) << 1);
   unsigned char val7 = ((bytes[6] >> 5) & 1) | (((bytes[4] >> 5) & 1) << 1);
   unsigned char val8 = ((bytes[7] >> 2) & 1) | (((bytes[5] >> 2) & 1) << 1);
   unsigned char val9 = ((bytes[7] >> 6) & 1) | (((bytes[5] >> 6) & 1) << 1);
   unsigned char val10 = ((bytes[6] >> 2) & 1) | (((bytes[4] >> 2) & 1) << 1);
   unsigned char val11 = ((bytes[6] >> 6) & 1) | (((bytes[4] >> 6) & 1) << 1);
   unsigned char val12 = ((bytes[7] >> 3) & 1) | (((bytes[5] >> 3) & 1) << 1);
   unsigned char val13 = ((bytes[7] >> 7) & 1) | (((bytes[5] >> 7) & 1) << 1);
   unsigned char val14 = ((bytes[6] >> 3) & 1) | (((bytes[4] >> 3) & 1) << 1);
   unsigned char val15 = ((bytes[6] >> 7) & 1) | (((bytes[4] >> 7) & 1) << 1);

   if (preserve_alpha)   // Depending on flip_flag.
      {
       if (flip_flag)
         {
            pDst_pixels_BGRA[0] = rg_etc1_color_quad_set(pDst_pixels_BGRA[0],
                                               subblock_colors0[rg_etc1_to_selector_index[val0]]);
            pDst_pixels_BGRA[1] = rg_etc1_color_quad_set(pDst_pixels_BGRA[1],
                                               subblock_colors0[rg_etc1_to_selector_index[val1]]);
            pDst_pixels_BGRA[2] = rg_etc1_color_quad_set(pDst_pixels_BGRA[2],
                                               subblock_colors0[rg_etc1_to_selector_index[val2]]);
            pDst_pixels_BGRA[3] = rg_etc1_color_quad_set(pDst_pixels_BGRA[3],
                                               subblock_colors0[rg_etc1_to_selector_index[val3]]);
            pDst_pixels_BGRA[4] = rg_etc1_color_quad_set(pDst_pixels_BGRA[4],
                                               subblock_colors0[rg_etc1_to_selector_index[val4]]);
            pDst_pixels_BGRA[5] = rg_etc1_color_quad_set(pDst_pixels_BGRA[5],
                                               subblock_colors0[rg_etc1_to_selector_index[val5]]);
            pDst_pixels_BGRA[6] = rg_etc1_color_quad_set(pDst_pixels_BGRA[6],
                                               subblock_colors0[rg_etc1_to_selector_index[val6]]);
            pDst_pixels_BGRA[7] = rg_etc1_color_quad_set(pDst_pixels_BGRA[7],
                                               subblock_colors0[rg_etc1_to_selector_index[val7]]);
            pDst_pixels_BGRA[8] = rg_etc1_color_quad_set(pDst_pixels_BGRA[8],
                                               subblock_colors1[rg_etc1_to_selector_index[val8]]);
            pDst_pixels_BGRA[9] = rg_etc1_color_quad_set(pDst_pixels_BGRA[9],
                                               subblock_colors1[rg_etc1_to_selector_index[val9]]);
            pDst_pixels_BGRA[10] = rg_etc1_color_quad_set(pDst_pixels_BGRA[10],
                                               subblock_colors1[rg_etc1_to_selector_index[val10]]);
            pDst_pixels_BGRA[11] = rg_etc1_color_quad_set(pDst_pixels_BGRA[11],
                                               subblock_colors1[rg_etc1_to_selector_index[val11]]);
            pDst_pixels_BGRA[12] = rg_etc1_color_quad_set(pDst_pixels_BGRA[12],
                                               subblock_colors1[rg_etc1_to_selector_index[val12]]);
            pDst_pixels_BGRA[13] = rg_etc1_color_quad_set(pDst_pixels_BGRA[13],
                                               subblock_colors1[rg_etc1_to_selector_index[val13]]);
            pDst_pixels_BGRA[14] = rg_etc1_color_quad_set(pDst_pixels_BGRA[14],
                                               subblock_colors1[rg_etc1_to_selector_index[val14]]);
            pDst_pixels_BGRA[15] = rg_etc1_color_quad_set(pDst_pixels_BGRA[15],
                                               subblock_colors1[rg_etc1_to_selector_index[val15]]);
         }
       else
         {
            pDst_pixels_BGRA[0] = rg_etc1_color_quad_set(pDst_pixels_BGRA[0],
                                               subblock_colors0[rg_etc1_to_selector_index[val0]]);
            pDst_pixels_BGRA[1] = rg_etc1_color_quad_set(pDst_pixels_BGRA[1],
                                               subblock_colors0[rg_etc1_to_selector_index[val1]]);
            pDst_pixels_BGRA[2] = rg_etc1_color_quad_set(pDst_pixels_BGRA[2],
                                               subblock_colors1[rg_etc1_to_selector_index[val2]]);
            pDst_pixels_BGRA[3] = rg_etc1_color_quad_set(pDst_pixels_BGRA[3],
                                               subblock_colors1[rg_etc1_to_selector_index[val3]]);
            pDst_pixels_BGRA[4] = rg_etc1_color_quad_set(pDst_pixels_BGRA[4],
                                               subblock_colors0[rg_etc1_to_selector_index[val4]]);
            pDst_pixels_BGRA[5] = rg_etc1_color_quad_set(pDst_pixels_BGRA[5],
                                               subblock_colors0[rg_etc1_to_selector_index[val5]]);
            pDst_pixels_BGRA[6] = rg_etc1_color_quad_set(pDst_pixels_BGRA[6],
                                               subblock_colors1[rg_etc1_to_selector_index[val6]]);
            pDst_pixels_BGRA[7] = rg_etc1_color_quad_set(pDst_pixels_BGRA[7],
                                               subblock_colors1[rg_etc1_to_selector_index[val7]]);
            pDst_pixels_BGRA[8] = rg_etc1_color_quad_set(pDst_pixels_BGRA[8],
                                               subblock_colors0[rg_etc1_to_selector_index[val8]]);
            pDst_pixels_BGRA[9] = rg_etc1_color_quad_set(pDst_pixels_BGRA[9],
                                               subblock_colors0[rg_etc1_to_selector_index[val9]]);
            pDst_pixels_BGRA[10] = rg_etc1_color_quad_set(pDst_pixels_BGRA[10],
                                               subblock_colors1[rg_etc1_to_selector_index[val10]]);
            pDst_pixels_BGRA[11] = rg_etc1_color_quad_set(pDst_pixels_BGRA[11],
                                               subblock_colors1[rg_etc1_to_selector_index[val11]]);
            pDst_pixels_BGRA[12] = rg_etc1_color_quad_set(pDst_pixels_BGRA[12],
                                               subblock_colors0[rg_etc1_to_selector_index[val12]]);
            pDst_pixels_BGRA[13] = rg_etc1_color_quad_set(pDst_pixels_BGRA[13],
                                               subblock_colors0[rg_etc1_to_selector_index[val13]]);
            pDst_pixels_BGRA[14] = rg_etc1_color_quad_set(pDst_pixels_BGRA[14],
                                               subblock_colors1[rg_etc1_to_selector_index[val14]]);
            pDst_pixels_BGRA[15] = rg_etc1_color_quad_set(pDst_pixels_BGRA[15],
                                               subblock_colors1[rg_etc1_to_selector_index[val15]]);
         }
      }
      else
      {
       if (flip_flag)
         {
            pDst_pixels_BGRA[0] = subblock_colors0[rg_etc1_to_selector_index[val0]];
            pDst_pixels_BGRA[1] = subblock_colors0[rg_etc1_to_selector_index[val1]];
            pDst_pixels_BGRA[2] = subblock_colors0[rg_etc1_to_selector_index[val2]];
            pDst_pixels_BGRA[3] = subblock_colors0[rg_etc1_to_selector_index[val3]];
            pDst_pixels_BGRA[4] = subblock_colors0[rg_etc1_to_selector_index[val4]];
            pDst_pixels_BGRA[5] = subblock_colors0[rg_etc1_to_selector_index[val5]];
            pDst_pixels_BGRA[6] = subblock_colors0[rg_etc1_to_selector_index[val6]];
            pDst_pixels_BGRA[7] = subblock_colors0[rg_etc1_to_selector_index[val7]];
            pDst_pixels_BGRA[8] = subblock_colors1[rg_etc1_to_selector_index[val8]];
            pDst_pixels_BGRA[9] = subblock_colors1[rg_etc1_to_selector_index[val9]];
            pDst_pixels_BGRA[10] = subblock_colors1[rg_etc1_to_selector_index[val10]];
            pDst_pixels_BGRA[11] = subblock_colors1[rg_etc1_to_selector_index[val11]];
            pDst_pixels_BGRA[12] = subblock_colors1[rg_etc1_to_selector_index[val12]];
            pDst_pixels_BGRA[13] = subblock_colors1[rg_etc1_to_selector_index[val13]];
            pDst_pixels_BGRA[14] = subblock_colors1[rg_etc1_to_selector_index[val14]];
            pDst_pixels_BGRA[15] = subblock_colors1[rg_etc1_to_selector_index[val15]];
         }
       else
         {
            pDst_pixels_BGRA[0] = subblock_colors0[rg_etc1_to_selector_index[val0]];
            pDst_pixels_BGRA[1] = subblock_colors0[rg_etc1_to_selector_index[val1]];
            pDst_pixels_BGRA[2] = subblock_colors1[rg_etc1_to_selector_index[val2]];
            pDst_pixels_BGRA[3] = subblock_colors1[rg_etc1_to_selector_index[val3]];
            pDst_pixels_BGRA[4] = subblock_colors0[rg_etc1_to_selector_index[val4]];
            pDst_pixels_BGRA[5] = subblock_colors0[rg_etc1_to_selector_index[val5]];
            pDst_pixels_BGRA[6] = subblock_colors1[rg_etc1_to_selector_index[val6]];
            pDst_pixels_BGRA[7] = subblock_colors1[rg_etc1_to_selector_index[val7]];
            pDst_pixels_BGRA[8] = subblock_colors0[rg_etc1_to_selector_index[val8]];
            pDst_pixels_BGRA[9] = subblock_colors0[rg_etc1_to_selector_index[val9]];
            pDst_pixels_BGRA[10] = subblock_colors1[rg_etc1_to_selector_index[val10]];
            pDst_pixels_BGRA[11] = subblock_colors1[rg_etc1_to_selector_index[val11]];
            pDst_pixels_BGRA[12] = subblock_colors0[rg_etc1_to_selector_index[val12]];
            pDst_pixels_BGRA[13] = subblock_colors0[rg_etc1_to_selector_index[val13]];
            pDst_pixels_BGRA[14] = subblock_colors1[rg_etc1_to_selector_index[val14]];
            pDst_pixels_BGRA[15] = subblock_colors1[rg_etc1_to_selector_index[val15]];
         }
      }

   return success;
}

// NOTE: Most of the following loop could be unrolled, but for sanity and readability, I did
// prefer to stick to simpler solution.
static inline unsigned int *
rg_etc1_indirect_radix_sort(unsigned int num_indices, unsigned int pIndices0[8], unsigned int pIndices1[8],
                            const unsigned short pKeys[8], unsigned int key_ofs, unsigned char key_size,
                            unsigned char init_indices)
{
   unsigned int hist[256 * 4];
   unsigned int *p;
   unsigned int *q;
   unsigned int *pCur;
   unsigned int *pNew;
   unsigned int key;
   unsigned int pass;

   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(key_ofs < sizeof(unsigned int) * 8)) return NULL;
   if (!((key_size >= 1) && (key_size <= 4))) return NULL;

   if (init_indices)
     {
        unsigned int i;

        p = pIndices0;
        for (i = 0; i < num_indices; p++, i++)
          *p = i;
     }

   memset(hist, 0, sizeof (hist));

#define RG_ETC1_GET_KEY(p) (*(const unsigned int*)((const unsigned char*)(pKeys + *(p)) + key_ofs))
#define RG_ETC1_GET_KEY_FROM_INDEX(i) (*(const unsigned int*)((const unsigned char*)(pKeys + (i)) + key_ofs))

   switch (key_size)
     {
      case 4:
         p = pIndices0;
         q = pIndices0 + num_indices;

         for (; p != q; p++)
           {
              key = RG_ETC1_GET_KEY(p);

              hist[        key        & 0xFF]++;
              hist[256 + ((key >>  8) & 0xFF)]++;
              hist[512 + ((key >> 16) & 0xFF)]++;
              hist[768 + ((key >> 24) & 0xFF)]++;
           }
         break;
      case 3:
         p = pIndices0;
         q = pIndices0 + num_indices;

         for ( ; p != q; p++)
           {
              key = RG_ETC1_GET_KEY(p);

              hist[        key        & 0xFF]++;
              hist[256 + ((key >>  8) & 0xFF)]++;
              hist[512 + ((key >> 16) & 0xFF)]++;
           }
         break;
      case 2:
         p = pIndices0;
         q = pIndices0 + num_indices;

         for ( ; p != q; p++)
           {
              key = RG_ETC1_GET_KEY(p);

              hist[        key        & 0xFF]++;
              hist[256 + ((key >>  8) & 0xFF)]++;
           }
         break;
      case 1:
         p = pIndices0;
         q = pIndices0 + num_indices;

         for ( ; p != q; p++)
           {
              key = RG_ETC1_GET_KEY(p);

              hist[key & 0xFF]++;
           }
         break;
      default:
         abort();
     }

   pCur = pIndices0;
   pNew = pIndices1;

   for (pass = 0; pass < key_size; pass++)
     {
        unsigned int offsets[256];
        const unsigned int *pHist = &hist[pass << 8];
        unsigned int *t;
        unsigned int cur_ofs = 0;
        unsigned int i;
        unsigned int pass_shift = pass << 3;

        for (i = 0; i < 256; i++)
          {
             offsets[i] = cur_ofs;
             cur_ofs += pHist[i];
          }

        q = pCur + num_indices;
        for (p = pCur; p != q; p++)
          {
             unsigned int dst_offset;
             unsigned int idx = p[0];
             unsigned int c = (RG_ETC1_GET_KEY_FROM_INDEX(idx) >> pass_shift) & 0xFF;

             dst_offset = offsets[c]++;
             pNew[dst_offset] = idx;
          }

        t = pCur;
        pCur = pNew;
        pNew = t;
     }

   return pCur;
}

typedef struct _Etc1_Solution_Coordinates Etc1_Solution_Coordinates;
struct _Etc1_Solution_Coordinates
{
   color_quad_u8 m_unscaled_color;
   unsigned int m_inten_table;
   unsigned char m_color4;
};

static inline void
rg_etc1_solution_coordinates_component_set(Etc1_Solution_Coordinates *solution,
                                           int r, int g, int b,
                                           unsigned int inten_table, unsigned char color4)
{
   rg_etc1_color_quad_u8_init(&solution->m_unscaled_color, r, g, b, 255);
   solution->m_inten_table = inten_table;
   solution->m_color4 = color4;
}

static inline void
rg_etc1_solution_coordinates_set(Etc1_Solution_Coordinates *solution,
                                 color_quad_u8 unscaled_color, unsigned int inten_table, unsigned char color4)
{
   rg_etc1_color_quad_u8_copy(&solution->m_unscaled_color, &unscaled_color);
   solution->m_inten_table = inten_table;
   solution->m_color4 = color4;
}

static inline void
rg_etc1_solution_coordinates_clear(Etc1_Solution_Coordinates *solution)
{
   memset(solution, 0, sizeof (Etc1_Solution_Coordinates));
}

static inline void
rg_etc1_solution_coordinates_component_get(const Etc1_Solution_Coordinates *solution,
                                           unsigned char *r, unsigned char *g, unsigned char *b)
{
   *r = solution->m_unscaled_color.comp.r;
   *g = solution->m_unscaled_color.comp.g;
   *b = solution->m_unscaled_color.comp.b;

   if (solution->m_color4)
     {
        *r = *r | (*r << 4);
        *g = *g | (*g << 4);
        *b = *b | (*b << 4);
     }
   else
     {
        *r = (*r >> 2) | (*r << 3);
        *g = (*g >> 2) | (*g << 3);
        *b = (*b >> 2) | (*b << 3);
     }
}

static inline void
rg_etc1_solution_coordinates_get_scaled_color(color_quad_u8 *color, const Etc1_Solution_Coordinates *coords)
{
   unsigned char br, bg, bb;

   rg_etc1_solution_coordinates_component_get(coords, &br, &bg, &bb);
   rg_etc1_color_quad_u8_init(color, br, bg, bb, 255);
}

static inline void
rg_etc1_solution_coordinates_block_colors_get(const Etc1_Solution_Coordinates *coords, color_quad_u8 colors[4])
{
   const int* pInten_table = rg_etc1_inten_tables[coords->m_inten_table];
   unsigned char i;
   unsigned char br, bg, bb;

   rg_etc1_solution_coordinates_component_get(coords, &br, &bg, &bb);

   for (i = 0; i < 4; i++)
     rg_etc1_color_quad_u8_init(&colors[i], br + pInten_table[i], bg + pInten_table[i], bb + pInten_table[i], 255);
}

static inline void
rg_etc1_pack_params_clear(rg_etc1_pack_params *params)
{
   params->m_quality = rg_etc1_high_quality;
   params->m_dithering = EINA_FALSE;
}

static const int rg_etc1_default_scan_delta[] = { 0 };

typedef struct _rg_etc1_optimizer_params rg_etc1_optimizer_params;
struct _rg_etc1_optimizer_params
{
   rg_etc1_pack_params *base_params;
   uint m_num_src_pixels;
   const color_quad_u8* m_pSrc_pixels;

   bool m_use_color4;
   const int* m_pScan_deltas;
   uint m_scan_delta_size;

   color_quad_u8 m_base_color5;
   bool m_constrain_against_base_color5;
};

static inline void
rg_etc1_optimizer_params_clean(rg_etc1_optimizer_params *params)
{
   params->m_num_src_pixels = 0;
   params->m_pSrc_pixels = 0;

   params->m_use_color4 = EINA_FALSE;
   params->m_pScan_deltas = rg_etc1_default_scan_delta;
   params->m_scan_delta_size = 1;

   rg_etc1_color_quad_u8_clear(&params->m_base_color5);
   params->m_constrain_against_base_color5 = EINA_FALSE;
}

static inline void
rg_etc1_optimizer_params_base_clear(rg_etc1_optimizer_params *params)
{
   rg_etc1_pack_params_clear(params->base_params);
   rg_etc1_optimizer_params_clean(params);
}

typedef struct
{
   uint64 m_error;
   color_quad_u8 m_block_color_unscaled;
   uint m_block_inten_table;
   uint m_n;
   uint8* m_pSelectors;
   bool m_block_color4;
} rg_etc1_optimizer_results;

static inline void
rg_etc1_optimizer_results_duplicate(rg_etc1_optimizer_results *dst, const rg_etc1_optimizer_results *src)
{
   rg_etc1_color_quad_u8_copy(&dst->m_block_color_unscaled,&src->m_block_color_unscaled);
   dst->m_block_color4 = src->m_block_color4;
   dst->m_block_inten_table = src->m_block_inten_table;
   dst->m_error = src->m_error;
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(dst->m_n == src->m_n)) return ;
   memcpy(dst->m_pSelectors, src->m_pSelectors, src->m_n);
}

typedef struct
{
   Etc1_Solution_Coordinates m_coords;
   uint8                      m_selectors[8];
   uint64                     m_error;
   bool                       m_valid;
} rg_etc1_potential_solution;

static inline void
rg_etc1_potential_solution_clear(rg_etc1_potential_solution *solution)
{
   rg_etc1_solution_coordinates_clear(&solution->m_coords);
   solution->m_error = cUINT64_MAX;
   solution->m_valid = EINA_FALSE;
}

typedef struct
{
   const rg_etc1_optimizer_params* m_pParams;
   rg_etc1_optimizer_results* m_pResult;

   int m_limit;

   float m_avg_color[3];
   int m_br, m_bg, m_bb;
   uint16 m_luma[8];
   uint32 m_sorted_luma[2][8];
   const uint32* m_pSorted_luma_indices;
   uint32* m_pSorted_luma;

   uint8 m_selectors[8];
   uint8 m_best_selectors[8];

   rg_etc1_potential_solution m_best_solution;
   rg_etc1_potential_solution m_trial_solution;
   uint8 m_temp_selectors[8];
}rg_etc1_optimizer;

static inline void
rg_etc1_optimizer_clear(rg_etc1_optimizer *optimizer)
{
   optimizer->m_pParams = NULL;
   optimizer->m_pResult = NULL;
   optimizer->m_pSorted_luma = NULL;
   optimizer->m_pSorted_luma_indices = NULL;
   optimizer->m_br = optimizer->m_bg = optimizer->m_bb = 0;
   rg_etc1_potential_solution_clear(&optimizer->m_best_solution);
   rg_etc1_potential_solution_clear(&optimizer->m_trial_solution);
}

static bool rg_etc1_optimizer_evaluate_solution(rg_etc1_optimizer *optimizer, const Etc1_Solution_Coordinates* coords,
                                                rg_etc1_potential_solution* trial_solution,
                                                rg_etc1_potential_solution* pBest_solution);
static bool rg_etc1_optimizer_evaluate_solution_fast(rg_etc1_optimizer *optimizer,const Etc1_Solution_Coordinates *coords,
                                                     rg_etc1_potential_solution *trial_solution,
                                                     rg_etc1_potential_solution *pBest_solution);

static bool
rg_etc1_optimizer_compute(rg_etc1_optimizer *optimizer)
{
   const uint n = optimizer->m_pParams->m_num_src_pixels;
   const int scan_delta_size = optimizer->m_pParams->m_scan_delta_size;
   int zdi;

   // Scan through a subset of the 3D lattice centered around the avg block color trying each 3D (555 or 444) lattice point as a potential block color.
   // Each time a better solution is found try to refine the current solution's block color based of the current selectors and intensity table index.
   for (zdi = 0; zdi < scan_delta_size; zdi++)
     {
        const int zd = optimizer->m_pParams->m_pScan_deltas[zdi];
        const int mbb = optimizer->m_bb + zd;
        int ydi;
        if (mbb < 0) continue; else if (mbb > optimizer->m_limit) break;

        for (ydi = 0; ydi < scan_delta_size; ydi++)
          {
             const int yd = optimizer->m_pParams->m_pScan_deltas[ydi];
             const int mbg = optimizer->m_bg + yd;
             int xdi;

             if (mbg < 0) continue; else if (mbg > optimizer->m_limit) break;

             for (xdi = 0; xdi < scan_delta_size; xdi++)
               {
                  const int xd = optimizer->m_pParams->m_pScan_deltas[xdi];
                  const int mbr = optimizer->m_br + xd;
                  Etc1_Solution_Coordinates coords;
                  uint max_refinement_trials;
                  uint refinement_trial;

                  rg_etc1_solution_coordinates_component_set(&coords, mbr, mbg, mbb, 0, optimizer->m_pParams->m_use_color4);
                  if (mbr < 0) continue; else if (mbr > optimizer->m_limit) break;

                  if (optimizer->m_pParams->base_params->m_quality == rg_etc1_high_quality)
                    {
                       if (!rg_etc1_optimizer_evaluate_solution(optimizer, &coords, &optimizer->m_trial_solution, &optimizer->m_best_solution))
                         continue;
                    }
                  else
                    {
                       if (!rg_etc1_optimizer_evaluate_solution_fast(optimizer, &coords, &optimizer->m_trial_solution, &optimizer->m_best_solution))
                         continue;
                    }

                  // Now we have the input block, the avg. color of the input pixels, a set of trial selector indices, and the block color+intensity index.
                  // Now, for each component, attempt to refine the current solution by solving a simple linear equation. For example, for 4 colors:
                  // The goal is:
                  // pixel0 - (block_color+inten_table[selector0]) + pixel1 - (block_color+inten_table[selector1]) + pixel2 - (block_color+inten_table[selector2]) + pixel3 - (block_color+inten_table[selector3]) = 0
                  // Rearranging this:
                  // (pixel0 + pixel1 + pixel2 + pixel3) - (block_color+inten_table[selector0]) - (block_color+inten_table[selector1]) - (block_color+inten_table[selector2]) - (block_color+inten_table[selector3]) = 0
                  // (pixel0 + pixel1 + pixel2 + pixel3) - block_color - inten_table[selector0] - block_color-inten_table[selector1] - block_color-inten_table[selector2] - block_color-inten_table[selector3] = 0
                  // (pixel0 + pixel1 + pixel2 + pixel3) - 4*block_color - inten_table[selector0] - inten_table[selector1] - inten_table[selector2] - inten_table[selector3] = 0
                  // (pixel0 + pixel1 + pixel2 + pixel3) - 4*block_color - (inten_table[selector0] + inten_table[selector1] + inten_table[selector2] + inten_table[selector3]) = 0
                  // (pixel0 + pixel1 + pixel2 + pixel3)/4 - block_color - (inten_table[selector0] + inten_table[selector1] + inten_table[selector2] + inten_table[selector3])/4 = 0
                  // block_color = (pixel0 + pixel1 + pixel2 + pixel3)/4 - (inten_table[selector0] + inten_table[selector1] + inten_table[selector2] + inten_table[selector3])/4
                  // So what this means:
                  // optimal_block_color = avg_input - avg_inten_delta
                  // So the optimal block color can be computed by taking the average block color and subtracting the current average of the intensity delta.
                  // Unfortunately, optimal_block_color must then be quantized to 555 or 444 so it's not always possible to improve matters using this formula.
                  // Also, the above formula is for unclamped intensity deltas. The actual implementation takes into account clamping.

                  max_refinement_trials = (optimizer->m_pParams->base_params->m_quality == rg_etc1_low_quality) ? 2 : (((xd | yd | zd) == 0) ? 4 : 2);
                  for (refinement_trial = 0; refinement_trial < max_refinement_trials; refinement_trial++)
                    {
                       const uint8* pSelectors = optimizer->m_best_solution.m_selectors;
                       const int* pInten_table = rg_etc1_inten_tables[optimizer->m_best_solution.m_coords.m_inten_table];

                       int delta_sum_r = 0, delta_sum_g = 0, delta_sum_b = 0;
                       uint index;
                       color_quad_u8 base_color;
                       float avg_delta_r_f, avg_delta_g_f, avg_delta_b_f;
                       int br1, bg1, bb1;
                       bool skip;
                       Etc1_Solution_Coordinates coords1;
                       rg_etc1_solution_coordinates_get_scaled_color(&base_color, &optimizer->m_best_solution.m_coords);

                       for (index = 0; index < n; index++)
                         {
                            const uint s = *pSelectors++;
                            const int yyd = pInten_table[s];
                            // Compute actual delta being applied to each pixel, taking into account clamping.
                            delta_sum_r += CLAMP(base_color.comp.r + yyd, 0, 255) - base_color.comp.r;
                            delta_sum_g += CLAMP(base_color.comp.g + yyd, 0, 255) - base_color.comp.g;
                            delta_sum_b += CLAMP(base_color.comp.b + yyd, 0, 255) - base_color.comp.b;
                         }
                       if ((!delta_sum_r) && (!delta_sum_g) && (!delta_sum_b))
                         break;
                       avg_delta_r_f = (float)(delta_sum_r) / n;
                       avg_delta_g_f = (float)(delta_sum_g) / n;
                       avg_delta_b_f = (float)(delta_sum_b) / n;
                       br1 = (uint)CLAMP(((optimizer->m_avg_color[0] - avg_delta_r_f) * optimizer->m_limit / 255.0f + .5f),
                                         0, optimizer->m_limit);
                       bg1 = (uint)CLAMP(((optimizer->m_avg_color[1] - avg_delta_g_f) * optimizer->m_limit / 255.0f + .5f),
                                         0, optimizer->m_limit);
                       bb1 = (uint)CLAMP(((optimizer->m_avg_color[2] - avg_delta_b_f) * optimizer->m_limit / 255.0f + .5f),
                                         0, optimizer->m_limit);
                       skip = EINA_FALSE;

                       if ((mbr == br1) && (mbg == bg1) && (mbb == bb1)) {
                          skip = EINA_TRUE;
                       } else if ((br1 == optimizer->m_best_solution.m_coords.m_unscaled_color.comp.r) &&
                                  (bg1 == optimizer->m_best_solution.m_coords.m_unscaled_color.comp.g) &&
                                  (bb1 == optimizer->m_best_solution.m_coords.m_unscaled_color.comp.b)) {
                          skip = EINA_TRUE;
                       } else if ((optimizer->m_br == br1) && (optimizer->m_bg == bg1) && (optimizer->m_bb == bb1)) {
                          skip = EINA_TRUE;
                       }

                       if (skip)
                         break;

                       rg_etc1_solution_coordinates_component_set(&coords1, br1, bg1, bb1, 0, optimizer->m_pParams->m_use_color4);
                       if (optimizer->m_pParams->base_params->m_quality == rg_etc1_high_quality)
                         {
                            if (!rg_etc1_optimizer_evaluate_solution(optimizer, &coords1, &optimizer->m_trial_solution, &optimizer->m_best_solution))
                              break;
                         }
                       else
                         {
                            if (!rg_etc1_optimizer_evaluate_solution_fast(optimizer, &coords1, &optimizer->m_trial_solution, &optimizer->m_best_solution))
                              break;
                         }

                    }  // refinement_trial

               } // xdi
          } // ydi
     } // zdi

   if (!optimizer->m_best_solution.m_valid)
     {
        optimizer->m_pResult->m_error = cUINT32_MAX;
        return EINA_FALSE;
     }

#ifdef RG_ETC1_BUILD_DEBUG
   {
      color_quad_u8 block_colors[4];
      const color_quad_u8* pSrc_pixels;
      uint64 actual_error=0;
      uint i;
      const uint8* pSelectors = optimizer->m_best_solution.m_selectors;

      rg_etc1_solution_coordinates_block_colors_get(&optimizer->m_best_solution.m_coords, block_colors);
      pSrc_pixels = optimizer->m_pParams->m_pSrc_pixels;
      for (i = 0; i < n; i++)
        actual_error += rg_etc1_color_quad_u8_rgb_squared_distance(pSrc_pixels[i], block_colors[pSelectors[i]]);

      // ERROR CASE NO ASSERT IN EVAS CODE
      if (actual_error != optimizer->m_best_solution.m_error)
        return EINA_FALSE;
   }
#endif

   optimizer->m_pResult->m_error = optimizer->m_best_solution.m_error;
   rg_etc1_color_quad_u8_copy(&optimizer->m_pResult->m_block_color_unscaled,&optimizer->m_best_solution.m_coords.m_unscaled_color);
   optimizer->m_pResult->m_block_color4 = optimizer->m_best_solution.m_coords.m_color4;
   optimizer->m_pResult->m_block_inten_table = optimizer->m_best_solution.m_coords.m_inten_table;
   memcpy(optimizer->m_pResult->m_pSelectors, optimizer->m_best_solution.m_selectors, n);
   optimizer->m_pResult->m_n = n;

   return EINA_TRUE;
}

void
rg_etc1_optimizer_init(rg_etc1_optimizer *optimizer, const rg_etc1_optimizer_params *params,
                       rg_etc1_optimizer_results *result)
{
   // This version is hardcoded for 8 pixel subblocks.
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (params->m_num_src_pixels != 8) return ;

   const uint n = 8;
   uint i;
   float avg_color[3];
   float fc[3];

   optimizer->m_pParams = params;
   optimizer->m_pResult = result;

   rg_etc1_vec_init(avg_color, 0.0f);
   optimizer->m_limit = optimizer->m_pParams->m_use_color4 ? 15 : 31;

   for ( i = 0; i < n; i++)
     {
        const color_quad_u8 *c = &optimizer->m_pParams->m_pSrc_pixels[i];
        rg_etc1_vec_set(fc, c->comp.r, c->comp.g, c->comp.b);

        rg_etc1_vec_add(avg_color,fc);

        optimizer->m_luma[i] = (uint16)(c->comp.r + c->comp.g + c->comp.b);
        optimizer->m_sorted_luma[0][i] = i;
     }
   rg_etc1_vec_scale(avg_color, (1.0f/(float)(n)));
   rg_etc1_vec_copy(optimizer->m_avg_color,avg_color);

   optimizer->m_br = MIN((int)(optimizer->m_avg_color[0] * optimizer->m_limit / 255.0f + .5f), optimizer->m_limit);
   optimizer->m_bg = MIN((int)(optimizer->m_avg_color[1] * optimizer->m_limit / 255.0f + .5f), optimizer->m_limit);
   optimizer->m_bb = MIN((int)(optimizer->m_avg_color[2] * optimizer->m_limit / 255.0f + .5f), optimizer->m_limit);

   if (optimizer->m_pParams->base_params->m_quality <= rg_etc1_medium_quality)
     {
        optimizer->m_pSorted_luma_indices = rg_etc1_indirect_radix_sort(n, optimizer->m_sorted_luma[0],
                                                                        optimizer->m_sorted_luma[1], optimizer->m_luma,
                                                                        0, sizeof(optimizer->m_luma[0]), EINA_FALSE);
        optimizer->m_pSorted_luma = optimizer->m_sorted_luma[0];

        if (optimizer->m_pSorted_luma_indices == optimizer->m_sorted_luma[0])
          optimizer->m_pSorted_luma = optimizer->m_sorted_luma[1];

        for (i = 0; i < n; i++)
          optimizer->m_pSorted_luma[i] = optimizer->m_luma[optimizer->m_pSorted_luma_indices[i]];
     }

   rg_etc1_solution_coordinates_clear(&optimizer->m_best_solution.m_coords);
   optimizer->m_best_solution.m_valid = EINA_FALSE;
   optimizer->m_best_solution.m_error = cUINT64_MAX;
}

static bool
rg_etc1_optimizer_evaluate_solution(rg_etc1_optimizer *optimizer, const Etc1_Solution_Coordinates* coords,
                                    rg_etc1_potential_solution* trial_solution, rg_etc1_potential_solution* pBest_solution)
{
   color_quad_u8 base_color;
   const uint n = 8;
   uint inten_table;
   bool success = EINA_FALSE;

   trial_solution->m_valid = EINA_FALSE;

   if (optimizer->m_pParams->m_constrain_against_base_color5)
     {
        int dr, dg, db;
        dr = coords->m_unscaled_color.comp.r - optimizer->m_pParams->m_base_color5.comp.r;
        dg = coords->m_unscaled_color.comp.g - optimizer->m_pParams->m_base_color5.comp.g;
        db = coords->m_unscaled_color.comp.b - optimizer->m_pParams->m_base_color5.comp.b;

        if ((MIN(MIN(dr,dg),db) < cETC1ColorDeltaMin) || (MAX(MAX(dr,dg),db) > cETC1ColorDeltaMax))
          return EINA_FALSE;
     }

   rg_etc1_solution_coordinates_get_scaled_color(&base_color, coords);
   trial_solution->m_error = cUINT64_MAX;

   for (inten_table = 0; inten_table < cETC1IntenModifierValues; inten_table++)
     {
        const int* pInten_table = rg_etc1_inten_tables[inten_table];
        uint64 total_error = 0;
        color_quad_u8 block_colors[4];
        const color_quad_u8* pSrc_pixels = optimizer->m_pParams->m_pSrc_pixels;

        uint c;
        for (c = 0; c < 4; c++)
          {
             const int yd = pInten_table[c];
             rg_etc1_color_quad_u8_init(&block_colors[c], base_color.comp.r+yd, base_color.comp.g+yd, base_color.comp.b+yd, 0);
          }

        for (c = 0; c < n; c++)
          {
             uint best_selector_index = 0, best_error, trial_error;
             const color_quad_u8* src_pixel = pSrc_pixels++;

             best_error = SQUARE((src_pixel->comp.r - block_colors[0].comp.r)) +
               SQUARE((src_pixel->comp.g - block_colors[0].comp.g)) +
               SQUARE((src_pixel->comp.b - block_colors[0].comp.b));
             trial_error = SQUARE((src_pixel->comp.r - block_colors[1].comp.r)) +
               SQUARE((src_pixel->comp.g - block_colors[1].comp.g)) +
               SQUARE((src_pixel->comp.b - block_colors[1].comp.b));
             if (trial_error < best_error)
               {
                  best_error = trial_error;
                  best_selector_index = 1;
               }

             trial_error = SQUARE((src_pixel->comp.r - block_colors[2].comp.r)) +
               SQUARE((src_pixel->comp.g - block_colors[2].comp.g)) +
               SQUARE((src_pixel->comp.b - block_colors[2].comp.b));
             if (trial_error < best_error)
               {
                  best_error = trial_error;
                  best_selector_index = 2;
               }

             trial_error = SQUARE((src_pixel->comp.r - block_colors[3].comp.r)) +
               SQUARE((src_pixel->comp.g - block_colors[3].comp.g)) +
               SQUARE((src_pixel->comp.b - block_colors[3].comp.b));
             if (trial_error < best_error)
               {
                  best_error = trial_error;
                  best_selector_index = 3;
               }

             optimizer->m_temp_selectors[c] = (uint8)(best_selector_index);

             total_error += best_error;
             if (total_error >= trial_solution->m_error)
               break;
          }

        if (total_error < trial_solution->m_error)
          {
             trial_solution->m_error = total_error;
             trial_solution->m_coords.m_inten_table = inten_table;
             memcpy(trial_solution->m_selectors, optimizer->m_temp_selectors, 8);
             trial_solution->m_valid = EINA_TRUE;
          }
     }
   rg_etc1_color_quad_u8_copy(&trial_solution->m_coords.m_unscaled_color,&coords->m_unscaled_color);
   trial_solution->m_coords.m_color4 = optimizer->m_pParams->m_use_color4;

   if (pBest_solution)
     {
        if (trial_solution->m_error < pBest_solution->m_error)
          {
             memcpy(pBest_solution,trial_solution,sizeof(rg_etc1_potential_solution));
             success = EINA_TRUE;
          }
     }

   return success;
}

static bool
rg_etc1_optimizer_evaluate_solution_fast(rg_etc1_optimizer *optimizer, const Etc1_Solution_Coordinates *coords,
                                         rg_etc1_potential_solution *trial_solution, rg_etc1_potential_solution *pBest_solution)
{
   color_quad_u8 base_color;
   const uint n = 8;
   int inten_table;
   bool success = EINA_FALSE;

   if (optimizer->m_pParams->m_constrain_against_base_color5)
     {
        int dr, dg, db;
        dr = coords->m_unscaled_color.comp.r - optimizer->m_pParams->m_base_color5.comp.r;
        dg = coords->m_unscaled_color.comp.g - optimizer->m_pParams->m_base_color5.comp.g;
        db = coords->m_unscaled_color.comp.b - optimizer->m_pParams->m_base_color5.comp.b;


        if ((MIN(MIN(dr,dg),db) < cETC1ColorDeltaMin) || (MAX(MAX(dr,dg),db) > cETC1ColorDeltaMax))
          {
             trial_solution->m_valid = EINA_FALSE;
             return EINA_FALSE;
          }
     }

   rg_etc1_solution_coordinates_get_scaled_color(&base_color,coords);

   trial_solution->m_error = cUINT64_MAX;

   for (inten_table = cETC1IntenModifierValues - 1; inten_table >= 0; --inten_table)
     {
        const int* pInten_table = rg_etc1_inten_tables[inten_table];
        uint block_inten[4];
        color_quad_u8 block_colors[4];
        uint block_inten_midpoints[3];
        uint64 total_error = 0;
        const color_quad_u8* pSrc_pixels = optimizer->m_pParams->m_pSrc_pixels;
        uint s, c;

        for (s = 0; s < 4; s++)
          {
             const int yd = pInten_table[s];
             rg_etc1_color_quad_u8_init(&block_colors[s], base_color.comp.r+yd, base_color.comp.g+yd, base_color.comp.b+yd, 0);
             block_inten[s] = block_colors[s].comp.r + block_colors[s].comp.g + block_colors[s].comp.b;
          }

        // evaluate_solution_fast() enforces/assumesd a total ordering of the input colors along the intensity (1,1,1) axis to more quickly classify the inputs to selectors.
        // The inputs colors have been presorted along the projection onto this axis, and ETC1 block colors are always ordered along the intensity axis, so this classification is fast.
        // 0   1   2   3
        //   01  12  23
        block_inten_midpoints[0] = block_inten[0] + block_inten[1];
        block_inten_midpoints[1] = block_inten[1] + block_inten[2];
        block_inten_midpoints[2] = block_inten[2] + block_inten[3];

        if ((optimizer->m_pSorted_luma[n - 1] * 2) < block_inten_midpoints[0])
          {
             if (block_inten[0] > optimizer->m_pSorted_luma[n - 1])
               {
                  const uint min_error = labs(block_inten[0] - optimizer->m_pSorted_luma[n - 1]);
                  if (min_error >= trial_solution->m_error)
                    continue;
               }

             memset(&optimizer->m_temp_selectors[0], 0, n);

             for (c = 0; c < n; c++) {
                total_error += rg_etc1_color_quad_u8_rgb_squared_distance(block_colors[0], pSrc_pixels[c]);
             }
          }
        else if ((optimizer->m_pSorted_luma[0] * 2) >= block_inten_midpoints[2])
          {
             if (optimizer->m_pSorted_luma[0] > block_inten[3])
               {
                  const uint min_error = labs(optimizer->m_pSorted_luma[0] - block_inten[3]);
                  if (min_error >= trial_solution->m_error)
                    continue;
               }

             memset(&optimizer->m_temp_selectors[0], 3, n);

             for (c = 0; c < n; c++)
               total_error += rg_etc1_color_quad_u8_rgb_squared_distance(block_colors[3], pSrc_pixels[c]);
          }
        else
          {
             uint cur_selector = 0;
             for (c = 0; c < n; c++)
               {
                  const uint y = optimizer->m_pSorted_luma[c];
                  while ((y * 2) >= block_inten_midpoints[cur_selector])
                    if (++cur_selector > 2)
                      goto done;
                  const uint sorted_pixel_index = optimizer->m_pSorted_luma_indices[c];
                  optimizer->m_temp_selectors[sorted_pixel_index] = (uint8)(cur_selector);
                  total_error += rg_etc1_color_quad_u8_rgb_squared_distance(block_colors[cur_selector],
                                                                            pSrc_pixels[sorted_pixel_index]);
               }
          done:
             while (c < n)
               {
                  const uint sorted_pixel_index = optimizer->m_pSorted_luma_indices[c];
                  optimizer->m_temp_selectors[sorted_pixel_index] = 3;
                  total_error += rg_etc1_color_quad_u8_rgb_squared_distance(block_colors[3], pSrc_pixels[sorted_pixel_index]);
                  ++c;
               }
          }

        if (total_error < trial_solution->m_error)
          {
             trial_solution->m_error = total_error;
             trial_solution->m_coords.m_inten_table = inten_table;
             memcpy(trial_solution->m_selectors, optimizer->m_temp_selectors, n);
             trial_solution->m_valid = EINA_TRUE;
             if (!total_error)
               break;
          }
     }
   rg_etc1_color_quad_u8_copy(&trial_solution->m_coords.m_unscaled_color,&coords->m_unscaled_color);
   trial_solution->m_coords.m_color4 = optimizer->m_pParams->m_use_color4;

   if (pBest_solution)
     {
        if (trial_solution->m_error < pBest_solution->m_error)
          {
             memcpy(pBest_solution,trial_solution,sizeof(rg_etc1_potential_solution));
             success = EINA_TRUE;
          }
     }

   return success;
}

static uint
etc1_decode_value(uint diff, uint inten, uint selector, uint packed_c)
{
   const uint limit = diff ? 32 : 16;
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!((diff < 2) && (inten < 8) && (selector < 4) && (packed_c < limit)))
     return 0;
   int c;
   if (diff)
     c = (packed_c >> 2) | (packed_c << 3);
   else
     c = packed_c | (packed_c << 4);
   c += rg_etc1_inten_tables[inten][selector];
   c = CLAMP(c, 0, 255);
   return c;
}

static inline int mul_8bit(int a, int b) { int t = a*b + 128; return (t + (t >> 8)) >> 8; }

void rg_etc1_pack_block_init()
{
   uint diff;
   uint expand5[32];
   int i;
   for (diff = 0; diff < 2; diff++)
     {
        const uint limit = diff ? 32 : 16;
        uint inten;

        for (inten = 0; inten < 8; inten++)
          {
             uint selector;
             for (selector = 0; selector < 4; selector++)
               {
                  const uint inverse_table_index = diff + (inten << 1) + (selector << 4);
                  uint color;
                  for (color = 0; color < 256; color++)
                    {
                       uint best_error = cUINT32_MAX, best_packed_c = 0;
                       uint packed_c;
                       for (packed_c = 0; packed_c < limit; packed_c++)
                         {
                            int v = etc1_decode_value(diff, inten, selector, packed_c);
                            uint err = labs(v - (int)color);
                            if (err < best_error)
                              {
                                 best_error = err;
                                 best_packed_c = packed_c;
                                 if (!best_error)
                                   break;
                              }
                         }
                       if (!(best_error <= 255))
                         {
                            fprintf(stderr, "ETC1: Failed to write the inverse lookup table!\n");
                            return;
                         }
                       rg_etc1_inverse_lookup[inverse_table_index][color] = (uint16)(best_packed_c | (best_error << 8));
                    }
               }
          }
     }

   for(i = 0; i < 32; i++)
     expand5[i] = (i << 3) | (i >> 2);

   for(i = 0; i < 256 + 16; i++)
     {
        int v = (int)CLAMP(i - 8, 0, 255);
        rg_etc_quant5_tab[i] = (uint8)(expand5[mul_8bit(v,31)]);
     }
}

// Packs solid color blocks efficiently using a set of small precomputed tables.
// For random 888 inputs, MSE results are better than Erricson's ETC1 packer in "slow" mode ~9.5% of the time, is slightly worse only ~.01% of the time, and is equal the rest of the time.
static uint64
rg_etc1_pack_block_solid_color(unsigned char *block, const color_quad_u8 *color, rg_etc1_pack_params *pack_params EINA_UNUSED)
{
   const uint8 *pColor = (uint8 *) &color->m_u32;

   if (!rg_etc1_inverse_lookup[0][255])
     rg_etc1_pack_block_init();

   if (!rg_etc1_inverse_lookup[0][255])
     {
        fprintf(stderr, "ETC1: Inverse lookup table not set (in %s)!\n", __FUNCTION__);
        return 0;
     }

   const uint s_next_comp[4] = { 1, 2, 0, 1 };
   uint best_error = cUINT32_MAX, best_i = 0;
   int best_x = 0, best_packed_c1 = 0, best_packed_c2 = 0;
   uint i;

   // For each possible 8-bit value, there is a precomputed list of diff/inten/selector configurations
   // that allow that 8-bit value to be encoded with no error.
   for (i = 0; i < 3; i++)
     {
        const int c0 = pColor[i];
        const int c1 = pColor[s_next_comp[i]];
        const int c2 = pColor[s_next_comp[i + 1]];

        const int delta_range = 1;
        int delta;
        for (delta = -delta_range; delta <= delta_range; delta++)
          {
             const int c_plus_delta = CLAMP(c0 + delta, 0, 255);

             uint16* pTable;
             if (!c_plus_delta)
               pTable = (uint16 *)rg_etc_color8_to_etc_block_config_0_255[0];
             else if (c_plus_delta == 255)
               pTable = (uint16 *)rg_etc_color8_to_etc_block_config_0_255[1];
             else
               pTable = (uint16 *)rg_etc_color8_to_etc_block_config_1_to_254[c_plus_delta - 1];

             do
               {
                  const uint x = *pTable++;
                  uint16* pInverse_table;
                  uint16 p1, p2;
                  uint trial_error;

#ifdef RG_ETC1_BUILD_DEBUG
                  const uint diff = x & 1;
                  const uint inten = (x >> 1) & 7;
                  const uint selector = (x >> 4) & 3;
                  const uint p0 = (x >> 8) & 255;
                  // ERROR CASE NO ASSERT IN EVAS CODE
                  if (etc1_decode_value(diff, inten, selector, p0) != (uint)c_plus_delta) return 0;
#endif

                  pInverse_table = rg_etc1_inverse_lookup[x & 0xFF];
                  p1 = pInverse_table[c1];
                  p2 = pInverse_table[c2];
                  trial_error = SQUARE((c_plus_delta - c0)) + SQUARE((p1 >> 8)) + SQUARE((p2 >> 8));
                  if (trial_error < best_error)
                    {
                       best_error = trial_error;
                       best_x = x;
                       best_packed_c1 = p1 & 0xFF;
                       best_packed_c2 = p2 & 0xFF;
                       best_i = i;
                       if (!best_error)
                         goto found_perfect_match;
                    }
               } while (*pTable != 0xFFFF);
          }
     }
 found_perfect_match:
   {
      const uint diff = best_x & 1;
      const uint inten = (best_x >> 1) & 7;
      uint etc1_selector;
      uint best_packed_c0;
      uint selector_val;

      block[0] = block[1] = block[2] = 0;
      block[3] = (uint8)(((inten | (inten << 3)) << 2) | (diff << 1));

      etc1_selector = rg_etc_selector_index_to_etc1[(best_x >> 4) & 3];
      selector_val = (etc1_selector & 2) ? 0xFFFF : 0;
      memcpy(&block[4], &selector_val, 2);
      selector_val = (etc1_selector & 1) ? 0xFFFF : 0;
      memcpy(&block[6], &selector_val, 2);

      best_packed_c0 = (best_x >> 8) & 255;
      if (diff)
        {
           block[best_i] = (uint8)(best_packed_c0 << 3);
           block[s_next_comp[best_i]] = (uint8)(best_packed_c1 << 3);
           block[s_next_comp[best_i+1]] = (uint8)(best_packed_c2 << 3);
        }
      else
        {
           block[best_i] = (uint8)(best_packed_c0 | (best_packed_c0 << 4));
           block[s_next_comp[best_i]] = (uint8)(best_packed_c1 | (best_packed_c1 << 4));
           block[s_next_comp[best_i+1]] = (uint8)(best_packed_c2 | (best_packed_c2 << 4));
        }
   }
   return best_error;
}

#if RG_ETC1_CONSTRAINED_SUBBLOCK
static uint
rg_etc1_pack_block_solid_color_constrained(rg_etc1_optimizer_results *results,uint num_colors,
                                           const uint8* pColor, rg_etc1_pack_params *pack_params EINA_UNUSED,
                                           bool use_diff, const color_quad_u8* pBase_color5_unscaled)
{
   static uint s_next_comp[4] = { 1, 2, 0, 1 };
   uint best_error = cUINT32_MAX, best_i = 0;
   int best_x = 0, best_packed_c1 = 0, best_packed_c2 = 0;
   uint i;

   if (!rg_etc1_inverse_lookup[0][255])
     rg_etc1_pack_block_init();

   if (!rg_etc1_inverse_lookup[0][255])
     {
        fprintf(stderr, "ETC1: Inverse lookup table not set (in %s)!\n", __FUNCTION__);
        return 0;
     }

   // For each possible 8-bit value, there is a precomputed list of diff/inten/selector configurations
   // that allow that 8-bit value to be encoded with no error.
   for (i = 0; i < 3; i++)
     {
        const uint c1 = pColor[s_next_comp[i]], c2 = pColor[s_next_comp[i + 1]];
        const int delta_range = 1;
        int delta;
        for (delta = -delta_range; delta <= delta_range; delta++)
          {
             const int c_plus_delta = CLAMP(pColor[i] + delta, 0, 255);
             const uint16* pTable;
             if (!c_plus_delta)
               pTable = rg_etc_color8_to_etc_block_config_0_255[0];
             else if (c_plus_delta == 255)
               pTable = rg_etc_color8_to_etc_block_config_0_255[1];
             else
               pTable = rg_etc_color8_to_etc_block_config_1_to_254[c_plus_delta - 1];

             do
               {
                  const uint x = *pTable++;
                  const uint diff = x & 1;
                  if (((uint)use_diff) != diff)
                    {
                       if (*pTable == 0xFFFF)
                         break;
                       continue;
                    }

                  if ((diff) && (pBase_color5_unscaled))
                    {
                       const int p0 = (x >> 8) & 255;
                       unsigned char cc1 = rg_etc1_color_quad_component_get(pBase_color5_unscaled->m_u32,
                                                                           i);
                       int delta1 = p0 - (int)(cc1);
                       if ((delta1 < cETC1ColorDeltaMin) || (delta1 > cETC1ColorDeltaMax))
                         {
                            if (*pTable == 0xFFFF)
                              break;
                            continue;
                         }
                    }

#ifdef RG_ETC1_BUILD_DEBUG
                  {
                     const uint inten = (x >> 1) & 7;
                     const uint selector = (x >> 4) & 3;
                     const uint p0 = (x >> 8) & 255;
                     // ERROR CASE NO ASSERT IN EVAS CODE
                     if (etc1_decode_value(diff, inten, selector, p0) != (uint)c_plus_delta) return 0;
                  }
#endif

                  const uint16* pInverse_table = rg_etc1_inverse_lookup[x & 0xFF];
                  uint16 p1 = pInverse_table[c1];
                  uint16 p2 = pInverse_table[c2];
                  uint trial_error;

                  if ((diff) && (pBase_color5_unscaled))
                    {
                       unsigned char cc1 = rg_etc1_color_quad_component_get(pBase_color5_unscaled->m_u32,
                                                                           s_next_comp[i]);
                       int delta1 = (p1 & 0xFF) - (int)(cc1);
                       unsigned char cc2 = rg_etc1_color_quad_component_get(pBase_color5_unscaled->m_u32,
                                                                           s_next_comp[i + 1]);
                       int delta2 = (p2 & 0xFF) - (int)(cc2);
                       if ((delta1 < cETC1ColorDeltaMin) || (delta1 > cETC1ColorDeltaMax) 
                           || (delta2 < cETC1ColorDeltaMin) || (delta2 > cETC1ColorDeltaMax))
                         {
                            if (*pTable == 0xFFFF)
                              break;
                            continue;
                         }
                    }

                  trial_error = SQUARE((c_plus_delta - pColor[i])) + SQUARE((p1 >> 8)) + SQUARE((p2 >> 8));
                  if (trial_error < best_error)
                    {
                       best_error = trial_error;
                       best_x = x;
                       best_packed_c1 = p1 & 0xFF;
                       best_packed_c2 = p2 & 0xFF;
                       best_i = i;
                       if (!best_error)
                         goto found_perfect_match;
                    }
               } while (*pTable != 0xFFFF);
          }
     }
 found_perfect_match:

   if (best_error == cUINT32_MAX)
     return best_error;

   best_error *= num_colors;

   results->m_n = num_colors;
   results->m_block_color4 = !(best_x & 1);
   results->m_block_inten_table = (best_x >> 1) & 7;
   memset(results->m_pSelectors, (best_x >> 4) & 3, num_colors);
   {
      const uint best_packed_c0 = (best_x >> 8) & 255;
      rg_etc1_color_quad_u8_component_set(&results->m_block_color_unscaled, best_i, (uint8)best_packed_c0);
      rg_etc1_color_quad_u8_component_set(&results->m_block_color_unscaled, s_next_comp[best_i], (uint8)best_packed_c1);
      rg_etc1_color_quad_u8_component_set(&results->m_block_color_unscaled, s_next_comp[best_i+1], (uint8)best_packed_c2);
      results->m_error = best_error;
   }
   return best_error;
}
#endif

#if RG_ETC1_DITHERING
// Function originally from RYG's public domain real-time DXT1 compressor, modified for 555.
static void
rg_etc1_dither_block_555(color_quad_u8* dest, color_quad_u8* block)
{
   int err[8],*ep1 = err,*ep2 = err+4;
   uint8 *quant = rg_etc_quant5_tab+8;
   int ch;

   memset(dest, 0xFF, sizeof(color_quad_u8)*16);

   // process channels seperately
   for(ch=0;ch<3;ch++)
     {
        uint8* bp = (uint8*)block;
        uint8* dp = (uint8*)dest;
        int y;

        bp += ch; dp += ch;

        memset(err,0, sizeof(err));
        for(y = 0; y < 4; y++)
          {
             int *tmp;
             // pixel 0
             dp[ 0] = quant[bp[ 0] + ((3*ep2[1] + 5*ep2[0]) >> 4)];
             ep1[0] = bp[ 0] - dp[ 0];

             // pixel 1
             dp[ 4] = quant[bp[ 4] + ((7*ep1[0] + 3*ep2[2] + 5*ep2[1] + ep2[0]) >> 4)];
             ep1[1] = bp[ 4] - dp[ 4];

             // pixel 2
             dp[ 8] = quant[bp[ 8] + ((7*ep1[1] + 3*ep2[3] + 5*ep2[2] + ep2[1]) >> 4)];
             ep1[2] = bp[ 8] - dp[ 8];

             // pixel 3
             dp[12] = quant[bp[12] + ((7*ep1[2] + 5*ep2[3] + ep2[2]) >> 4)];
             ep1[3] = bp[12] - dp[12];

             // advance to next line
             tmp = ep1; ep1 = ep2; ep2 = tmp;
             bp += 16;
             dp += 16;
          }
     }
}
#endif

static inline unsigned int
_bgra_to_rgba(unsigned int val)
{
   //(((a) << 24) + ((r) << 16) + ((g) << 8) + (b))
   return ARGB_JOIN(A_VAL_GET(&val), R_VAL_GET(&val), G_VAL_GET(&val), B_VAL_GET(&val));
}

static void
_bgra_to_rgba_block(color_quad_u8 *output, const unsigned int *input, int len)
{
   for (int k = len; k; --k)
     {
        output->m_u32 = _bgra_to_rgba(*input++);
        output++;
     }
}

unsigned int
rg_etc1_pack_block(void* pETC1_block, const unsigned int* pSrc_pixels_BGRA, rg_etc1_pack_params *pack_params)
{
   color_quad_u8 pSrc_pixels[16];
   unsigned char *dst_block = (unsigned char *)pETC1_block;
   unsigned int first_pixel_u32;
   int r;
   color_quad_u8 subblock_pixels[8];
   uint64 best_error = cUINT64_MAX;
   uint best_use_color4=EINA_FALSE;
   uint best_flip=EINA_FALSE;
   uint8 best_selectors[2][8];
   rg_etc1_optimizer optimizer = { 0 };
   rg_etc1_optimizer_results best_results[2] = { { 0 } };
   rg_etc1_optimizer_results results[3] = { { 0 } };
   rg_etc1_optimizer_params params;
   uint i, flip;
   uint8 selectors[3][8];
   int dr, dg, db;
   uint selector0 = 0, selector1 = 0;
   static const int s_scan_delta_0_to_4[] = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
   static const int s_scan_delta_0_to_1[] = { -1, 0, 1 };
   static const int s_scan_delta_0[] = { 0 };

#ifdef RG_ETC1_BUILD_DEBUG
   // Ensure all alpha values are 0xFF.
   for (i = 0; i < 16; i++)
     {
        // ERROR CASE NO ASSERT IN EVAS CODE
        if (pSrc_pixels[i].comp.a != 255) return 0;
     }
#endif
   rg_etc1_optimizer_clear(&optimizer);

   // Convert evas BGRA to rg_etc1 RGBA
   _bgra_to_rgba_block(pSrc_pixels, pSrc_pixels_BGRA, 16);
   first_pixel_u32 = pSrc_pixels[0].m_u32;

   // Check for solid block.
   for (r = 15; r >= 1; --r)
     if (pSrc_pixels[r].m_u32 != first_pixel_u32)
       break;
   if (!r)
     return (unsigned int)(16 * rg_etc1_pack_block_solid_color(dst_block, &pSrc_pixels[0], pack_params));

#if RG_ETC1_DITHERING
   // Dithering gives mitigated results... It would be nice to know when to use it.
   color_quad_u8 dithered_pixels[16];
   if (pack_params->m_dithering)
     {
        rg_etc1_dither_block_555(dithered_pixels, pSrc_pixels);
        pSrc_pixels = dithered_pixels;
     }
#endif

   for (i = 0; i < 2; i++)
     {
        best_results[i].m_n = 8;
        best_results[i].m_pSelectors = best_selectors[i];
     }

   for (i = 0; i < 3; i++)
     {
        results[i].m_n = 8;
        results[i].m_pSelectors = selectors[i];
     }

   rg_etc1_optimizer_params_clean(&params);
   params.base_params = pack_params;
   params.m_num_src_pixels = 8;
   params.m_pSrc_pixels = subblock_pixels;

   // try horizontal VS. vertical split
   for (flip = 0; flip < 2; flip++)
     {
        // try two color types: RGB555 + diff333 or RGB444 & RGB444
        uint use_color4;
        for (use_color4 = 0; use_color4 < 2; use_color4++)
          {
             uint64 trial_error = 0;

             // subblock is either top/bottom or left/right
             uint subblock;
             for (subblock = 0; subblock < 2; subblock++)
               {
                  results[2].m_error = cUINT64_MAX;

                  if (flip)
                    // subblock is top or bottom, copy source
                    memcpy(subblock_pixels, pSrc_pixels + subblock * 8, sizeof(color_quad_u8) * 8);
                  else
                    {
                       // subblock = 1 : left, subblock = 2 : right, copy source
                       const color_quad_u8* pSrc_col = pSrc_pixels + subblock * 2;
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[0], &pSrc_col[0]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[1], &pSrc_col[4]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[2], &pSrc_col[8]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[3], &pSrc_col[12]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[4], &pSrc_col[1]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[5], &pSrc_col[5]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[6], &pSrc_col[9]);
                       rg_etc1_color_quad_u8_copy(&subblock_pixels[7], &pSrc_col[13]);
                    }

#if RG_ETC1_CONSTRAINED_SUBBLOCK
                  if ((params.base_params->m_quality >= rg_etc1_medium_quality) && ((subblock) || (use_color4)))
                    {
                       const uint32 subblock_pixel0_u32 = subblock_pixels[0].m_u32;
                       for (r = 7; r >= 1; --r)
                         if (subblock_pixels[r].m_u32 != subblock_pixel0_u32)
                           break;
                       if (!r)
                         {
                            // all pixels in subblock have the same color
                            rg_etc1_pack_block_solid_color_constrained(&results[2], 8, &subblock_pixels[0].comp.r,
                                                                       pack_params, !use_color4,
                                                                       (subblock && !use_color4) ? &results[0].m_block_color_unscaled : NULL);
                         }
                    }
#endif

                  params.m_use_color4 = (use_color4 != 0);
                  params.m_constrain_against_base_color5 = EINA_FALSE;

                  if ((!use_color4) && (subblock))
                    {
                       params.m_constrain_against_base_color5 = EINA_TRUE;
                       rg_etc1_color_quad_u8_copy(&params.m_base_color5,&results[0].m_block_color_unscaled);
                    }

                  if (params.base_params->m_quality == rg_etc1_high_quality)
                    {
                       params.m_scan_delta_size = RG_ETC1_ARRAY_SIZE(s_scan_delta_0_to_4);
                       params.m_pScan_deltas = s_scan_delta_0_to_4;
                    }
                  else if (params.base_params->m_quality == rg_etc1_medium_quality)
                    {
                       params.m_scan_delta_size = RG_ETC1_ARRAY_SIZE(s_scan_delta_0_to_1);
                       params.m_pScan_deltas = s_scan_delta_0_to_1;
                    }
                  else
                    {
                       params.m_scan_delta_size = RG_ETC1_ARRAY_SIZE(s_scan_delta_0);
                       params.m_pScan_deltas = s_scan_delta_0;
                    }

                  rg_etc1_optimizer_init(&optimizer, &params, &results[subblock]);
                  if (!rg_etc1_optimizer_compute(&optimizer))
                    break;

                  if (params.base_params->m_quality >= rg_etc1_medium_quality)
                    {
                       // TODO: Fix fairly arbitrary/unrefined thresholds that control how far away to scan for potentially better solutions.
                       const uint refinement_error_thresh0 = 3000;
                       const uint refinement_error_thresh1 = 6000;
                       if (results[subblock].m_error > refinement_error_thresh0)
                         {
                            if (params.base_params->m_quality == rg_etc1_medium_quality)
                              {
                                 static const int s_scan_delta_2_to_3[] = { -3, -2, 2, 3 };
                                 params.m_scan_delta_size = RG_ETC1_ARRAY_SIZE(s_scan_delta_2_to_3);
                                 params.m_pScan_deltas = s_scan_delta_2_to_3;
                              }
                            else
                              {
                                 static const int s_scan_delta_5_to_5[] = { -5, 5 };
                                 static const int s_scan_delta_5_to_8[] = { -8, -7, -6, -5, 5, 6, 7, 8 };
                                 if (results[subblock].m_error > refinement_error_thresh1)
                                   {
                                      params.m_scan_delta_size = RG_ETC1_ARRAY_SIZE(s_scan_delta_5_to_8);
                                      params.m_pScan_deltas = s_scan_delta_5_to_8;
                                   }
                                 else
                                   {
                                      params.m_scan_delta_size = RG_ETC1_ARRAY_SIZE(s_scan_delta_5_to_5);
                                      params.m_pScan_deltas = s_scan_delta_5_to_5;
                                   }
                              }

                            if (!rg_etc1_optimizer_compute(&optimizer))
                              break;
                         }

                       if (results[2].m_error < results[subblock].m_error)
                         results[subblock] = results[2];
                    }

                  trial_error += results[subblock].m_error;
                  if (trial_error >= best_error)
                    break;
               }

             if (subblock < 2)
               continue;

             best_error = trial_error;
             rg_etc1_optimizer_results_duplicate(&best_results[0], &results[0]);
             rg_etc1_optimizer_results_duplicate(&best_results[1], &results[1]);
             best_flip = flip;
             best_use_color4 = use_color4;
          } // use_color4
     } // flip

   dr = best_results[1].m_block_color_unscaled.comp.r - best_results[0].m_block_color_unscaled.comp.r;
   dg = best_results[1].m_block_color_unscaled.comp.g - best_results[0].m_block_color_unscaled.comp.g;
   db = best_results[1].m_block_color_unscaled.comp.b - best_results[0].m_block_color_unscaled.comp.b;
   // ERROR CASE NO ASSERT IN EVAS CODE
   if (!(best_use_color4 || ((MIN(MIN(dr, dg), db) >= cETC1ColorDeltaMin) && (MAX(MAX(dr, dg), db) <= cETC1ColorDeltaMax)))) return 0;

   if (best_use_color4)
     {
        dst_block[0] = (uint8)(best_results[1].m_block_color_unscaled.comp.r |
                               (best_results[0].m_block_color_unscaled.comp.r << 4));
        dst_block[1] = (uint8)(best_results[1].m_block_color_unscaled.comp.g |
                               (best_results[0].m_block_color_unscaled.comp.g << 4));
        dst_block[2] = (uint8)(best_results[1].m_block_color_unscaled.comp.b |
                               (best_results[0].m_block_color_unscaled.comp.b << 4));
     }
   else
     {
        if (dr < 0) dr += 8; dst_block[0] = (uint8)((best_results[0].m_block_color_unscaled.comp.r << 3) | dr);
        if (dg < 0) dg += 8; dst_block[1] = (uint8)((best_results[0].m_block_color_unscaled.comp.g << 3) | dg);
        if (db < 0) db += 8; dst_block[2] = (uint8)((best_results[0].m_block_color_unscaled.comp.b << 3) | db);
     }

   dst_block[3] = (uint8)((best_results[1].m_block_inten_table << 2) |
                          (best_results[0].m_block_inten_table << 5) |
                          ((~best_use_color4 & 1) << 1) | best_flip );

   if (best_flip)
     {
        // flipped:
        // { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 },
        // { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }
        //
        // { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 },
        // { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }
        const uint8* pSelectors0 = best_results[0].m_pSelectors;
        const uint8* pSelectors1 = best_results[1].m_pSelectors;
        int x;
        for (x = 3; x >= 0; --x)
          {
             uint b;
             b = rg_etc_selector_index_to_etc1[pSelectors1[4 + x]];
             selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);

             b = rg_etc_selector_index_to_etc1[pSelectors1[x]];
             selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);

             b = rg_etc_selector_index_to_etc1[pSelectors0[4 + x]];
             selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);

             b = rg_etc_selector_index_to_etc1[pSelectors0[x]];
             selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);
          }
     }
   else
     {
        // non-flipped:
        // { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
        // { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }
        //
        // { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 },
        // { 3, 0 }, { 3, 1 }, { 3, 2 }, { 3, 3 }
        int subblock;
        for (subblock = 1; subblock >= 0; --subblock)
          {
             const uint8* pSelectors = best_results[subblock].m_pSelectors + 4;
             uint j;
             for (j = 0; j < 2; j++)
               {
                  uint b;
                  b = rg_etc_selector_index_to_etc1[pSelectors[3]];
                  selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);

                  b = rg_etc_selector_index_to_etc1[pSelectors[2]];
                  selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);

                  b = rg_etc_selector_index_to_etc1[pSelectors[1]];
                  selector0 = (selector0 << 1) | (b & 1); selector1 = (selector1 << 1) | (b >> 1);

                  b = rg_etc_selector_index_to_etc1[pSelectors[0]];
                  selector0 = (selector0 << 1) | (b & 1);selector1 = (selector1 << 1) | (b >> 1);

                  pSelectors -= 4;
               }
          }
     }

   dst_block[4] = (uint8)(selector1 >> 8); dst_block[5] = (uint8)(selector1 & 0xFF);
   dst_block[6] = (uint8)(selector0 >> 8); dst_block[7] = (uint8)(selector0 & 0xFF);
   return (unsigned int)(best_error);
}
