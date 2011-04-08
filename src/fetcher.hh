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

#ifndef _FETCHER_HH
#define _FETCHER_HH

#include <list>
#include <cstdlib>
#include <stdint.h>
#include <sstream>
#include <string>

#include "user.hh"
#include "messages.hh"

#include "fluxy.hh"

struct FetcherItem {
	UserId user_id;
	UserMessageValues values;

	void serialize_php(std::stringstream &out);
};

class Fetcher {
private:
	typedef std::list<FetcherItem> List;
	List list;
	unsigned int size;

private:
	void normalize();

public:
	time_t oldest;

	void clear();
	void add(UserId const user_id, FluxyMessage const *message);
	void finalize();
	void serialize_php(std::stringstream &out);

	Fetcher(int const size = 64);
};

#endif
