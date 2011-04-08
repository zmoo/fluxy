/*
 *  Copyright (C) 2009 Nicolas Vion <nico@picapo.net>
 *
 *   This file is part of Fluxy.
 *
 *   Fluxy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Fluxy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Fluxy; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _ADDSTATS_HH
#define _ADDSTATS_HH

#include <sstream>
#include <stdint.h>

#include "pthread++.hh"

#define COUNTER_SIZE 256

class Addstats {
private:
	unsigned int counter[COUNTER_SIZE];
	PMutex mutex;

public:
	void inc(uint8_t const i);

	void serialize_php(std::stringstream &out);
	Addstats();
};

#ifdef _ADDSTATS_CC
Addstats addstats;
#else
extern Addstats addstats;
#endif

#endif

