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

#define _ADDSTATS_CC

#include "addstats.hh"

void Addstats::inc(uint8_t const i) {
	if (i < COUNTER_SIZE) {
		mutex.lock();
		counter[i]++;
		mutex.unlock();
	}
}

void Addstats::serialize_php(std::stringstream &out) {
	mutex.lock();

	int size = 0;
	bool show[COUNTER_SIZE];
	for (int i = 0; i < COUNTER_SIZE; i++) {
		show[i] = (counter[i] != 0);
		if (show[i])
			size++;
	}

	out << "a:" << size << ":{";
	for (int i = 0; i < COUNTER_SIZE; i++) {
		if (show[i])
			out << "i:" << i << ";i:" << counter[i] << ";";
	}
	out << "}";

	mutex.unlock();
}

Addstats::Addstats() {
	for (int i = 0; i < COUNTER_SIZE; i++) {
		counter[i] = 0;
	}
}
