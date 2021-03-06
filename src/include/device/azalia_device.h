/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 DMP Electronics Inc.
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

#ifndef DEVICE_AZALIA_H
#define DEVICE_AZALIA_H

#include <device/device.h>

void azalia_audio_init(struct device *dev);
extern struct device_operations default_azalia_audio_ops;

extern const u32 *cim_verb_data;
extern u32 cim_verb_data_size;
extern const u32 *pc_beep_verbs;
extern u32 pc_beep_verbs_size;

#endif /* DEVICE_AZALIA_H */
