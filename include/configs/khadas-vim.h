/*
 * Configuration for Khadas VIM
 *
 * Copyright (C) 2017 Baylibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MISC_INIT_R

#define CONFIG_PHY_ADDR		8

#define MESON_FDTFILE_SETTING "fdtfile=amlogic/meson-gxl-s905x-khadas-vim.dtb\0"

#include <configs/meson-gxbb-common.h>

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#define CONFIG_BOOTCOMMAND "if fatload mmc 1 1020000 s905_autoscript; then source 1020000; fi;"

#endif /* __CONFIG_H */
