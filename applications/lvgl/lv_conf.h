/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-18     Meco Man      First version
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_USE_PERF_MONITOR     1
#define LV_COLOR_DEPTH          16
#define LV_HOR_RES_MAX          800
#define LV_VER_RES_MAX          480

#define LV_USE_GPU_STM32_DMA2D  1
#define LV_GPU_DMA2D_CMSIS_INCLUDE "stm32f469xx.h"

#define LV_FONT_MONTSERRAT_10              8
#define LV_FONT_MONTSERRAT_12              8
#define LV_FONT_MONTSERRAT_14              8
#define LV_FONT_MONTSERRAT_16              8
#define LV_FONT_MONTSERRAT_20              8
#define LV_FONT_MONTSERRAT_24              8
#define LV_FONT_MONTSERRAT_32              8
#define LV_FONT_MONTSERRAT_36              8
#define LV_FONT_MONTSERRAT_40              8
#define LV_FONT_MONTSERRAT_48              8
#define CODE_NEW_ROMAN_BOLD_DIGIT_80       8

#define LV_FONT_CUSTOM_DECLARE  LV_FONT_DECLARE(code_new_roman_bold_digit_80)\
                                LV_FONT_DECLARE(code_new_roman_bold_digit_160)

#ifdef PKG_USING_LV_MUSIC_DEMO
/* music player demo */
#define LV_USE_DEMO_RTT_MUSIC       1
#define LV_DEMO_RTT_MUSIC_AUTO_PLAY 1
#define LV_FONT_MONTSERRAT_12       1
#define LV_FONT_MONTSERRAT_16       1
#define LV_COLOR_SCREEN_TRANSP      1
#endif /* PKG_USING_LV_MUSIC_DEMO */

#endif
