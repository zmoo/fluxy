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

#include "top.hh"

#include "users.hh"

bool compare_items(TopItem first, TopItem second) {
	return (first.score > second.score);
}

void TopBase::add(UserId user, UserScore score) {
	list.push_back((TopItem) {user, score});
}

void TopBase::serialize_php(std::stringstream &out, int unsigned const size, int const users_count, int unsigned const from) {
	unsigned int final_size = (from < list.size()) ? MIN(size, list.size() - from) : 0;

	out << "a:2:{";
	out << "s:11:\"users_count\";i:" << users_count << ";";
	out << "s:4:\"list\";a:" << final_size << ":{";

	if (final_size != 0) {
		List::iterator it = list.begin();
		for (unsigned int i = 0; i < from; i++) {
			it++;
		}

		for (unsigned int i = 0; i < final_size; i++) {
			out << "i:" << (i + from) << ";";
			out << "a:2:{";
			out << "s:5:\"score\";i:" << it->score << ";";
			out << "s:4:\"user\";i:" << it->user << ";";
			out << "}";
			it++;
		}
	}
	out << "}}";
}

void TopBase::clear() {
	list.clear();
}

int TopBase::count() {
	return list.size();
}

void Top::normalize() {
	list.sort(compare_items);
	
	if (list.size() > size)
		list.resize(size);

	worse_score_def = true;
	worse_score = list.back().score;
}

void Top::add(UserId const user, UserScore const score) {
	users_count++;

	if (!worse_score_def or score > worse_score) {
		TopBase::add(user, score);
		if (list.size() > size * 2 + 16)
			normalize();
	}
}

void Top::finalize() {
	normalize();
}

void Top::serialize_php(std::stringstream &out) {
	TopBase::serialize_php(out, size, users_count);
}

Top::Top(int const _size) {
	size = _size;
	users_count = 0;
	worse_score_def = false;
}

