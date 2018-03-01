#ifndef __HI_VPSS_REG_H__
#define __HI_VPSS_REG_H__

/* Define the union U_VPSS_NR_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_en               : 1   ; /* [0]  */
        unsigned int    nry1_en               : 1   ; /* [1]  */
        unsigned int    nry2_en               : 1   ; /* [2]  */
        unsigned int    nry3_en               : 1   ; /* [3]  */
        unsigned int    uvflag                : 1   ; /* [4]  */
        unsigned int    rfrnryid              : 3   ; /* [7..5]  */
        unsigned int    iey_en                : 1   ; /* [8]  */
        unsigned int    iey_post              : 1   ; /* [9]  */
        unsigned int    sfkbig                : 1   ; /* [10]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_CFG;

/* Define the union U_VPSS_NR_ISHARPEN_REG3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    maxfactor0            : 8   ; /* [7..0]  */
        unsigned int    maxfactor1            : 8   ; /* [15..8]  */
        unsigned int    maxfactor2            : 8   ; /* [23..16]  */
        unsigned int    maxfactor3            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_ISHARPEN_REG3;

/* Define the union U_VPSS_NR_SFK0_RDESALTEX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesaltex0       : 8   ; /* [7..0]  */
        unsigned int    sfk0_rdesaltex1       : 9   ; /* [16..8]  */
        unsigned int    sfk0_rdesaltex2       : 12  ; /* [28..17]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALTEX;

/* Define the union U_VPSS_NR_SFK0_RFRID */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rfrid            : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RFRID;

/* Define the union U_VPSS_NR_SFK0_RDEPEPP0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepepp0         : 14  ; /* [13..0]  */
        unsigned int    reserved_0            : 18  ; /* [31..14]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPP0;

/* Define the union U_VPSS_NR_SFK0_RDEPEPP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepepp1         : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPP1;

/* Define the union U_VPSS_NR_SFK0_RDEPEPP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepepp2         : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPP2;

/* Define the union U_VPSS_NR_SFK0_RDEPEPP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepepp3         : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPP3;

/* Define the union U_VPSS_NR_SFK0_RDEPEPP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepepp4         : 18  ; /* [17..0]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPP4;

/* Define the union U_VPSS_NR_SFK0_RDEPEPP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepepp5         : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPP5;

/* Define the union U_VPSS_NR_SFK0_RDESALT0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesalt0         : 14  ; /* [13..0]  */
        unsigned int    reserved_0            : 18  ; /* [31..14]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALT0;

/* Define the union U_VPSS_NR_SFK0_RDESALT1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesalt1         : 15  ; /* [14..0]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALT1;

/* Define the union U_VPSS_NR_SFK0_RDESALT2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesalt2         : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALT2;

/* Define the union U_VPSS_NR_SFK0_RDESALT3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesalt3         : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALT3;

/* Define the union U_VPSS_NR_SFK0_RDESALT4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesalt4         : 18  ; /* [17..0]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALT4;

/* Define the union U_VPSS_NR_SFK0_RDESALT5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdesalt5         : 19  ; /* [18..0]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDESALT5;

/* Define the union U_VPSS_NR_SFK0_RDEPEPPEX */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfk0_rdepeppex0       : 8   ; /* [7..0]  */
        unsigned int    sfk0_rdepeppex1       : 9   ; /* [16..8]  */
        unsigned int    sfk0_rdepeppex2       : 12  ; /* [28..17]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_SFK0_RDEPEPPEX;

/* Define the union U_VPSS_NRY1_DILATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horzdila         : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 3   ; /* [7..5]  */
        unsigned int    nry1_vertdila         : 5   ; /* [12..8]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    nry1_horzdilamode     : 1   ; /* [16]  */
        unsigned int    reserved_2            : 3   ; /* [19..17]  */
        unsigned int    nry1_vertdilamode     : 1   ; /* [20]  */
        unsigned int    reserved_3            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_DILATE;

/* Define the union U_VPSS_NRY2_DILATE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horzdila         : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 3   ; /* [7..5]  */
        unsigned int    nry2_vertdila         : 5   ; /* [12..8]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    nry2_horzdilamode     : 1   ; /* [16]  */
        unsigned int    reserved_2            : 3   ; /* [19..17]  */
        unsigned int    nry2_vertdilamode     : 1   ; /* [20]  */
        unsigned int    reserved_3            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_DILATE;

/* Define the union U_VPSS_NRY0_HSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DUALDIR;

/* Define the union U_VPSS_NRY0_HSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry0_horz_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry0_horz_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_RFR;

/* Define the union U_VPSS_NRY0_HSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DS1;

/* Define the union U_VPSS_NRY0_HSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DS2;

/* Define the union U_VPSS_NRY0_HSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DS3;

/* Define the union U_VPSS_NRY0_HSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DS4;

/* Define the union U_VPSS_NRY0_HSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DS5;

/* Define the union U_VPSS_NRY0_HSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DP1;

/* Define the union U_VPSS_NRY0_HSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DP2;

/* Define the union U_VPSS_NRY0_HSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DP3;

/* Define the union U_VPSS_NRY0_HSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DP4;

/* Define the union U_VPSS_NRY0_HSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_horz_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_HSFI_DP5;

/* Define the union U_VPSS_NRY0_VSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DUALDIR;

/* Define the union U_VPSS_NRY0_VSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry0_vert_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry0_vert_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_RFR;

/* Define the union U_VPSS_NRY0_VSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DS1;

/* Define the union U_VPSS_NRY0_VSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DS2;

/* Define the union U_VPSS_NRY0_VSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DS3;

/* Define the union U_VPSS_NRY0_VSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DS4;

/* Define the union U_VPSS_NRY0_VSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DS5;

/* Define the union U_VPSS_NRY0_VSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DP1;

/* Define the union U_VPSS_NRY0_VSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DP2;

/* Define the union U_VPSS_NRY0_VSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DP3;

/* Define the union U_VPSS_NRY0_VSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DP4;

/* Define the union U_VPSS_NRY0_VSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_vert_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_VSFI_DP5;

/* Define the union U_VPSS_NRY0_REG5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_msfrbase0        : 4   ; /* [3..0]  */
        unsigned int    nry0_msfrbase1        : 4   ; /* [7..4]  */
        unsigned int    nry0_msfrbase2        : 4   ; /* [11..8]  */
        unsigned int    nry0_msfrpeak0        : 4   ; /* [15..12]  */
        unsigned int    nry0_msfrpeak1        : 4   ; /* [19..16]  */
        unsigned int    nry0_msfrpeak2        : 4   ; /* [23..20]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG5;

/* Define the union U_VPSS_NRY0_REG6 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_sfrweight        : 5   ; /* [4..0]  */
        unsigned int    nry0_maxwnd           : 2   ; /* [6..5]  */
        unsigned int    nry0_tstype           : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG6;

/* Define the union U_VPSS_NRY0_REG7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_mmadithr0        : 12  ; /* [11..0]  */
        unsigned int    nry0_mmadislp0        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG7;

/* Define the union U_VPSS_NRY0_REG8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_mmadithr1        : 12  ; /* [11..0]  */
        unsigned int    nry0_mmadislp1        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG8;

/* Define the union U_VPSS_NRY0_REG9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_mmadithr2        : 12  ; /* [11..0]  */
        unsigned int    nry0_mmadislp2        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG9;

/* Define the union U_VPSS_NRY0_REG10 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_msfrpkrate0      : 5   ; /* [4..0]  */
        unsigned int    nry0_msfrpkrate1      : 5   ; /* [9..5]  */
        unsigned int    nry0_msfrpkrate2      : 5   ; /* [14..10]  */
        unsigned int    nry0_blsben           : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG10;

/* Define the union U_VPSS_NRY0_REG11 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_temporate0       : 5   ; /* [4..0]  */
        unsigned int    nry0_temporate1       : 5   ; /* [9..5]  */
        unsigned int    nry0_masfrrate        : 5   ; /* [14..10]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG11;

/* Define the union U_VPSS_NRY0_REG12 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_tmadirate0       : 5   ; /* [4..0]  */
        unsigned int    nry0_tmadirate1       : 5   ; /* [9..5]  */
        unsigned int    nry0_madimask         : 6   ; /* [15..10]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG12;

/* Define the union U_VPSS_NRY0_REG13 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_rfroutweight     : 7   ; /* [6..0]  */
        unsigned int    nry0_refmode          : 2   ; /* [8..7]  */
        unsigned int    nry0_outmode          : 2   ; /* [10..9]  */
        unsigned int    nry0_mtsrpmi          : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG13;

/* Define the union U_VPSS_NRY0_REG14 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_refweight1       : 8   ; /* [7..0]  */
        unsigned int    nry0_mamirange        : 8   ; /* [15..8]  */
        unsigned int    nry0_mamicorng        : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG14;

/* Define the union U_VPSS_NRY0_REG15 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_refweight2       : 1   ; /* [0]  */
        unsigned int    nry0_refweight        : 5   ; /* [5..1]  */
        unsigned int    nry0_mamicoslp        : 2   ; /* [7..6]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG15;

/* Define the union U_VPSS_NRY0_REG16 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_msfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry0_msfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry0_sfrlimflg0       : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG16;

/* Define the union U_VPSS_NRY0_REG17 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_ssfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry0_ssfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry0_sfrlimflg1       : 1   ; /* [12]  */
        unsigned int    nry0_tempothr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG17;

/* Define the union U_VPSS_NRY0_REG18 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_mtsrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry0_mtsrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry0_sfrlimflg2       : 1   ; /* [12]  */
        unsigned int    nry0_tmadithr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG18;

/* Define the union U_VPSS_NRY0_REG19 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_ymotionthr0      : 12  ; /* [11..0]  */
        unsigned int    nry0_mdafactor        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG19;

/* Define the union U_VPSS_NRY0_REG20 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_ymotionthr1      : 12  ; /* [11..0]  */
        unsigned int    nry0_peakratio        : 3   ; /* [14..12]  */
        unsigned int    nry0_madfactor        : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG20;

/* Define the union U_VPSS_NRY0_REG21 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_ymotionslp0      : 4   ; /* [3..0]  */
        unsigned int    nry0_dadfactor        : 3   ; /* [6..4]  */
        unsigned int    nry0_ymotionslp1      : 4   ; /* [10..7]  */
        unsigned int    nry0_ydeltaslp        : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG21;

/* Define the union U_VPSS_NRY0_REG22 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_ymothdltmadislp0 : 4   ; /* [3..0]  */
        unsigned int    nry0_ymothdltmadimax0 : 12  ; /* [15..4]  */
        unsigned int    nry0_ymothdltmadislp1 : 4   ; /* [19..16]  */
        unsigned int    nry0_ymothdltmadimax1 : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG22;

/* Define the union U_VPSS_NRY0_REG23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_maprefweight     : 16  ; /* [15..0]  */
        unsigned int    nry0_ydeltathr        : 12  ; /* [27..16]  */
        unsigned int    nry0_exmode           : 2   ; /* [29..28]  */
        unsigned int    nry0_extype           : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG23;

/* Define the union U_VPSS_NRY0_REG24 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_ymothdltmadithr0 : 9   ; /* [8..0]  */
        unsigned int    nry0_bmotlsben        : 1   ; /* [9]  */
        unsigned int    nry0_ymothdltmadithr1 : 9   ; /* [18..10]  */
        unsigned int    nry0_bmotmaxen        : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG24;

/* Define the union U_VPSS_NRY0_REG25 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry0_exmadithr        : 12  ; /* [11..0]  */
        unsigned int    nry0_exmadislp        : 4   ; /* [15..12]  */
        unsigned int    nry0_exmadpthr        : 12  ; /* [27..16]  */
        unsigned int    nry0_exmadpslp        : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY0_REG25;

/* Define the union U_VPSS_NRY1_HSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DUALDIR;

/* Define the union U_VPSS_NRY1_HSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry1_horz_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry1_horz_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_RFR;

/* Define the union U_VPSS_NRY1_HSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DS1;

/* Define the union U_VPSS_NRY1_HSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DS2;

/* Define the union U_VPSS_NRY1_HSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DS3;

/* Define the union U_VPSS_NRY1_HSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DS4;

/* Define the union U_VPSS_NRY1_HSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DS5;

/* Define the union U_VPSS_NRY1_HSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DP1;

/* Define the union U_VPSS_NRY1_HSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DP2;

/* Define the union U_VPSS_NRY1_HSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DP3;

/* Define the union U_VPSS_NRY1_HSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DP4;

/* Define the union U_VPSS_NRY1_HSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_horz_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_HSFI_DP5;

/* Define the union U_VPSS_NRY1_VSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DUALDIR;

/* Define the union U_VPSS_NRY1_VSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry1_vert_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry1_vert_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_RFR;

/* Define the union U_VPSS_NRY1_VSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DS1;

/* Define the union U_VPSS_NRY1_VSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DS2;

/* Define the union U_VPSS_NRY1_VSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DS3;

/* Define the union U_VPSS_NRY1_VSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DS4;

/* Define the union U_VPSS_NRY1_VSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DS5;

/* Define the union U_VPSS_NRY1_VSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DP1;

/* Define the union U_VPSS_NRY1_VSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DP2;

/* Define the union U_VPSS_NRY1_VSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DP3;

/* Define the union U_VPSS_NRY1_VSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DP4;

/* Define the union U_VPSS_NRY1_VSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_vert_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_VSFI_DP5;

/* Define the union U_VPSS_NRY1_REG26 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_msfrbase0        : 4   ; /* [3..0]  */
        unsigned int    nry1_msfrbase1        : 4   ; /* [7..4]  */
        unsigned int    nry1_msfrbase2        : 4   ; /* [11..8]  */
        unsigned int    nry1_msfrpeak0        : 4   ; /* [15..12]  */
        unsigned int    nry1_msfrpeak1        : 4   ; /* [19..16]  */
        unsigned int    nry1_msfrpeak2        : 4   ; /* [23..20]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG26;

/* Define the union U_VPSS_NRY1_REG27 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_sfrweight        : 5   ; /* [4..0]  */
        unsigned int    nry1_maxwnd           : 2   ; /* [6..5]  */
        unsigned int    nry1_tstype           : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG27;

/* Define the union U_VPSS_NRY1_REG28 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_mmadithr0        : 12  ; /* [11..0]  */
        unsigned int    nry1_mmadislp0        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG28;

/* Define the union U_VPSS_NRY1_REG29 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_mmadithr1        : 12  ; /* [11..0]  */
        unsigned int    nry1_mmadislp1        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG29;

/* Define the union U_VPSS_NRY1_REG30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_mmadithr2        : 12  ; /* [11..0]  */
        unsigned int    nry1_mmadislp2        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG30;

/* Define the union U_VPSS_NRY1_REG31 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_msfrpkrate0      : 5   ; /* [4..0]  */
        unsigned int    nry1_msfrpkrate1      : 5   ; /* [9..5]  */
        unsigned int    nry1_msfrpkrate2      : 5   ; /* [14..10]  */
        unsigned int    nry1_blsben           : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG31;

/* Define the union U_VPSS_NRY1_REG32 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_temporate0       : 5   ; /* [4..0]  */
        unsigned int    nry1_temporate1       : 5   ; /* [9..5]  */
        unsigned int    nry1_masfrrate        : 5   ; /* [14..10]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG32;

/* Define the union U_VPSS_NRY1_REG33 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_tmadirate0       : 5   ; /* [4..0]  */
        unsigned int    nry1_tmadirate1       : 5   ; /* [9..5]  */
        unsigned int    nry1_madimask         : 6   ; /* [15..10]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG33;

/* Define the union U_VPSS_NRY1_REG34 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_rfroutweight     : 7   ; /* [6..0]  */
        unsigned int    nry1_refmode          : 2   ; /* [8..7]  */
        unsigned int    nry1_outmode          : 2   ; /* [10..9]  */
        unsigned int    nry1_mtsrpmi          : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG34;

/* Define the union U_VPSS_NRY1_REG35 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_refweight1       : 8   ; /* [7..0]  */
        unsigned int    nry1_mamirange        : 8   ; /* [15..8]  */
        unsigned int    nry1_mamicorng        : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG35;

/* Define the union U_VPSS_NRY1_REG36 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_refweight2       : 1   ; /* [0]  */
        unsigned int    nry1_refweight        : 5   ; /* [5..1]  */
        unsigned int    nry1_mamicoslp        : 2   ; /* [7..6]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG36;

/* Define the union U_VPSS_NRY1_REG37 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_msfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry1_msfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry1_sfrlimflg0       : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG37;

/* Define the union U_VPSS_NRY1_REG38 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_ssfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry1_ssfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry1_sfrlimflg1       : 1   ; /* [12]  */
        unsigned int    nry1_tempothr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG38;

/* Define the union U_VPSS_NRY1_REG39 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_mtsrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry1_mtsrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry1_sfrlimflg2       : 1   ; /* [12]  */
        unsigned int    nry1_tmadithr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG39;

/* Define the union U_VPSS_NRY1_REG40 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_ymotionthr0      : 12  ; /* [11..0]  */
        unsigned int    nry1_mdafactor        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG40;

/* Define the union U_VPSS_NRY1_REG41 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_ymotionthr1      : 12  ; /* [11..0]  */
        unsigned int    nry1_peakratio        : 3   ; /* [14..12]  */
        unsigned int    nry1_madfactor        : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG41;

/* Define the union U_VPSS_NRY1_REG42 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_ymotionslp0      : 4   ; /* [3..0]  */
        unsigned int    nry1_dadfactor        : 3   ; /* [6..4]  */
        unsigned int    nry1_ymotionslp1      : 4   ; /* [10..7]  */
        unsigned int    nry1_ydeltaslp        : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG42;

/* Define the union U_VPSS_NRY1_REG43 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_ymothdltmadislp0 : 4   ; /* [3..0]  */
        unsigned int    nry1_ymothdltmadimax0 : 12  ; /* [15..4]  */
        unsigned int    nry1_ymothdltmadislp1 : 4   ; /* [19..16]  */
        unsigned int    nry1_ymothdltmadimax1 : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG43;

/* Define the union U_VPSS_NRY1_REG44 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_maprefweight     : 16  ; /* [15..0]  */
        unsigned int    nry1_ydeltathr        : 12  ; /* [27..16]  */
        unsigned int    nry1_exmode           : 2   ; /* [29..28]  */
        unsigned int    nry1_extype           : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG44;

/* Define the union U_VPSS_NRY1_REG45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_ymothdltmadithr0 : 9   ; /* [8..0]  */
        unsigned int    nry1_bmotlsben        : 1   ; /* [9]  */
        unsigned int    nry1_ymothdltmadithr1 : 9   ; /* [18..10]  */
        unsigned int    nry1_bmotmaxen        : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG45;

/* Define the union U_VPSS_NRY1_REG46 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry1_exmadithr        : 12  ; /* [11..0]  */
        unsigned int    nry1_exmadislp        : 4   ; /* [15..12]  */
        unsigned int    nry1_exmadpthr        : 12  ; /* [27..16]  */
        unsigned int    nry1_exmadpslp        : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY1_REG46;

/* Define the union U_VPSS_NRY2_HSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DUALDIR;

/* Define the union U_VPSS_NRY2_HSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry2_horz_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry2_horz_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_RFR;

/* Define the union U_VPSS_NRY2_HSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DS1;

/* Define the union U_VPSS_NRY2_HSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DS2;

/* Define the union U_VPSS_NRY2_HSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DS3;

/* Define the union U_VPSS_NRY2_HSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DS4;

/* Define the union U_VPSS_NRY2_HSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DS5;

/* Define the union U_VPSS_NRY2_HSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DP1;

/* Define the union U_VPSS_NRY2_HSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DP2;

/* Define the union U_VPSS_NRY2_HSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DP3;

/* Define the union U_VPSS_NRY2_HSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DP4;

/* Define the union U_VPSS_NRY2_HSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_horz_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_HSFI_DP5;

/* Define the union U_VPSS_NRY2_VSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DUALDIR;

/* Define the union U_VPSS_NRY2_VSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry2_vert_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry2_vert_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_RFR;

/* Define the union U_VPSS_NRY2_VSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DS1;

/* Define the union U_VPSS_NRY2_VSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DS2;

/* Define the union U_VPSS_NRY2_VSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DS3;

/* Define the union U_VPSS_NRY2_VSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DS4;

/* Define the union U_VPSS_NRY2_VSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DS5;

/* Define the union U_VPSS_NRY2_VSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DP1;

/* Define the union U_VPSS_NRY2_VSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DP2;

/* Define the union U_VPSS_NRY2_VSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DP3;

/* Define the union U_VPSS_NRY2_VSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DP4;

/* Define the union U_VPSS_NRY2_VSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_vert_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_VSFI_DP5;

/* Define the union U_VPSS_NRY2_REG26 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_msfrbase0        : 4   ; /* [3..0]  */
        unsigned int    nry2_msfrbase1        : 4   ; /* [7..4]  */
        unsigned int    nry2_msfrbase2        : 4   ; /* [11..8]  */
        unsigned int    nry2_msfrpeak0        : 4   ; /* [15..12]  */
        unsigned int    nry2_msfrpeak1        : 4   ; /* [19..16]  */
        unsigned int    nry2_msfrpeak2        : 4   ; /* [23..20]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG26;

/* Define the union U_VPSS_NRY2_REG27 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_sfrweight        : 5   ; /* [4..0]  */
        unsigned int    nry2_maxwnd           : 2   ; /* [6..5]  */
        unsigned int    nry2_tstype           : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG27;

/* Define the union U_VPSS_NRY2_REG28 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_mmadithr0        : 12  ; /* [11..0]  */
        unsigned int    nry2_mmadislp0        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG28;

/* Define the union U_VPSS_NRY2_REG29 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_mmadithr1        : 12  ; /* [11..0]  */
        unsigned int    nry2_mmadislp1        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG29;

/* Define the union U_VPSS_NRY2_REG30 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_mmadithr2        : 12  ; /* [11..0]  */
        unsigned int    nry2_mmadislp2        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG30;

/* Define the union U_VPSS_NRY2_REG31 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_msfrpkrate0      : 5   ; /* [4..0]  */
        unsigned int    nry2_msfrpkrate1      : 5   ; /* [9..5]  */
        unsigned int    nry2_msfrpkrate2      : 5   ; /* [14..10]  */
        unsigned int    nry2_blsben           : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG31;

/* Define the union U_VPSS_NRY2_REG32 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_temporate0       : 5   ; /* [4..0]  */
        unsigned int    nry2_temporate1       : 5   ; /* [9..5]  */
        unsigned int    nry2_masfrrate        : 5   ; /* [14..10]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG32;

/* Define the union U_VPSS_NRY2_REG33 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_tmadirate0       : 5   ; /* [4..0]  */
        unsigned int    nry2_tmadirate1       : 5   ; /* [9..5]  */
        unsigned int    nry2_madimask         : 6   ; /* [15..10]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG33;

/* Define the union U_VPSS_NRY2_REG34 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_rfroutweight     : 7   ; /* [6..0]  */
        unsigned int    nry2_refmode          : 2   ; /* [8..7]  */
        unsigned int    nry2_outmode          : 2   ; /* [10..9]  */
        unsigned int    nry2_mtsrpmi          : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG34;

/* Define the union U_VPSS_NRY2_REG35 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_refweight1       : 8   ; /* [7..0]  */
        unsigned int    nry2_mamirange        : 8   ; /* [15..8]  */
        unsigned int    nry2_mamicorng        : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG35;

/* Define the union U_VPSS_NRY2_REG36 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_refweight2       : 1   ; /* [0]  */
        unsigned int    nry2_refweight        : 5   ; /* [5..1]  */
        unsigned int    nry2_mamicoslp        : 2   ; /* [7..6]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG36;

/* Define the union U_VPSS_NRY2_REG37 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_msfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry2_msfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry2_sfrlimflg0       : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG37;

/* Define the union U_VPSS_NRY2_REG38 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_ssfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry2_ssfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry2_sfrlimflg1       : 1   ; /* [12]  */
        unsigned int    nry2_tempothr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG38;

/* Define the union U_VPSS_NRY2_REG39 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_mtsrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry2_mtsrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry2_sfrlimflg2       : 1   ; /* [12]  */
        unsigned int    nry2_tmadithr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG39;

/* Define the union U_VPSS_NRY2_REG40 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_ymotionthr0      : 12  ; /* [11..0]  */
        unsigned int    nry2_mdafactor        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG40;

/* Define the union U_VPSS_NRY2_REG41 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_ymotionthr1      : 12  ; /* [11..0]  */
        unsigned int    nry2_peakratio        : 3   ; /* [14..12]  */
        unsigned int    nry2_madfactor        : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG41;

/* Define the union U_VPSS_NRY2_REG42 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_ymotionslp0      : 4   ; /* [3..0]  */
        unsigned int    nry2_dadfactor        : 3   ; /* [6..4]  */
        unsigned int    nry2_ymotionslp1      : 4   ; /* [10..7]  */
        unsigned int    nry2_ydeltaslp        : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG42;

/* Define the union U_VPSS_NRY2_REG43 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_ymothdltmadislp0 : 4   ; /* [3..0]  */
        unsigned int    nry2_ymothdltmadimax0 : 12  ; /* [15..4]  */
        unsigned int    nry2_ymothdltmadislp1 : 4   ; /* [19..16]  */
        unsigned int    nry2_ymothdltmadimax1 : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG43;

/* Define the union U_VPSS_NRY2_REG44 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_maprefweight     : 16  ; /* [15..0]  */
        unsigned int    nry2_ydeltathr        : 12  ; /* [27..16]  */
        unsigned int    nry2_exmode           : 2   ; /* [29..28]  */
        unsigned int    nry2_extype           : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG44;

/* Define the union U_VPSS_NRY2_REG45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_ymothdltmadithr0 : 9   ; /* [8..0]  */
        unsigned int    nry2_bmotlsben        : 1   ; /* [9]  */
        unsigned int    nry2_ymothdltmadithr1 : 9   ; /* [18..10]  */
        unsigned int    nry2_bmotmaxen        : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG45;

/* Define the union U_VPSS_NRY2_REG46 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry2_exmadithr        : 12  ; /* [11..0]  */
        unsigned int    nry2_exmadislp        : 4   ; /* [15..12]  */
        unsigned int    nry2_exmadpthr        : 12  ; /* [27..16]  */
        unsigned int    nry2_exmadpslp        : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY2_REG46;

/* Define the union U_VPSS_NRY3_HSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DUALDIR;

/* Define the union U_VPSS_NRY3_HSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry3_horz_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry3_horz_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_RFR;

/* Define the union U_VPSS_NRY3_HSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DS1;

/* Define the union U_VPSS_NRY3_HSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DS2;

/* Define the union U_VPSS_NRY3_HSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DS3;

/* Define the union U_VPSS_NRY3_HSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DS4;

/* Define the union U_VPSS_NRY3_HSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DS5;

/* Define the union U_VPSS_NRY3_HSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DP1;

/* Define the union U_VPSS_NRY3_HSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DP2;

/* Define the union U_VPSS_NRY3_HSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DP3;

/* Define the union U_VPSS_NRY3_HSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DP4;

/* Define the union U_VPSS_NRY3_HSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_horz_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_HSFI_DP5;

/* Define the union U_VPSS_NRY3_VSFI_DUALDIR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_dualdir     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DUALDIR;

/* Define the union U_VPSS_NRY3_VSFI_RFR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_rfr2        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    nry3_vert_rfr3        : 2   ; /* [5..4]  */
        unsigned int    reserved_1            : 2   ; /* [7..6]  */
        unsigned int    nry3_vert_rfr4        : 2   ; /* [9..8]  */
        unsigned int    reserved_2            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_RFR;

/* Define the union U_VPSS_NRY3_VSFI_DS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_ds1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DS1;

/* Define the union U_VPSS_NRY3_VSFI_DS2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_ds2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DS2;

/* Define the union U_VPSS_NRY3_VSFI_DS3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_ds3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DS3;

/* Define the union U_VPSS_NRY3_VSFI_DS4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_ds4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DS4;

/* Define the union U_VPSS_NRY3_VSFI_DS5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_ds5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DS5;

/* Define the union U_VPSS_NRY3_VSFI_DP1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_dp1         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DP1;

/* Define the union U_VPSS_NRY3_VSFI_DP2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_dp2         : 9   ; /* [8..0]  */
        unsigned int    reserved_0            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DP2;

/* Define the union U_VPSS_NRY3_VSFI_DP3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_dp3         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DP3;

/* Define the union U_VPSS_NRY3_VSFI_DP4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_dp4         : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DP4;

/* Define the union U_VPSS_NRY3_VSFI_DP5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_vert_dp5         : 12  ; /* [11..0]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_VSFI_DP5;

/* Define the union U_VPSS_NRY3_REG47 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_msfrbase0        : 4   ; /* [3..0]  */
        unsigned int    nry3_msfrbase1        : 4   ; /* [7..4]  */
        unsigned int    nry3_msfrbase2        : 4   ; /* [11..8]  */
        unsigned int    nry3_msfrpeak0        : 4   ; /* [15..12]  */
        unsigned int    nry3_msfrpeak1        : 4   ; /* [19..16]  */
        unsigned int    nry3_msfrpeak2        : 4   ; /* [23..20]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG47;

/* Define the union U_VPSS_NRY3_REG48 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_sfrweight        : 5   ; /* [4..0]  */
        unsigned int    nry3_maxwnd           : 2   ; /* [6..5]  */
        unsigned int    nry3_tstype           : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG48;

/* Define the union U_VPSS_NRY3_REG49 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_mmadithr0        : 12  ; /* [11..0]  */
        unsigned int    nry3_mmadislp0        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG49;

/* Define the union U_VPSS_NRY3_REG50 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_mmadithr1        : 12  ; /* [11..0]  */
        unsigned int    nry3_mmadislp1        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG50;

/* Define the union U_VPSS_NRY3_REG51 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_mmadithr2        : 12  ; /* [11..0]  */
        unsigned int    nry3_mmadislp2        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG51;

/* Define the union U_VPSS_NRY3_REG52 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_msfrpkrate0      : 5   ; /* [4..0]  */
        unsigned int    nry3_msfrpkrate1      : 5   ; /* [9..5]  */
        unsigned int    nry3_msfrpkrate2      : 5   ; /* [14..10]  */
        unsigned int    nry3_blsben           : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG52;

/* Define the union U_VPSS_NRY3_REG53 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_temporate0       : 5   ; /* [4..0]  */
        unsigned int    nry3_temporate1       : 5   ; /* [9..5]  */
        unsigned int    nry3_masfrrate        : 5   ; /* [14..10]  */
        unsigned int    reserved_0            : 17  ; /* [31..15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG53;

/* Define the union U_VPSS_NRY3_REG54 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_tmadirate0       : 5   ; /* [4..0]  */
        unsigned int    nry3_tmadirate1       : 5   ; /* [9..5]  */
        unsigned int    nry3_madimask         : 6   ; /* [15..10]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG54;

/* Define the union U_VPSS_NRY3_REG55 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_rfroutweight     : 7   ; /* [6..0]  */
        unsigned int    nry3_refmode          : 2   ; /* [8..7]  */
        unsigned int    nry3_outmode          : 2   ; /* [10..9]  */
        unsigned int    nry3_mtsrpmi          : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG55;

/* Define the union U_VPSS_NRY3_REG56 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_refweight1       : 8   ; /* [7..0]  */
        unsigned int    nry3_mamirange        : 8   ; /* [15..8]  */
        unsigned int    nry3_mamicorng        : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG56;

/* Define the union U_VPSS_NRY3_REG57 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_refweight2       : 1   ; /* [0]  */
        unsigned int    nry3_refweight        : 5   ; /* [5..1]  */
        unsigned int    nry3_mamicoslp        : 2   ; /* [7..6]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG57;

/* Define the union U_VPSS_NRY3_REG58 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_msfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry3_msfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry3_sfrlimflg0       : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG58;

/* Define the union U_VPSS_NRY3_REG59 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_ssfrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry3_ssfrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry3_sfrlimflg1       : 1   ; /* [12]  */
        unsigned int    nry3_tempothr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG59;

/* Define the union U_VPSS_NRY3_REG60 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_mtsrexthr        : 8   ; /* [7..0]  */
        unsigned int    nry3_mtsrexslp        : 4   ; /* [11..8]  */
        unsigned int    nry3_sfrlimflg2       : 1   ; /* [12]  */
        unsigned int    nry3_tmadithr1        : 8   ; /* [20..13]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG60;

/* Define the union U_VPSS_NRY3_REG61 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_ymotionthr0      : 12  ; /* [11..0]  */
        unsigned int    nry3_mdafactor        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG61;

/* Define the union U_VPSS_NRY3_REG62 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_ymotionthr1      : 12  ; /* [11..0]  */
        unsigned int    nry3_peakratio        : 3   ; /* [14..12]  */
        unsigned int    nry3_madfactor        : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG62;

/* Define the union U_VPSS_NRY3_REG63 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_ymotionslp0      : 4   ; /* [3..0]  */
        unsigned int    nry3_dadfactor        : 3   ; /* [6..4]  */
        unsigned int    nry3_ymotionslp1      : 4   ; /* [10..7]  */
        unsigned int    nry3_ydeltaslp        : 5   ; /* [15..11]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG63;

/* Define the union U_VPSS_NRY3_REG64 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_ymothdltmadislp0 : 4   ; /* [3..0]  */
        unsigned int    nry3_ymothdltmadimax0 : 12  ; /* [15..4]  */
        unsigned int    nry3_ymothdltmadislp1 : 4   ; /* [19..16]  */
        unsigned int    nry3_ymothdltmadimax1 : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG64;

/* Define the union U_VPSS_NRY3_REG65 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_maprefweight     : 16  ; /* [15..0]  */
        unsigned int    nry3_ydeltathr        : 12  ; /* [27..16]  */
        unsigned int    nry3_exmode           : 2   ; /* [29..28]  */
        unsigned int    nry3_extype           : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG65;

/* Define the union U_VPSS_NRY3_REG66 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_ymothdltmadithr0 : 9   ; /* [8..0]  */
        unsigned int    nry3_bmotlsben        : 1   ; /* [9]  */
        unsigned int    nry3_ymothdltmadithr1 : 9   ; /* [18..10]  */
        unsigned int    nry3_bmotmaxen        : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG66;

/* Define the union U_VPSS_NRY3_REG67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nry3_exmadithr        : 12  ; /* [11..0]  */
        unsigned int    nry3_exmadislp        : 4   ; /* [15..12]  */
        unsigned int    nry3_exmadpthr        : 12  ; /* [27..16]  */
        unsigned int    nry3_exmadpslp        : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NRY3_REG67;

/* Define the union U_VPSS_NR_C_PARA0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrc_iirrange          : 8   ; /* [7..0]  */
        unsigned int    nrc_sfrrate           : 6   ; /* [13..8]  */
        unsigned int    nrc_horrange          : 8   ; /* [21..14]  */
        unsigned int    nrc_tfrrate           : 6   ; /* [27..22]  */
        unsigned int    reserved_0            : 3   ; /* [30..28]  */
        unsigned int    boutputref            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_C_PARA0;

/* Define the union U_VPSS_NR_C_PARA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nrc_iirhbflg          : 1   ; /* [0]  */
        unsigned int    nrc_iirexflg          : 1   ; /* [1]  */
        unsigned int    nrc_horhbflg          : 1   ; /* [2]  */
        unsigned int    nrc_tfrexflg          : 1   ; /* [3]  */
        unsigned int    nrc_tfrprofile        : 6   ; /* [9..4]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} U_VPSS_NR_C_PARA1;





#endif
