/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "mc.h"
#include "car.h"
#include "timers.h"

void mc_config_tsec_carveout(uint32_t bom, uint32_t size1mb, bool lock)
{
    MAKE_MC_REG(MC_SEC_CARVEOUT_BOM) = bom;
    MAKE_MC_REG(MC_SEC_CARVEOUT_SIZE_MB) = size1mb;
    
    if (lock)
        MAKE_MC_REG(MC_SEC_CARVEOUT_REG_CTRL) = 1;
}

void mc_config_carveout()
{
    *(volatile uint32_t *)0x8005FFFC = 0xC0EDBBCC;
    
    MAKE_MC_REG(MC_VIDEO_PROTECT_GPU_OVERRIDE_0) = 1;
    MAKE_MC_REG(MC_VIDEO_PROTECT_GPU_OVERRIDE_1) = 0;
    MAKE_MC_REG(MC_VIDEO_PROTECT_BOM) = 0;
    MAKE_MC_REG(MC_VIDEO_PROTECT_SIZE_MB) = 0;
    MAKE_MC_REG(MC_VIDEO_PROTECT_REG_CTRL) = 1;

    mc_config_tsec_carveout(0, 0, true);

    MAKE_MC_REG(MC_MTS_CARVEOUT_BOM) = 0;
    MAKE_MC_REG(MC_MTS_CARVEOUT_SIZE_MB) = 0;
    MAKE_MC_REG(MC_MTS_CARVEOUT_ADR_HI) = 0;
    MAKE_MC_REG(MC_MTS_CARVEOUT_REG_CTRL) = 1;
    
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_BOM) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_BOM_HI) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_SIZE_128KB) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_FORCE_INTERNAL_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_FORCE_INTERNAL_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_FORCE_INTERNAL_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_FORCE_INTERNAL_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CLIENT_FORCE_INTERNAL_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT1_CFG0) = 0x4000006;

    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_BOM) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_BOM_HI) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_SIZE_128KB) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_ACCESS2) = (BIT(CSR_GPUSRD) | BIT(CSW_GPUSWR));
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_ACCESS4) = (BIT(CSR_GPUSRD2) | BIT(CSW_GPUSWR2));
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_FORCE_INTERNAL_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_FORCE_INTERNAL_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_FORCE_INTERNAL_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_FORCE_INTERNAL_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CLIENT_FORCE_INTERNAL_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT3_CFG0) = 0x4401E7E;
    
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_BOM) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_BOM_HI) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_SIZE_128KB) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_FORCE_INTERNAL_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_FORCE_INTERNAL_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_FORCE_INTERNAL_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_FORCE_INTERNAL_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CLIENT_FORCE_INTERNAL_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT4_CFG0) = 0x8F;
    
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_BOM) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_BOM_HI) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_SIZE_128KB) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_FORCE_INTERNAL_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_FORCE_INTERNAL_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_FORCE_INTERNAL_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_FORCE_INTERNAL_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CLIENT_FORCE_INTERNAL_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT5_CFG0) = 0x8F;
}

void mc_config_carveout_finalize()
{
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_BOM) = 0x80020000;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_BOM_HI) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_SIZE_128KB) = 2;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_ACCESS2) = (BIT(CSR_GPUSRD) | BIT(CSW_GPUSWR));
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_ACCESS4) = (BIT(CSR_GPUSRD2) | BIT(CSW_GPUSWR2));
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_FORCE_INTERNAL_ACCESS0) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_FORCE_INTERNAL_ACCESS1) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_FORCE_INTERNAL_ACCESS2) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_FORCE_INTERNAL_ACCESS3) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CLIENT_FORCE_INTERNAL_ACCESS4) = 0;
    MAKE_MC_REG(MC_SECURITY_CARVEOUT2_CFG0) = 0x440167E;
}

void mc_enable_ahb_redirect()
{
    volatile tegra_car_t *car = car_get_regs();
    car->lvl2_clk_gate_ovrd = ((car->lvl2_clk_gate_ovrd & 0xFFF7FFFF) | 0x80000);
    
    MAKE_MC_REG(MC_IRAM_BOM) = 0x40000000;
    MAKE_MC_REG(MC_IRAM_TOM) = 0x4003F000;
}

void mc_disable_ahb_redirect()
{
    volatile tegra_car_t *car = car_get_regs();
    
    MAKE_MC_REG(MC_IRAM_BOM) = 0xFFFFF000;
    MAKE_MC_REG(MC_IRAM_TOM) = 0;
    
    car->lvl2_clk_gate_ovrd &= 0xFFF7FFFF;
}

void mc_enable()
{
    volatile tegra_car_t *car = car_get_regs();
    
    /* Set EMC clock source. */
    car->clk_source_emc = ((car->clk_source_emc & 0x1FFFFFFF) | 0x40000000);
    
    /* Enable MIPI CAL clock. */
    car->clk_enb_h_set = ((car->clk_enb_h_set & 0xFDFFFFFF) | 0x2000000);
    
    /* Enable MC clock. */
    car->clk_enb_h_set = ((car->clk_enb_h_set & 0xFFFFFFFE) | 1);

    /* Enable EMC DLL clock. */
    car->clk_enb_x_set = ((car->clk_enb_x_set & 0xFFFFBFFF) | 0x4000);
    
    /* Clear EMC and MC reset. */
    /* NOTE: [4.0.0+] This was changed to use the right register. */
    /* car->rst_dev_h_set = 0x2000001; */
    car->rst_dev_h_clr = 0x2000001; 
    udelay(5);

    mc_disable_ahb_redirect();
}