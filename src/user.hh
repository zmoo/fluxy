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

#ifndef _USER_HH
#define _USER_HH

#include <cstdlib>
#include <stdint.h>
#include <sstream>
#include <string>

#include "fluxy.hh"
#include "pthread++.hh"
#include "result.hh"
#include "words_parser.hh"
#include "messages.hh"

struct UserMessageSearch {
	FluxyMessageId id;
	bool check_type;
	FluxyMessageType type;

public:
	void parse(WordsParser *parser);
	void output(std::stringstream &replication_query);
};

struct UserMessageFilter {
	bool id;
	bool date;
	bool type;
	bool flags;
	bool version;
	bool seen;
	bool body;

public:
	UserMessageFilter();
};

struct UserMessageValues {
	FluxyMessageId id;
	time_t date;
	FluxyMessageType type;
	FluxyMessageFlags flags;
	FluxyMessageVersion version;
	bool seen;
	std::string body;
public:
	void copy_head(FluxyMessageHead &head) const;
	void set(FluxyMessage *message);
	bool parse_query(WordsParser *parser, bool &autoid);
	void replicate_query(std::stringstream &output, bool autoid);

	UserMessageValues(FluxyMessage const *message);
	UserMessageValues();
};

class Fetcher;

struct UserMessages : private FluxyMessages {
	friend class UsersReport;
public:
	void debug();

	FluxyMessage *find(UserMessageSearch const search) const;
	bool add(UserMessageValues const values);
	bool del(UserMessageSearch const search);
	bool update(UserMessageSearch const search, UserMessageFilter const filter, UserMessageValues const values);
	bool seen(UserMessageSearch const last, bool const value);
	bool seen_all(bool const value);
	int count();
	int count_unseen();
	void serialize_php(std::stringstream &out);
	void cleanup();
	void clear();
	void fetch(UserId const user_id, Fetcher &fetcher, UserMessageFlags const mask = 0, UserMessageFlags const value = 0, time_t const after = 0, time_t const before = 0);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);

	void dump(FILE *f);
	bool restore(FILE *f);

	UserScore score();
};

struct UserData : public FluxyBuffer {
public:
	void serialize_php(std::stringstream &out);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query);
};

struct User {
public:
	UserId id;
	UserFlags flags;
//	UserData data;
	UserMessages messages;

private:
	bool _deleted;

public:
	PMutex *lock();
	void serialize_php(std::stringstream &out);
	void show(std::stringstream &out);
	bool parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser);
	void cleanup();
	void clear();
	void del();
	void undel();
	bool deleted();
	void dump(FILE *f);
	UserScore score();

	User(UserId const id);
	User(FILE *f);
};

#define MEMSTATS_COUNT (FLUXY_SCALE_MAX - FLUXY_SCALE_MIN + 1)
struct UsersReport {
private:
	int count;
	int memstats[MEMSTATS_COUNT];
	int empty;
	int deleted_count;

public:
	void add(User &user);
	void serialize_php(std::stringstream &out);
	UsersReport();
};

struct UserLocks {
	PMutex lock[1024];
};

#ifdef _USER_CC
UserLocks user_locks;
time_t user_messages_expiration;
int user_messages_size;
#else
extern UserLocks user_locks;
extern time_t user_messages_expiration;
extern int user_messages_size;
#endif


#endif
