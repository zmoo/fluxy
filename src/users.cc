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

#define _USERS_CC

#include "users.hh"

#include <iostream>
#include <fstream>
#include <cstring>

#include "stringutils.hh"
#include "threads.hh"
#include "stats.hh"
#include "top.hh"
#include "log.hh"
#include "replicator.hh"
#include "autoid.hh"

void VectorUsers::lock() {
	mutex.lock();
}

bool VectorUsers::trylock() {
	return mutex.trylock();
}

void VectorUsers::unlock() {
	mutex.unlock();
}

void VectorUsers::clear() {
	lock();
	list.clear();
	unlock();
}

void VectorUsers::report(std::stringstream &out) {
	lock();
	UsersReport report;
	User *user;
	for (VectorUsers::List::iterator it = list.begin(); it != list.end(); it++) {
		user = *it;
		PMutex *mutex = user->lock();
		report.add(*user);
		mutex->unlock();
	}
	unlock();
	report.serialize_php(out);
}

void VectorUsers::top(std::stringstream &out, int const size) {
	lock();
	Top top(size);
	User *user;
	for (VectorUsers::List::iterator it = list.begin(); it != list.end(); it++) {
		user = *it;
		PMutex *mutex = user->lock();
		if (!user->deleted())
			top.add(user->id, user->score());
		mutex->unlock();
	}
	unlock();
	top.finalize();
	top.serialize_php(out);
}

void VectorUsers::cleanup() {
	lock();
	User *user;
	for (VectorUsers::List::iterator it = list.begin(); it != list.end(); it++) {
		user = *it;
		PMutex *mutex = user->lock();
		if (!user->deleted())
			user->cleanup();
		mutex->unlock();
	}
	unlock();
}

#define DUMP_MAGIC 4242
#define DUMP_VERSION 2

void VectorUsers::dump(FILE *f) {
	lock();

	//start pattern
	char pattern[5] = {'F', 'l', 'u', 'x', 'y'};
	fwrite(&pattern, 1, sizeof(pattern), f);

	//endianness magic number
	uint16_t magic = DUMP_MAGIC;
	fwrite(&magic, 1, sizeof(magic), f);

	//User header size
	uint8_t user_header_size = DUMP_VERSION;
	fwrite(&user_header_size, 1, sizeof(user_header_size), f);

	//global data
	auto_id.dump(f);
	
	//number of users
	unsigned int count = list.size();
	fwrite(&count, 1, sizeof(count), f);

	//users
	User *user;
	for (VectorUsers::List::iterator it = list.begin(); it != list.end(); it++) {
		user = *it;
		PMutex *mutex = user->lock();
		user->dump(f);
		mutex->unlock();
	}

	//end pattern
	fwrite(&pattern, 1, sizeof(pattern), f);

	unlock();
}

bool Users::dump(std::string const path) {
	vector.lock();
	vector_update();
	vector.unlock();

	std::string tmp = path + ".tmp";
	FILE *f = fopen(tmp.c_str(), "wb");
	if (f) {
		vector.dump(f);
		fclose(f);
		return (rename(tmp.c_str(), path.c_str()) == 0);
	}
	return false;
}

bool Users::restore(FILE *f) {
	//start pattern
	char pattern[5];
	size_t readen;

	readen = fread(&pattern, 1, sizeof(pattern), f);
	if (readen != sizeof(pattern) or pattern[0] != 'F' or pattern[1] != 'l' or pattern[2] != 'u' or pattern[3] != 'x' or pattern[4] != 'y') {
		log.msg(LOG_ERR, "Not a valid start pattern");
		return false;
	}

	//check endianness
	uint16_t magic;
	readen = fread(&magic, 1, sizeof(magic), f);
	if (readen != sizeof(magic) or magic != DUMP_MAGIC) {
		log.msg(LOG_ERR, "Endianness error");
		return false;
	}

	//format version
	uint8_t format_version = 0;
	readen = fread(&format_version, 1, sizeof(format_version), f);
	if (format_version != DUMP_VERSION) {
		log.msg(LOG_ERR, "Invalid format version");
		return false;
	}

	//global data
	if (!auto_id.restore(f)) {
		log.msg(LOG_ERR, "Could not restore AutoId object");
		return false;
	}
	
	//number of users
	unsigned int count;
	readen = fread(&count, 1, sizeof(count), f);
	if (readen != sizeof(count)) {
		log.msg(LOG_ERR, "Unexpected end of file");
		return false;
	}

	mutex.lock();
	vector.lock();
	User *user;
	for (unsigned int i = 0; i < count; i++) {
		user = new User(f);
		//TODO: check that user doesn't exist in order to avoid double definitions
		hash_table.add((int*) &user->id, user);
		vector.list.push_back(user);
	}
	vector.unlock();
	mutex.unlock();

	//end pattern
	readen = fread(&pattern, 1, sizeof(pattern), f);
	if (readen != sizeof(pattern) or pattern[0] != 'F' or pattern[1] != 'l' or pattern[2] != 'u' or pattern[3] != 'x' or pattern[4] != 'y') {
		log.msg(LOG_ERR, "Not a valid end pattern");
		return false;
	}
	return true;
}

bool Users::restore(std::string const filename) {
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		bool result = restore(f);
		fclose(f);
		return result;
	}
	return false;
}

void Users::vector_update() {
	vector_new_users.lock();
	for (VectorUsers::List::iterator it = vector_new_users.list.begin(); it != vector_new_users.list.end(); it++) {
		vector.list.push_back(*it);
	}
	vector_new_users.list.clear();
	vector_new_users.unlock();
}

void Users::user_add(User *user) {
	mutex.lock();
	hash_table.add((int*) &user->id, user);
	mutex.unlock();

	if (vector.trylock()) {
		vector_update();
		vector.list.push_back(user);
		vector.unlock();
	}
	else {
		vector_new_users.lock();
		vector_new_users.list.push_back(user);
		vector_new_users.unlock();
	}
}

User *Users::lookup(UserId const id) {
	mutex.lock();
	User *res = hash_table.lookup(id);
	mutex.unlock();
	return res;
}

unsigned int Users::count() {
	mutex.lock();
	unsigned int res = hash_table.size();
	mutex.unlock();
	return res;
}

User *Users::user_find(UserId const id) {
	mutex.lock();
	User *user = hash_table.lookup(id);
	mutex.unlock();
	if (!user or user->deleted()) {
		return NULL;
	}
	return user;
}

User *Users::user_find_or_create(UserId const id) {
	mutex.lock();
	User *user = hash_table.lookup(id);
	mutex.unlock();

	if (!user) {
		user = new User(id);
		user_add(user);
	}
	else if (user->deleted())
		user->undel();

	return user;
}

void Users::send(std::list<UserId> &list, UserMessageValues const values) {
	User *user;
	for (std::list<UserId>::iterator it = list.begin(); it != list.end(); it++) {
		user = user_find_or_create(*it);
		if (user) {
			PMutex *mutex = user->lock();
			user->messages.add(values);
			mutex->unlock();
		}
	}
}

void Users::fetch(FetchItems &list, Fetcher &fetcher, time_t const before) {
	fetcher.clear();
	User *user;
	for (FetchItems::iterator it = list.begin(); it != list.end(); it++) {
		user = user_find(it->id);
		if (user) {
			PMutex *mutex = user->lock();
			user->messages.fetch(user->id, fetcher, it->mask, it->value, fetcher.oldest, before);
			mutex->unlock();
		}
	}
	fetcher.finalize();
}

void Users::cleanup() {
	vector.cleanup();
}

void Users::report(std::stringstream &out) {
	vector.report(out);
}

void Users::top(std::stringstream &out, int const size) {
	vector.top(out, size);
}

#define BUF_SIZE 4096
bool Users::parse_file(std::string const filename) {
	ifstream is;
	is.open(filename.c_str(), ios::binary);
	if (!is.is_open())
		return false;

	char buf[BUF_SIZE];

	while (!is.eof()) {
		is.getline(buf, BUF_SIZE);
		size_t size = strlen(buf);
		if (size > 0)
			parse_buf(buf, size);
	}

	is.close();
	return true;
}

void Users::parse_buf(char *data, size_t size) {
	try {
		std::stringstream stream;
		stream.write(data, size);
		WordsParser parser(&stream);
		parser.next();

		ClientResult result(NULL);
		parse_query(result, &parser, NONE);
	}
	catch (...) {
		log.msg(LOG_ERR, "Not a valid command buffer");
	}
}

bool Users::parse_query(ClientResult &result, WordsParser *parser, OutputType const type) {
	if (parser->current == "#") {
		result.replicated = true;
		parser->next();
	}

	//!user <user_id> <command> [args]
	//!	Execute a command on a existing user
	//!	See "user <user_id> help" for more information
	//!user* <user_id> <command> [args]
	//!	Execute a command on a user.
	//!	Create a new user if <user_id> doesn't exists
	if (parser->current == "user") {
		parser->next();
		bool find_or_create = false;
		if (parser->current == "*") {
			find_or_create = true;
			parser->next();
		}

		UserId id = StringUtils::to_uint(parser->current);
		User *user = (find_or_create) ? user_find_or_create(id) : user_find(id);
		parser->next();

		if (user) {
			PMutex *mutex = user->lock();
			if (!user->parse_query(result, "user::", parser))
				result.error();
			mutex->unlock();
			result.send();
		}
		else {
			stats.inc("error::UNEXISTING_USER");
			result.error("Not a valid user id.");
			result.send();
		}
		return true;
	}

	//!send <user_id1>,<user_id2>,... <id> <date> <type> <flags> <version> <body>
	//!	Send a message to a set of users
	if (parser->current == "send") {
		stats.inc("send");
		std::list<UserId> target;
		do {
			target.push_back(StringUtils::to_uint(parser->next()));
		} while (parser->next() == ",");

		UserMessageValues values;
		bool autoid = false;
		values.parse_query(parser, autoid);

		send(target, values);
		result.send();

		//replication
		if (replicator.opened and !result.replicated) {
			std::stringstream replication_query;
			replication_query << "#send ";
			for (std::list<UserId>::iterator it = target.begin(); it != target.end(); it++) {
				if (it != target.begin())
					replication_query << ",";
				replication_query << *it;
			}
			replication_query << " ";
			values.replicate_query(replication_query, autoid);
			replicator.add(replication_query);
		}
		return true;
	}

	//!fetch <user_id1> [ [<mask> <value>] ], <user_id2>,... [before <date>] [limit <n = 128>]
	//!	Fetch messages from a set of users
	if (parser->current == "fetch") {
		stats.inc("fetch");
		FetchItems list;

		UserMessageFlags default_mask = 0, default_value = 0;
		do {
			parser->next();

			//get default filter
			if (parser->current == "[") {
				default_mask = StringUtils::to_uint(parser->next());
				default_value = StringUtils::to_uint(parser->next());
				parser->next(); //]
				parser->next(); //id
			}

			//get user id
			UserId id = StringUtils::to_uint(parser->current);

			//get user filter
			UserMessageFlags mask = default_mask, value = default_value;
			if (parser->next() == "[") {
				mask = StringUtils::to_uint(parser->next());
				value = StringUtils::to_uint(parser->next());
				parser->next(); //]
				parser->next(); //,
			}

			list.push_back((FetchItem) {id, mask, value});
		} while (parser->current == ",");

		time_t before = 0;
		if (parser->current == "before") {
			before = StringUtils::to_uint(parser->next());
			parser->next();
		}

		int limit = 64;
		if (parser->current == "limit") {
			limit = StringUtils::to_uint(parser->next());
			parser->next();
		}

		Fetcher fetcher(limit);
		fetch(list, fetcher, before);
		result.type = PHP_SERIALIZE;
		fetcher.serialize_php(result.data);
		result.send();
		return true;
	}

	//!dump <path>
	//!	Dump data in file <path>
	if (parser->current == "dump") {
		stats.inc("dump");
		std::string target = parser->next(true);
		DumpThread *thread = new DumpThread(result.get_client(), target);
		thread->run();
		return true;
	}

	//!parse
	//!	Parse queries file
	if (parser->current == "parse") {
		std::string target = parser->next(true);
		parse_file(target);
		result.send();
		return true;
	}

	//!report
	//!	Generate report
	if (parser->current == "report") {
		stats.inc("report");
		ReportThread *thread = new ReportThread(result.get_client());
		thread->run();
		return true;
	}

	//!top [limit <n = 128>]
	//!	Generate top
	if (parser->current == "top") {
		stats.inc("top");

		int limit = 128;
		if (parser->next() == "limit") {
			limit = StringUtils::to_uint(parser->next());
		}

		TopThread *thread = new TopThread(result.get_client(), limit);
		thread->run();
		return true;
	}

	//!cleanup
	//!	Delete old messages to free memory
	if (parser->current == "cleanup") {
		stats.inc("cleanup");
		CleanupThread *thread = new CleanupThread(result.get_client());
		thread->run();
		return true;
	}

	/*
	if (parser->current == "sleep") {
		time_t sec = StringUtils::to_uint(parser->next());
		sleep(sec);
		result.send();
		return true;
	}
	*/
	return false;
}


