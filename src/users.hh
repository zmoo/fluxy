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

#ifndef _USERS_HH
#define _USERS_HH

#include <list>
#include <vector>

#include "ghash++.hh"
#include "user.hh"
#include "fetcher.hh"
#include "words_parser.hh"
#include "result.hh"

class VectorUsers {
private:
	PMutex mutex;
public:
	typedef std::vector<User*> List;
	List list;

	void clear();
	void dump(FILE *f);
	void report(std::stringstream &out);
	void top(std::stringstream &out, int const size = 64);
	void cleanup();

	void lock();
	bool trylock();
	void unlock();
};

class Users {
private:
	typedef HashTable<User> HashTableUsers;
	HashTableUsers hash_table;
	PMutex mutex;

	VectorUsers vector;
	VectorUsers vector_new_users;

	void vector_update();
	void user_add(User *user);
	User *lookup(UserId const id);


	struct FetchItem {
		UserId id;
		UserMessageFlags mask;
		UserMessageFlags value;
	};
	typedef std::list<FetchItem> FetchItems;

public:
	unsigned int count();
	bool dump(std::string const target);
	void cleanup();
	void report(std::stringstream &out);
	void top(std::stringstream &out, int const size = 64);
	bool restore(FILE *f);
	bool restore(std::string const filename);

	User *user_find(UserId const id);
	User *user_find_or_create(UserId const id);
	void send(std::list<UserId> &list, UserMessageValues const values);
	void fetch(FetchItems &list, Fetcher &fetcher, time_t const since = 0);
	bool parse_file(std::string const filename);
	void parse_buf(char *data, size_t size);
	bool parse_query(ClientResult &result, WordsParser *parser, OutputType const type);
};

#ifdef _USERS_CC
Users users;
#else
extern Users users;
#endif

#endif

