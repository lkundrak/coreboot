/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2008 coresystems GmbH
 *               2012 secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef NORTHBRIDGE_INTEL_I965_CHIP_H
#define NORTHBRIDGE_INTEL_I965_CHIP_H

struct northbridge_intel_i965_config {
	int gpu_use_spread_spectrum_clock;
	int gpu_lvds_dual_channel;
	int gpu_link_frequency_270_mhz;
	int gpu_lvds_num_lanes;
};

#endif				/* NORTHBRIDGE_INTEL_I965_CHIP_H */
