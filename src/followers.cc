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

#include "followers.hh"

#include <iostream>

template <typename ItemType> 
ItemType *FluxyVector<ItemType>::item_ptr(int n) {
	if (n < 0) return NULL;
	return (ItemType *) ptr_offset(n * sizeof(ItemType));
}

template <typename ItemType> 
int FluxyVector<ItemType>::count() {
	return get_size() / sizeof(ItemType);
}

template <typename ItemType> 
void FluxyVector<ItemType>::add(ItemType item) {
	size_t offset = get_size();
	set_size(offset + sizeof(ItemType));
	ItemType *ptr = (ItemType *) ptr_offset(offset);
	*ptr = item;
}

template <typename ItemType> 
int FluxyVector<ItemType>::find(ItemType item) {
	int n = 0;
	FLUXY_VECTOR_FOREACH(ItemType *, it) {
		if (*it == item)
			return n;
		n++;
	}
	return -1;
}

template <typename ItemType> 
bool FluxyVector<ItemType>::del(int n) {
	int items = count();
	if (items < 1)
		return false;

	ItemType *last = item_ptr(items - 1);
	ItemType *item = item_ptr(n);
	if (item != last) {
		*item = *last;
	}

	set_size((items - 1) * sizeof(ItemType));
	return true;
}

template <typename ItemType> 
void FluxyVector<ItemType>::debug() {
	std::cout << "=== Vector ===" << std::endl;
	FLUXY_VECTOR_FOREACH(ItemType *, it) {
		std::cout << *it << std::endl;
	}
}

template class FluxyVector<int>;

