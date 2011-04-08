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

#ifndef _TOP_HH
#define _TOP_HH

#include <list>
#include <string>
#include <sstream>

#include "fluxy.hh"
#include "user.hh"

typedef struct {
	UserId user;
	UserScore score;
} TopItem;

class TopBase {
protected:
	typedef std::list<TopItem> List;
	List list;

public:
	void add(UserId const user, UserScore score);
	void serialize_php(std::stringstream &out, int unsigned const size, int const users_count, int unsigned const from = 0);
	int count();
	void clear();
};

class Top : public TopBase {
private:
	UserScore worse_score;
	bool worse_score_def;

	unsigned int size;
	unsigned int users_count;
	void normalize();

public:
	void add(UserId const user, UserScore const score);
	void finalize();
	void serialize_php(std::stringstream &out);
	Top(int const _size);
};

#endif
