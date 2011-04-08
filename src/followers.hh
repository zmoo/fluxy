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

#ifndef _FOLLOWERS_HH
#define _FOLLOWERS_HH

#include "buffer.hh"

template <typename ItemType> 
struct FluxyVector : public FluxyBuffer {
private:
	ItemType *item_ptr(int n);

public:
	int count();
	void add(ItemType item);
	int find(ItemType item);
	bool del(int n);
	void debug();
};

#define FLUXY_VECTOR_FOREACH(type, it) for (type it = (type) ptr_begin(); it < (type) ptr_end(); it++)

#endif
