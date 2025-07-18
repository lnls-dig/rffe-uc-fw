#ifndef CDCE906_H_
#define CDCE906_H_
#include "CDCE906_reg.hpp"
#include <cstdint>
#include "mbed.h"


enum cdce906_clk_src {
    CDCE906_CLK_SRC_CRYSTAL,
    CDCE906_CLK_SRC_CLKIN0_LVCMOS,
    CDCE906_CLK_SRC_CLKIN1_LVCMOS,
    CDCE906_CLK_SRC_CLKIN_DIFF,
    CDCE906_CLK_SRC_INVALID,
};

enum cdce906_pll_vco_mux {
    CDCE906_PLL_VCO_MUX_PLL,
    CDCE906_PLL_VCO_MUX_VCO,
};

enum cdce906_pll_fvco {
    CDCE906_PLL_FVCO_80_200MHZ,
    CDCE906_PLL_FVCO_180_300MHZ,
};

enum cdce906_s0_cfg {
    CDCE906_S0_CFG_POWER_DOWN_CTRL,
    CDCE906_S0_CFG_PLL_DIV_BYPASS_CTRL,
    CDCE906_S0_CFG_CLKSEL_CTRL,
    CDCE906_S0_CFG_I2C_ADDR,
};

enum cdce906_s1_cfg {
    CDCE906_S1_CFG_Yx_FIXED_OUTPUT_CTRL,
    CDCE906_S1_CFG_Yx_TRISTATE_CTRL,
    CDCE906_S1_CFG_I2C_ADDR,
    CDCE906_S1_CFG_INVALID,
};

enum cdce906_px_pll_sel {
    CDCE906_Px_PLL_SEL_BYPASS,
    CDCE906_Px_PLL_SEL_PLL1,
    CDCE906_Px_PLL_SEL_PLL2,
    CDCE906_Px_PLL_SEL_PLL2_SSC,
    CDCE906_Px_PLL_SEL_PLL3,
    CDCE906_Px_PLL_SEL_INVALID,
};

enum cdce906_yx_out_cfg {
    CDCE906_Yx_OUT_CFG_EN,
    CDCE906_Yx_OUT_CFG_EN_INV,
    CDCE906_Yx_OUT_CFG_DIS_LOW,
    CDCE906_Yx_OUT_CFG_DIS_HIGH,
};

enum cdce906_yx_slew_cfg {
    CDCE906_Yx_SLEW_CFG_NOMINAL,
    CDCE906_Yx_SLEW_CFG_NOMINAL_1NS,
    CDCE906_Yx_SLEW_CFG_NOMINAL_2NS,
    CDCE906_Yx_SLEW_CFG_NOMINAL_3NS,
};

enum cdce906_yx_px_sel {
    CDCE906_Yx_Px_SEL_P0,
    CDCE906_Yx_Px_SEL_P1,
    CDCE906_Yx_Px_SEL_P2,
    CDCE906_Yx_Px_SEL_P3,
    CDCE906_Yx_Px_SEL_P4,
    CDCE906_Yx_Px_SEL_P5,
    CDCE906_Yx_Px_SEL_INVALID,
};

enum cdce906_ssc_mod_amount {
    CDCE906_SSC_MOD_AMOUNT_OFF = 0,
    CDCE906_SSC_MOD_AMOUNT_0P1_CENTER,
    CDCE906_SSC_MOD_AMOUNT_0P25_CENTER,
    CDCE906_SSC_MOD_AMOUNT_0P4_CENTER,
    CDCE906_SSC_MOD_AMOUNT_1P0_DOWN,
    CDCE906_SSC_MOD_AMOUNT_1P5_DOWN,
    CDCE906_SSC_MOD_AMOUNT_2P0_DOWN,
    CDCE906_SSC_MOD_AMOUNT_3P0_DOWN,
};

enum cdce906_ssc_mod_freq {
    CDCE906_SSC_MOD_FREQ_5680 = 0,
    CDCE906_SSC_MOD_FREQ_5412,
    CDCE906_SSC_MOD_FREQ_5144,
    CDCE906_SSC_MOD_FREQ_4876,
    CDCE906_SSC_MOD_FREQ_4608,
    CDCE906_SSC_MOD_FREQ_4340,
    CDCE906_SSC_MOD_FREQ_4072,
    CDCE906_SSC_MOD_FREQ_3804,
    CDCE906_SSC_MOD_FREQ_3536,
    CDCE906_SSC_MOD_FREQ_3286,
    CDCE906_SSC_MOD_FREQ_3000,
    CDCE906_SSC_MOD_FREQ_2732,
    CDCE906_SSC_MOD_FREQ_2464,
    CDCE906_SSC_MOD_FREQ_2196,
    CDCE906_SSC_MOD_FREQ_1928,
    CDCE906_SSC_MOD_FREQ_1660,
};


struct cdce906_cfg {
    enum cdce906_clk_src clksrc;
    uint16_t pll_div_m[3];
    uint16_t pll_div_n[3];
    enum cdce906_pll_vco_mux pll_vco_mux[3];
    enum cdce906_pll_fvco pll_fvco[3];
    enum cdce906_ssc_mod_amount ssc_mod_amount;
    enum cdce906_ssc_mod_freq ssc_mod_freq;
    enum cdce906_s0_cfg s0_cfg;
    enum cdce906_s1_cfg s1_cfg;
    enum cdce906_px_pll_sel pll_sel[6];
    uint8_t p_div[6];
    enum cdce906_yx_slew_cfg y_slew[6];
    enum cdce906_yx_px_sel y_p_sel[6];
    enum cdce906_yx_out_cfg y_out[6];
};

class CDCE906
{
public:

    CDCE906(mbed::I2C * i2c, int8_t i2c_addr);
    ~CDCE906() = default;

    int cdce906_read_cfg(cdce906_cfg &cfg, uint8_t (&arr)[25]);
    int cdce906_write_cfg(cdce906_cfg &cfg, uint8_t (&arr)[25]);
    int cdce906_write_eeprom(cdce906_cfg &cfg);

private:
    mbed::I2C * _i2c;

    int8_t _i2c_addr;

    void cdce906_get_ssc(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_ssc(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_pll_fvco(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_pll_fvco(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_pll_mux(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_pll_mux(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_clksrc(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_clksrc(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_px_pll_sel(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_px_pll_sel(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_pll_div(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_pll_div(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_yx_px_sel(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_yx_px_sel(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_yx_slew(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_yx_slew(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_px_div(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_px_div(cdce906_cfg &cfg, uint8_t * buff);

    void cdce906_get_yx_out_cfg(cdce906_cfg &cfg, const uint8_t * buff);
    void cdce906_set_yx_out_cfg(cdce906_cfg &cfg, uint8_t * buff);

};



#endif
