/*
 * Defining registers address and its bit definitions of MAX77620 and MAX20024
 *
 * Copyright (C) 2016 NVIDIA CORPORATION. All rights reserved.
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef _MFD_MAX77620_H_
#define _MFD_MAX77620_H_

#define MAX77620_I2C_ADDR     0x3C
#define MAX77620_RTC_I2C_ADDR 0x68

/* RTC Registers */
#define MAX77620_REG_RTCINT     0x00
#define MAX77620_REG_RTCINTM        0x01
#define MAX77620_REG_RTCCNTLM       0x02
#define MAX77620_REG_RTCCNTL        0x03
#define MAX77620_REG_RTCUPDATE0     0x04
#define MAX77620_REG_RTCUPDATE1     0x05
#define MAX77620_REG_RTCSMPL        0x06
#define MAX77620_REG_RTCSEC     0x07
#define MAX77620_REG_RTCMIN     0x08
#define MAX77620_REG_RTCHOUR        0x09
#define MAX77620_REG_RTCDOW     0x0A
#define MAX77620_REG_RTCMONTH       0x0B
#define MAX77620_REG_RTCYEAR        0x0C
#define MAX77620_REG_RTCDOM     0x0D
#define MAX77620_REG_RTCSECA1       0x0E
#define MAX77620_REG_RTCMINA1       0x0F
#define MAX77620_REG_RTCHOURA1      0x10
#define MAX77620_REG_RTCDOWA1       0x11
#define MAX77620_REG_RTCMONTHA1     0x12
#define MAX77620_REG_RTCYEARA1      0x13
#define MAX77620_REG_RTCDOMA1       0x14
#define MAX77620_REG_RTCSECA2       0x15
#define MAX77620_REG_RTCMINA2       0x16
#define MAX77620_REG_RTCHOURA2      0x17
#define MAX77620_REG_RTCDOWA2       0x18
#define MAX77620_REG_RTCMONTHA2     0x19
#define MAX77620_REG_RTCYEARA2      0x1A
#define MAX77620_REG_RTCDOMA2       0x1B

/* GLOBAL, PMIC, GPIO, FPS, ONOFFC, CID Registers */
#define MAX77620_REG_CNFGGLBL1          0x00
#define MAX77620_REG_CNFGGLBL2          0x01
#define MAX77620_REG_CNFGGLBL3          0x02
#define MAX77620_REG_CNFG1_32K          0x03
#define MAX77620_REG_CNFGBBC            0x04
#define MAX77620_REG_IRQTOP         0x05
#define MAX77620_REG_INTLBT         0x06
#define MAX77620_REG_IRQSD          0x07
#define MAX77620_REG_IRQ_LVL2_L0_7      0x08
#define MAX77620_REG_IRQ_LVL2_L8        0x09
#define MAX77620_REG_IRQ_LVL2_GPIO      0x0A
#define MAX77620_REG_ONOFFIRQ           0x0B
#define MAX77620_REG_NVERC          0x0C
#define MAX77620_REG_IRQTOPM            0x0D
#define MAX77620_REG_INTENLBT           0x0E
#define MAX77620_REG_IRQMASKSD          0x0F
#define MAX77620_REG_IRQ_MSK_L0_7       0x10
#define MAX77620_REG_IRQ_MSK_L8         0x11
#define MAX77620_REG_ONOFFIRQM          0x12
#define MAX77620_REG_STATLBT            0x13
#define MAX77620_REG_STATSD         0x14
#define MAX77620_REG_ONOFFSTAT          0x15

/* SD and LDO Registers */
#define MAX77620_REG_SD0            0x16
#define MAX77620_REG_SD1            0x17
#define MAX77620_REG_SD2            0x18
#define MAX77620_REG_SD3            0x19
#define MAX77620_REG_SD4            0x1A
#define MAX77620_REG_DVSSD0         0x1B
#define MAX77620_REG_DVSSD1         0x1C
#define MAX77620_REG_SD0_CFG            0x1D
#define MAX77620_REG_SD1_CFG            0x1E
#define MAX77620_REG_SD2_CFG            0x1F
#define MAX77620_REG_SD3_CFG            0x20
#define MAX77620_REG_SD4_CFG            0x21
#define MAX77620_REG_SD_CFG2            0x22
#define MAX77620_REG_LDO0_CFG           0x23
#define MAX77620_REG_LDO0_CFG2          0x24
#define MAX77620_REG_LDO1_CFG           0x25
#define MAX77620_REG_LDO1_CFG2          0x26
#define MAX77620_REG_LDO2_CFG           0x27
#define MAX77620_REG_LDO2_CFG2          0x28
#define MAX77620_REG_LDO3_CFG           0x29
#define MAX77620_REG_LDO3_CFG2          0x2A
#define MAX77620_REG_LDO4_CFG           0x2B
#define MAX77620_REG_LDO4_CFG2          0x2C
#define MAX77620_REG_LDO5_CFG           0x2D
#define MAX77620_REG_LDO5_CFG2          0x2E
#define MAX77620_REG_LDO6_CFG           0x2F
#define MAX77620_REG_LDO6_CFG2          0x30
#define MAX77620_REG_LDO7_CFG           0x31
#define MAX77620_REG_LDO7_CFG2          0x32
#define MAX77620_REG_LDO8_CFG           0x33
#define MAX77620_REG_LDO8_CFG2          0x34
#define MAX77620_REG_LDO_CFG3           0x35

#define MAX77620_LDO_SLEW_RATE_MASK     0x1

/* LDO Configuration 3 */
#define MAX77620_TRACK4_MASK            (1 << 5)
#define MAX77620_TRACK4_SHIFT           5

/* Voltage */
#define MAX77620_SDX_VOLT_MASK          0xFF
#define MAX77620_SD0_VOLT_MASK          0x3F
#define MAX77620_SD1_VOLT_MASK          0x7F
#define MAX77620_LDO_VOLT_MASK          0x3F

#define MAX77620_REG_GPIO0          0x36
#define MAX77620_REG_GPIO1          0x37
#define MAX77620_REG_GPIO2          0x38
#define MAX77620_REG_GPIO3          0x39
#define MAX77620_REG_GPIO4          0x3A
#define MAX77620_REG_GPIO5          0x3B
#define MAX77620_REG_GPIO6          0x3C
#define MAX77620_REG_GPIO7          0x3D
#define MAX77620_REG_PUE_GPIO           0x3E
#define MAX77620_REG_PDE_GPIO           0x3F
#define MAX77620_REG_AME_GPIO           0x40
#define MAX77620_REG_ONOFFCNFG1         0x41
#define MAX77620_REG_ONOFFCNFG2         0x42

/* FPS Registers */
#define MAX77620_REG_FPS_CFG0           0x43
#define MAX77620_REG_FPS_CFG1           0x44
#define MAX77620_REG_FPS_CFG2           0x45
#define MAX77620_REG_FPS_LDO0           0x46
#define MAX77620_REG_FPS_LDO1           0x47
#define MAX77620_REG_FPS_LDO2           0x48
#define MAX77620_REG_FPS_LDO3           0x49
#define MAX77620_REG_FPS_LDO4           0x4A
#define MAX77620_REG_FPS_LDO5           0x4B
#define MAX77620_REG_FPS_LDO6           0x4C
#define MAX77620_REG_FPS_LDO7           0x4D
#define MAX77620_REG_FPS_LDO8           0x4E
#define MAX77620_REG_FPS_SD0            0x4F
#define MAX77620_REG_FPS_SD1            0x50
#define MAX77620_REG_FPS_SD2            0x51
#define MAX77620_REG_FPS_SD3            0x52
#define MAX77620_REG_FPS_SD4            0x53
#define MAX77620_REG_FPS_NONE           0

#define MAX77620_FPS_SRC_MASK           0xC0
#define MAX77620_FPS_SRC_SHIFT          6
#define MAX77620_FPS_PU_PERIOD_MASK     0x38
#define MAX77620_FPS_PU_PERIOD_SHIFT        3
#define MAX77620_FPS_PD_PERIOD_MASK     0x07
#define MAX77620_FPS_PD_PERIOD_SHIFT        0
#define MAX77620_FPS_TIME_PERIOD_MASK       0x38
#define MAX77620_FPS_TIME_PERIOD_SHIFT      3
#define MAX77620_FPS_EN_SRC_MASK        0x06
#define MAX77620_FPS_EN_SRC_SHIFT       1
#define MAX77620_FPS_ENFPS_SW_MASK      0x01
#define MAX77620_FPS_ENFPS_SW           0x01

/* Minimum and maximum FPS period time (in microseconds) are
 * different for MAX77620 and Max20024.
 */
#define MAX77620_FPS_PERIOD_MIN_US      40
#define MAX20024_FPS_PERIOD_MIN_US      20

#define MAX77620_FPS_PERIOD_MAX_US      2560
#define MAX20024_FPS_PERIOD_MAX_US      5120

#define MAX77620_REG_FPS_GPIO1          0x54
#define MAX77620_REG_FPS_GPIO2          0x55
#define MAX77620_REG_FPS_GPIO3          0x56
#define MAX77620_REG_FPS_RSO            0x57
#define MAX77620_REG_CID0           0x58
#define MAX77620_REG_CID1           0x59
#define MAX77620_REG_CID2           0x5A
#define MAX77620_REG_CID3           0x5B
#define MAX77620_REG_CID4           0x5C
#define MAX77620_REG_CID5           0x5D

#define MAX77620_REG_DVSSD4         0x5E
#define MAX20024_REG_MAX_ADD            0x70

#define MAX77620_CID_DIDM_MASK          0xF0
#define MAX77620_CID_DIDM_SHIFT         4

/* CNCG2SD */
#define MAX77620_SD_CNF2_ROVS_EN_SD1        (1 << 1)
#define MAX77620_SD_CNF2_ROVS_EN_SD0        (1 << 2)

/* Device Identification Metal */
#define MAX77620_CID5_DIDM(n)           (((n) >> 4) & 0xF)
/* Device Indentification OTP */
#define MAX77620_CID5_DIDO(n)           ((n) & 0xF)

/* SD CNFG1 */
#define MAX77620_SD_SR_MASK         0xC0
#define MAX77620_SD_SR_SHIFT            6
#define MAX77620_SD_POWER_MODE_MASK     0x30
#define MAX77620_SD_POWER_MODE_SHIFT        4
#define MAX77620_SD_CFG1_ADE_MASK       (1 << 3)
#define MAX77620_SD_CFG1_ADE_DISABLE        0
#define MAX77620_SD_CFG1_ADE_ENABLE     (1 << 3)
#define MAX77620_SD_FPWM_MASK           0x04
#define MAX77620_SD_FPWM_SHIFT          2
#define MAX77620_SD_FSRADE_MASK         0x01
#define MAX77620_SD_FSRADE_SHIFT        0
#define MAX77620_SD_CFG1_FPWM_SD_MASK       (1 << 2)
#define MAX77620_SD_CFG1_FPWM_SD_SKIP       0
#define MAX77620_SD_CFG1_FPWM_SD_FPWM       (1 << 2)
#define MAX20024_SD_CFG1_MPOK_MASK      (1 << 1)
#define MAX77620_SD_CFG1_FSRADE_SD_MASK     (1 << 0)
#define MAX77620_SD_CFG1_FSRADE_SD_DISABLE  0
#define MAX77620_SD_CFG1_FSRADE_SD_ENABLE   (1 << 0)

/* LDO_CNFG2 */
#define MAX77620_LDO_POWER_MODE_MASK        0xC0
#define MAX77620_LDO_POWER_MODE_SHIFT       6
#define MAX20024_LDO_CFG2_MPOK_MASK     (1 << 2)
#define MAX77620_LDO_CFG2_ADE_MASK      (1 << 1)
#define MAX77620_LDO_CFG2_ADE_DISABLE       0
#define MAX77620_LDO_CFG2_ADE_ENABLE        (1 << 1)
#define MAX77620_LDO_CFG2_SS_MASK       (1 << 0)
#define MAX77620_LDO_CFG2_SS_FAST       (1 << 0)
#define MAX77620_LDO_CFG2_SS_SLOW       0

#define MAX77620_IRQ_TOP_GLBL_MASK      (1 << 7)
#define MAX77620_IRQ_TOP_SD_MASK        (1 << 6)
#define MAX77620_IRQ_TOP_LDO_MASK       (1 << 5)
#define MAX77620_IRQ_TOP_GPIO_MASK      (1 << 4)
#define MAX77620_IRQ_TOP_RTC_MASK       (1 << 3)
#define MAX77620_IRQ_TOP_32K_MASK       (1 << 2)
#define MAX77620_IRQ_TOP_ONOFF_MASK     (1 << 1)

#define MAX77620_IRQ_LBM_MASK           (1 << 3)
#define MAX77620_IRQ_TJALRM1_MASK       (1 << 2)
#define MAX77620_IRQ_TJALRM2_MASK       (1 << 1)

#define MAX77620_CNFG_GPIO_DRV_MASK     (1 << 0)
#define MAX77620_CNFG_GPIO_DRV_PUSHPULL     (1 << 0)
#define MAX77620_CNFG_GPIO_DRV_OPENDRAIN    0
#define MAX77620_CNFG_GPIO_DIR_MASK     (1 << 1)
#define MAX77620_CNFG_GPIO_DIR_INPUT        (1 << 1)
#define MAX77620_CNFG_GPIO_DIR_OUTPUT       0
#define MAX77620_CNFG_GPIO_INPUT_VAL_MASK   (1 << 2)
#define MAX77620_CNFG_GPIO_OUTPUT_VAL_MASK  (1 << 3)
#define MAX77620_CNFG_GPIO_OUTPUT_VAL_HIGH  (1 << 3)
#define MAX77620_CNFG_GPIO_OUTPUT_VAL_LOW   0
#define MAX77620_CNFG_GPIO_INT_MASK     (0x3 << 4)
#define MAX77620_CNFG_GPIO_INT_FALLING      (1 << 4)
#define MAX77620_CNFG_GPIO_INT_RISING       (1 << 5)
#define MAX77620_CNFG_GPIO_DBNC_MASK        (0x3 << 6)
#define MAX77620_CNFG_GPIO_DBNC_None        (0x0 << 6)
#define MAX77620_CNFG_GPIO_DBNC_8ms     (0x1 << 6)
#define MAX77620_CNFG_GPIO_DBNC_16ms        (0x2 << 6)
#define MAX77620_CNFG_GPIO_DBNC_32ms        (0x3 << 6)

#define MAX77620_IRQ_LVL2_GPIO_EDGE0        (1 << 0)
#define MAX77620_IRQ_LVL2_GPIO_EDGE1        (1 << 1)
#define MAX77620_IRQ_LVL2_GPIO_EDGE2        (1 << 2)
#define MAX77620_IRQ_LVL2_GPIO_EDGE3        (1 << 3)
#define MAX77620_IRQ_LVL2_GPIO_EDGE4        (1 << 4)
#define MAX77620_IRQ_LVL2_GPIO_EDGE5        (1 << 5)
#define MAX77620_IRQ_LVL2_GPIO_EDGE6        (1 << 6)
#define MAX77620_IRQ_LVL2_GPIO_EDGE7        (1 << 7)

#define MAX77620_CNFG1_32K_OUT0_EN      (1 << 2)

#define MAX77620_ONOFFCNFG1_SFT_RST     (1 << 7)
#define MAX77620_ONOFFCNFG1_MRT_MASK        0x38
#define MAX77620_ONOFFCNFG1_MRT_SHIFT       0x3
#define MAX77620_ONOFFCNFG1_SLPEN       (1 << 2)
#define MAX77620_ONOFFCNFG1_PWR_OFF     (1 << 1)
#define MAX20024_ONOFFCNFG1_CLRSE       0x18

#define MAX77620_ONOFFCNFG2_SFT_RST_WK      (1 << 7)
#define MAX77620_ONOFFCNFG2_WD_RST_WK       (1 << 6)
#define MAX77620_ONOFFCNFG2_SLP_LPM_MSK     (1 << 5)
#define MAX77620_ONOFFCNFG2_WK_ALARM1       (1 << 2)
#define MAX77620_ONOFFCNFG2_WK_EN0      (1 << 0)

#define MAX77620_GLBLM_MASK         (1 << 0)

#define MAX77620_WDTC_MASK          0x3
#define MAX77620_WDTOFFC            (1 << 4)
#define MAX77620_WDTSLPC            (1 << 3)
#define MAX77620_WDTEN              (1 << 2)

#define MAX77620_TWD_MASK           0x3
#define MAX77620_TWD_2s             0x0
#define MAX77620_TWD_16s            0x1
#define MAX77620_TWD_64s            0x2
#define MAX77620_TWD_128s           0x3

#define MAX77620_CNFGGLBL1_LBDAC_EN     (1 << 7)
#define MAX77620_CNFGGLBL1_MPPLD        (1 << 6)
#define MAX77620_CNFGGLBL1_LBHYST       ((1 << 5) | (1 << 4))
#define MAX77620_CNFGGLBL1_LBHYST_N     (1 << 4)
#define MAX77620_CNFGGLBL1_LBDAC        0x0E
#define MAX77620_CNFGGLBL1_LBDAC_N      (1 << 1)
#define MAX77620_CNFGGLBL1_LBRSTEN      (1 << 0)

/* CNFG BBC registers */
#define MAX77620_CNFGBBC_ENABLE         (1 << 0)
#define MAX77620_CNFGBBC_CURRENT_MASK       0x06
#define MAX77620_CNFGBBC_CURRENT_SHIFT      1
#define MAX77620_CNFGBBC_VOLTAGE_MASK       0x18
#define MAX77620_CNFGBBC_VOLTAGE_SHIFT      3
#define MAX77620_CNFGBBC_LOW_CURRENT_DISABLE    (1 << 5)
#define MAX77620_CNFGBBC_RESISTOR_MASK      0xC0
#define MAX77620_CNFGBBC_RESISTOR_SHIFT     6

#define MAX77620_FPS_COUNT          3

/* Interrupts */
enum {
    MAX77620_IRQ_TOP_GLBL,      /* Low-Battery */
    MAX77620_IRQ_TOP_SD,        /* SD power fail */
    MAX77620_IRQ_TOP_LDO,       /* LDO power fail */
    MAX77620_IRQ_TOP_GPIO,      /* TOP GPIO internal int to MAX77620 */
    MAX77620_IRQ_TOP_RTC,       /* RTC */
    MAX77620_IRQ_TOP_32K,       /* 32kHz oscillator */
    MAX77620_IRQ_TOP_ONOFF,     /* ON/OFF oscillator */
    MAX77620_IRQ_LBT_MBATLOW,   /* Thermal alarm status, > 120C */
    MAX77620_IRQ_LBT_TJALRM1,   /* Thermal alarm status, > 120C */
    MAX77620_IRQ_LBT_TJALRM2,   /* Thermal alarm status, > 140C */
};

/* GPIOs */
enum {
    MAX77620_GPIO0,
    MAX77620_GPIO1,
    MAX77620_GPIO2,
    MAX77620_GPIO3,
    MAX77620_GPIO4,
    MAX77620_GPIO5,
    MAX77620_GPIO6,
    MAX77620_GPIO7,
    MAX77620_GPIO_NR,
};

/* FPS Source */
enum max77620_fps_src {
    MAX77620_FPS_SRC_0,
    MAX77620_FPS_SRC_1,
    MAX77620_FPS_SRC_2,
    MAX77620_FPS_SRC_NONE,
    MAX77620_FPS_SRC_DEF,
};

enum max77620_chip_id {
    MAX77620,
    MAX20024,
};

#endif /* _MFD_MAX77620_H_ */
