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

#define _RECYCLER_CC

#include "recycler.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>


char *Recycler::alloc(FluxyScale const scale) {
	//std::cout << "alloc(" << (int) scale << ")" << std::endl;
	int idx = scale - FLUXY_SCALE_MIN;
	if (availables[idx].size() != 0) {
		char* result = availables[idx].back();
		availables[idx].pop_back();
		return result;
	}
	else {
		return (char *) malloc(FLUXY_SCALE_SIZE(scale));
	}
}

void Recycler::free(char *buffer, FluxyScale const scale) {
	//std::cout << "free(" << (int) scale << ")" << std::endl;
	availables[scale - FLUXY_SCALE_MIN].push_back(buffer);
}

char *Recycler::realloc(char *buffer, FluxyScale const old_scale, FluxyScale const new_scale) {
	if (old_scale == new_scale) {
		return buffer;
	}

	char *new_buffer = alloc(new_scale);
	memcpy(new_buffer, buffer, FLUXY_SCALE_SIZE(std::min(old_scale, new_scale)));
	free(buffer, old_scale);
	return new_buffer;
}

void Recycler::stats(std::stringstream &out) {
	out << "a:" << (FLUXY_SCALE_MAX - FLUXY_SCALE_MIN + 1) << ":{";
	for (int i = FLUXY_SCALE_MIN; i <= FLUXY_SCALE_MAX; i++) {
		out << "i:" << FLUXY_SCALE_SIZE(i) << ";i:" << availables[i - FLUXY_SCALE_MIN].size() << ";";
	}
	out << "}";
}

void Recycler::lock() {
	mutex.lock();
}

void Recycler::unlock() {
	mutex.unlock();
}
