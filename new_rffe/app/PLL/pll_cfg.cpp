#include "CDCE906_reg.hpp"
#include "pll_cfg.hpp"

struct cdce906_cfg write_cfg =
{
    CDCE906_CLK_SRC_CLKIN0_LVCMOS, // clksrc
    {3, 1, 1}, // pll_div_m
    {25, 1, 1}, // pll_div_n
    { // pll_vco_mux
        CDCE906_PLL_VCO_MUX_PLL,
        CDCE906_PLL_VCO_MUX_PLL,
        CDCE906_PLL_VCO_MUX_PLL,
    },
    { // pll_fvco
        CDCE906_PLL_FVCO_80_200MHZ,
        CDCE906_PLL_FVCO_80_200MHZ,
        CDCE906_PLL_FVCO_80_200MHZ,
    },
    CDCE906_SSC_MOD_AMOUNT_OFF, // ssc_mod_amount
    CDCE906_SSC_MOD_FREQ_3286, // ssc_mod_freq
    CDCE906_S0_CFG_POWER_DOWN_CTRL, // s0_cfg
    CDCE906_S1_CFG_Yx_FIXED_OUTPUT_CTRL, // s1_cfg
    { // pll_sel
        CDCE906_Px_PLL_SEL_PLL1,
        CDCE906_Px_PLL_SEL_BYPASS,
        CDCE906_Px_PLL_SEL_BYPASS,
        CDCE906_Px_PLL_SEL_BYPASS,
        CDCE906_Px_PLL_SEL_BYPASS,
        CDCE906_Px_PLL_SEL_BYPASS,
    },
    {6, 1, 1, 1, 1, 1}, // p_div
    { // y_slew
        CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
        CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
        CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
        CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
        CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
        CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
    },
    { // y_p_sel
        CDCE906_Yx_Px_SEL_P0,
        CDCE906_Yx_Px_SEL_P1,
        CDCE906_Yx_Px_SEL_P2,
        CDCE906_Yx_Px_SEL_P3,
        CDCE906_Yx_Px_SEL_P4,
        CDCE906_Yx_Px_SEL_P5,
    },
    { // y_out
        CDCE906_Yx_OUT_CFG_EN,
        CDCE906_Yx_OUT_CFG_DIS_LOW,
        CDCE906_Yx_OUT_CFG_DIS_LOW,
        CDCE906_Yx_OUT_CFG_DIS_LOW,
        CDCE906_Yx_OUT_CFG_DIS_LOW,
        CDCE906_Yx_OUT_CFG_DIS_LOW,
    },
};
