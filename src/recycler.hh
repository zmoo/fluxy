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

#ifndef _RECYCLER_HH
#define _RECYCLER_HH

#include <vector>
#include <sstream>

#include "scale.hh"
#include "pthread++.hh"

#define SCALES_COUNT (FLUXY_SCALE_MAX - FLUXY_SCALE_MIN + 1)


class Recycler {
private:
	typedef std::vector<char *> List;
	List availables[SCALES_COUNT];
	PMutex mutex;

public:
	char *alloc(FluxyScale const scale);
	void free(char *buffer, FluxyScale const scale);
	char *realloc(char *buffer, FluxyScale const old_scale, FluxyScale const new_scale);
	void stats(std::stringstream &out);

	void lock();
	void unlock();
};

#ifdef _RECYCLER_CC
Recycler recycler;
#else
extern Recycler recycler;
#endif

#endif
