/**
 *    @file     pll_cfg.hpp
 *    @author   João Victor Silva e Santos (joao.ssantos@lnls.br)
 *
 *    @brief Header file defining the default configuration for the CDCE906 PLL synthesizer.
 *
 *    @details
 *      Provides a global configuration instance (`write_cfg`) for the Texas Instruments
 *      CDCE906 programmable clock synthesizer. This configuration sets up:
 *        - Input clock source (CLKIN0, LVCMOS)
 *        - PLL dividers (M, N) for frequency multiplication and division
 *        - VCO muxing and operating frequency range
 *        - Spread-spectrum modulation (SSC) settings
 *        - Output selection (PLL or bypass paths per output)
 *        - Output dividers (P) and slew rate configuration
 *        - Output enable/disable control for Y0–Y5
 *
 *      The configuration is used by the RFFE system to generate stable and
 *      synchronized clock signals for internal modules.
 *
 *    @date 2025
 *
 *    @copyright
 *      Copyright (C) 2025 CNPEM (cnpem.br)
 */


#pragma once
#include "CDCE906.hpp"

/**
 * @brief Default configuration instance for CDCE906 PLL synthesizer.
 */
extern struct cdce906_cfg write_cfg;
