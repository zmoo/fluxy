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

#include "fetcher.hh"

void FetcherItem::serialize_php(std::stringstream &out) {
	out << "a:8:{";
	out << "s:4:\"user\";i:" << user_id << ";";
	out << "s:2:\"id\";i:" << values.id << ";";
	out << "s:4:\"date\";i:" << values.date << ";";
	out << "s:4:\"type\";i:" << (int) values.type << ";";
	out << "s:5:\"flags\";i:" << values.flags << ";";
	out << "s:7:\"version\";i:" << (int) values.version << ";";
	out << "s:4:\"seen\";b:" << (values.seen ? "1" : "0") << ";";
	out << "s:4:\"body\";s:" << values.body.size() << ":\"" << values.body << "\";";
	out << "}";
}

void Fetcher::clear() {
	list.clear();
}

void Fetcher::serialize_php(std::stringstream &out) {
	int count = list.size();
	out << "a:" << count << ":{";
	int i = 0;
	for (List::iterator it = list.begin(); it != list.end(); it++) {
		out << "i:" << i++ << ";";
		it->serialize_php(out);
	}
	out << "}";
}

bool compare_items(FetcherItem first, FetcherItem second) {
	return (first.values.date > second.values.date);
}

void Fetcher::normalize() {
	list.sort(compare_items);
	if (size > 0 and list.size() > size)
		list.resize(size);

	oldest = list.back().values.date;
}

void Fetcher::add(UserId const user_id, FluxyMessage const *message) {
	if (oldest == 0 or message->head.get_date() > oldest) {
		list.push_back((FetcherItem) {user_id, UserMessageValues(message)});
		if (list.size() > size * 2 + 16)
			normalize();
	}
}

void Fetcher::finalize() {
	normalize();
}

Fetcher::Fetcher(int const _size) : size(_size), oldest(0) {
}
