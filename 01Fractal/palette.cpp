#include "palette.h"

/*
#FFFFCC
#FFF5B5
#FFEC9D
#FEE187
#FED470
#FEBF5A
#FEAB49
#FD9740
#FD7C37
#FC5B2E
#F43D25
#E6211E
#D41020
#C00225
#A10026
#800026

0xFFCCFFFF
0xFFB5F5FF
0xFF9DECFF
0xFF87E1FE
0xFF70D4FE
0xFF5ABFFE
0xFF49ABFE
0xFF4097FD
0xFF377CFD
0xFF2E5BFC
0xFF253DF4
0xFF1E21E6
0xFF2010D4
0xFF2502C0
0xFF2600A1
0xFF260080
*/

// RAMP GENERATOD
// https://www.rampgenerator.com

uint32_t bswap32(uint32_t a){
    return
    ((a & 0x000000FF) << 24) |
    ((a & 0x0000FF00) << 8) |
    ((a & 0x00FF0000) >> 8) |
    ((a & 0xFF000000) >> 24);
}

std::vector<uint32_t> color_ramp = {
    bswap32(0x000004FF),
    bswap32(0x110628FF),
    bswap32(0x230C4AFF),
    bswap32(0x3F0E5EFF),
    bswap32(0x5C126DFF),
    bswap32(0x771C6BFF),
    bswap32(0x912667FF),
    bswap32(0xAA315EFF),
    bswap32(0xC3404FFF),
    bswap32(0xDA523BFF),
    bswap32(0xEA6A25FF),
    bswap32(0xF6850FFF),
    bswap32(0xFAA71DFF),
    bswap32(0xFBCB37FF),
    bswap32(0xFBE56BFF),
    bswap32(0xFCFFA4FF)
};
