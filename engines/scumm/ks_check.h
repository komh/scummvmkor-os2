/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM Kor. Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header: /cvsroot/scummkor/scummvm-kor/scumm/ks_check.h,v 1.4 2004/08/18 09:51:55 wonst719 Exp $
 */

#ifndef KS_CHECK_H
#define KS_CHECK_H

#include "common/scummsys.h"

namespace Scumm {

int checkHangul(byte hi, byte lo);
byte checkJongsung(byte hi, byte lo);

} // End of namespace Scumm

#endif
