// -*- coding:unix -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkx.h>

//--------------------------------------------------------
// Contributed code from Pclouds (Nguyen Thai Ngoc Duy)
// for synchronizing with xvnkb GUI
// (these definitions are copied from xvnkb by pclouds)
//
// Many thanks to pclouds and Dao Hai Lam (xvnkb author)
//--------------------------------------------------------

#ifndef __XVNKB_H
#define __XVNKB_H

#define VK_SHIFT		0x01
#define	VK_CAPS_LOCK		0x02
#define VK_CONTROL		0x04
#define VK_ALT			0x08
#define VK_WINKEY		0x40
#define	VK_NUM_LOCK		0x10
#define	VK_SCROLL_LOCK		0x80
#define	VK_NOTACCEPT_STATES	0x6C
#define VK_UPPERCASE_STATES	(VK_SHIFT|VK_CAPS_LOCK)

// when ukxim is run in xvnkb sync mode, it will
// use these atoms for synchronization
#define VKP_CHARSET	"VK_CHARSET"
#define VKP_METHOD	"VK_METHOD"
#define VKP_USING	"VK_USING"
#define VKP_SPELLING	"VK_SPELLING"
#define VKP_HOTKEY	"VK_HOTKEY"


// If we want to run both unikey and xvnkb core at
// the same time, we must use a different set of 
// atoms to avoid conflicts. (that also means no synchronization)
#define UKP_CHARSET	"UK_CHARSET"
#define UKP_METHOD	"UK_METHOD"
#define UKP_USING	"UK_USING"
#define UKP_SPELLING	"UK_SPELLING"
#define UKP_HOTKEY	"UK_HOTKEY"


typedef enum {
  VKM_OFF,
  VKM_VNI,
  VKM_TELEX,
  VKM_VIQR,
  VKM_VIQR_STAR
} vk_methods;

typedef enum {
  VKC_TCVN,
  VKC_VNI,
  VKC_VIQR,
  VKC_VISCII,
  VKC_VPS,
  VKC_UTF8
} vk_charsets;

typedef struct {
  long state;
  long sym;
} vk_hotkey_info;

//pklong: added atoms for stored preferd GUI posistion
#define UKP_GUI_POS_X "UK_GUI_X_POSITION"
#define UKP_GUI_POS_Y "UK_GUI_Y_POSITION"

//suspendNotify atom
#define UKP_SUSPEND "UK_SUSPEND_MODE"

struct _Xvnkb {
  vk_hotkey_info hotkey;
  Atom AIMCharset;
  Atom AIMUsing;
  Atom AIMMethod;
  Atom AIMSwitchKey;
  Atom AIMSpelling;
};

#endif
