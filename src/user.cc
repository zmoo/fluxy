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

#define _USER_CC

#include "user.hh"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "help.hh"
#include "stats.hh"
#include "addstats.hh"
#include "stringutils.hh"
#include "recycler.hh"
#include "replicator.hh"
#include "autoid.hh"
#include "fetcher.hh"
#include "log.hh"

//------ UserMessageSearch

void UserMessageSearch::parse(WordsParser *parser) {
	id = StringUtils::to_uint(parser->next());
	check_type = false;
	parser->next();
	if (parser->current == ":") {
		check_type = true;
		type = StringUtils::to_uint(parser->next());
		parser->next();
	}
}

void UserMessageSearch::output(std::stringstream &out) {
	out << id;
	if (check_type) 
		out << ":" << (int) type;
}

//------ UserMessageSearch

UserMessageFilter::UserMessageFilter() : id(false), date(false), type(false), flags(false), version(false), seen(false), body(false) {
}

//------ UserMessageValues

void UserMessageValues::copy_head(FluxyMessageHead &head) const {
	head.set_id(id);
	head.set_date(date);
	head.set_type(type);
	head.set_flags(flags);
	head.set_version(version);
	head.set_seen(seen);
}

UserMessageValues::UserMessageValues(FluxyMessage const *message) {
	id = FLUXY_MESSAGE_HEAD_GET_ID(&message->head);
	date = FLUXY_MESSAGE_HEAD_GET_DATE(&message->head);
	type = FLUXY_MESSAGE_HEAD_GET_TYPE(&message->head);
	flags = FLUXY_MESSAGE_HEAD_GET_FLAGS(&message->head);
	version = FLUXY_MESSAGE_HEAD_GET_VERSION(&message->head);
	seen = FLUXY_MESSAGE_HEAD_GET_SEEN(&message->head);
	body = FLUXY_MESSAGE_BODY_STR(message);
}

UserMessageValues::UserMessageValues() {
}

bool UserMessageValues::parse_query(WordsParser *parser, bool &autoid) {
	//id
	id = 0;
	autoid = false;
	if (parser->current == "autoid") {
		parser->next();
		id = StringUtils::to_uint(parser->current);
		auto_id.set(id);
		autoid = true;
	}
	else if (parser->current == "auto") {
		id = auto_id.inc();
		autoid = true;
	}
	else {
		id = StringUtils::to_uint(parser->current);
	}
	parser->next();

	//date
	date = (parser->current == "now") ? time(NULL) : StringUtils::to_uint(parser->current);

	//type
	type = StringUtils::to_uint(parser->next());

	//flags
	flags = StringUtils::to_uint(parser->next());

	//version
	version = StringUtils::to_uint(parser->next());

	//seen
	seen = false;

	//body
	parser->next_char();
	body = parser->until_end();
	return true;
}

void UserMessageValues::replicate_query(std::stringstream &output, bool autoid) {
	if (autoid)
		output << "autoid ";
	output << id << " " << date << " " << (int) type << " " << flags << " " << (int) version << " " << body;
}

//------ UserMessages

FluxyMessage *UserMessages::find(UserMessageSearch const search) const {
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		if (it->head.get_id() == search.id and (!search.check_type or it->head.get_type() == search.type)) { 
			return it;
		}
	}
	return NULL;
}

void UserMessages::debug() {
	FluxyBuffer::debug();
	FluxyMessages::debug();
}

bool UserMessages::add(UserMessageValues const values) {
	del_before(time(NULL) - user_messages_expiration, user_messages_size - 1);

	FluxyMessageHead head;
	values.copy_head(head);
	return (unshift(&head, values.body.c_str()) != NULL);
}

bool UserMessages::del(UserMessageSearch const search) {
	FluxyMessage *message = find(search);
	if (message == NULL)
		return false;

	FluxyMessages::del(message);
	return true;
}

bool UserMessages::update(UserMessageSearch const search, UserMessageFilter const filter, UserMessageValues const values) {
	FluxyMessage *message = find(search);
	if (!message) {
		return false;
	}

	if (filter.id) message->head.set_id(values.id);
	if (filter.date) message->head.set_date(values.date);
	if (filter.type) message->head.set_type(values.type);
	if (filter.flags) message->head.set_flags(values.flags);
	if (filter.version) message->head.set_version(values.version);
	if (filter.seen) message->head.set_seen(values.seen);

	if (filter.body) {
		update_body(message, values.body.c_str());
	}

	return true;
}

bool UserMessages::seen(UserMessageSearch const last, bool const value) {
	FluxyMessage *message = find(last);
	if (!message) {
		return false;
	}

	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH2(message, (FluxyMessage *) ptr_end(), it) {
		it->head.set_seen(value);
	}
	return true;
}

bool UserMessages::seen_all(bool const value) {
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		it->head.set_seen(value);
	}
	return true;
}

int UserMessages::count() {
	return FluxyMessages::count();
}

int UserMessages::count_unseen() {
	int i = 0;
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		if (!it->head.get_seen()) 
			i++;
	}
	return i;
}

void UserMessages::serialize_php(std::stringstream &out) {
	out << "a:" << count() << ":{";

	int i = 0;
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		out << "i:" << i << ";a:7:{";
		out << "s:2:\"id\";i:" << it->head.get_id() << ";";
		out << "s:4:\"date\";i:" << it->head.get_date() << ";";
		out << "s:4:\"type\";i:" << (int) it->head.get_type() << ";";
		out << "s:5:\"flags\";i:" << it->head.get_flags() << ";";
		out << "s:7:\"version\";i:" << (int) it->head.get_version() << ";";
		out << "s:4:\"seen\";b:" << (it->head.get_seen() ? "1" : "0") << ";";

		std::string body = it->get_body();
		out << "s:4:\"body\";s:" << body.length() << ":\"" << body << "\";";
		out << "}";
		i++;
	}
	out << "}";
}

void UserMessages::cleanup() {
	del_before(time(NULL) - user_messages_expiration);
}

void UserMessages::clear() {
	FluxyBuffer::clear();
}

void UserMessages::fetch(UserId const user_id, Fetcher &fetcher, UserMessageFlags const mask, UserMessageFlags const value, time_t const after, time_t const before) {
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		if (before == 0 or it->head.get_date() <= before) {
			if (mask == 0 or (it->head.get_flags() & mask) == value) {
				fetcher.add(user_id, it);
			}
		}
	}
}

UserScore UserMessages::score() {
	FluxyMessage *last = num(0);
	if (!last) return 0;

	FluxyMessage *first = num(10);
	if (!first) return 0;

	time_t delta_t = last->head.get_date() - first->head.get_date();
	return 3600 * 10 / delta_t;
}

void UserMessages::dump(FILE *f) {
	FluxyBuffer::fwrite(f);
}

bool UserMessages::restore(FILE *f) {
	FluxyBuffer::fread(f);
	return FluxyMessages::check();
}

bool UserMessages::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {

	//!messages add <id>|auto <date>|now <type> <flags> <version> <body>
	//!	Add new message
	if (parser->current == "add") {
		stats.inc(cmd_prefix + "add");
		parser->next();

		UserMessageValues values;

		bool autoid;
		values.parse_query(parser, autoid);

		if (!add(values))
			return false;

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "add ";
			values.replicate_query(replication_query, autoid);
			replicator.add(replication_query);
		}

		result.type = PHP_SERIALIZE;
		result.data << "a:2:{s:2:\"id\";i:" << values.id << ";s:4:\"date\";i:" << values.date << ";}";
		return true;
	}

	//!messages delete <id>[:<type>]
	//!	Delete message
	if (parser->current == "delete") {
		stats.inc(cmd_prefix + "delete");

		UserMessageSearch search;
		search.parse(parser);

		if (!del(search))
			return false;

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "delete ";
			search.output(replication_query);
			replicator.add(replication_query);
		}
		return true;
	}

	//!messages update <id>[:<type>] [date <date>] [type <type>] [flags <flags>] [version <version>] [seen <1/0>] [body <body>]
	//!	Update message
	if (parser->current == "update") {
		stats.inc(cmd_prefix + "update");

		UserMessageSearch search;
		UserMessageValues values;
		UserMessageFilter filter;

		//search
		search.parse(parser);

		//date
		if (parser->current == "date") {
			filter.date = true;
			values.date = StringUtils::to_uint(parser->next());
			parser->next();
		}

		//type
		if (parser->current == "type") {
			filter.type = true;
			values.type = StringUtils::to_uint(parser->next());
			parser->next();
		}

		//flags
		if (parser->current == "flags") {
			filter.flags = true;
			values.flags = StringUtils::to_uint(parser->next());
			parser->next();
		}

		//version
		if (parser->current == "version") {
			filter.version = true;
			values.version = StringUtils::to_uint(parser->next());
			parser->next();
		}

		//seen
		if (parser->current == "seen") {
			filter.seen = true;
			values.seen = (parser->next() != "0");
			parser->next();
		}

		//body
		if (parser->current == "body") {
			filter.body = true;
			parser->next_char();
			values.body = parser->until_end();
		}

		if (!update(search, filter, values))
			return false;

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "update ";
			search.output(replication_query);
			if (filter.date) replication_query << " date " << values.date;
			if (filter.type) replication_query << " type " << (int) values.type;
			if (filter.flags) replication_query << " flags " << values.flags;
			if (filter.version) replication_query << " version " << (int) values.version;
			if (filter.seen) replication_query << " seen " << (values.seen ? "1" : "0");
			if (filter.body) replication_query << " body " << values.body;
			replicator.add(replication_query);
		}
		return true;
	}

	//!messages get
	//!	Get messages
	if (parser->current == "get") {
		stats.inc(cmd_prefix + "get");
		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		return true;
	}

	//!messages seen all|last <id>[:<type>]
	//!	Mark messages as seen
	if (parser->current == "seen") {
		stats.inc(cmd_prefix + "seen");
		parser->next();

		bool all = false;
		UserMessageSearch last;

		if (parser->current == "all")
			all = true;
		else if (parser->current == "last")
			last.parse(parser);
		else
			return false;
		
		if (all) {
			seen_all(true);
		}
		else {
			if (!seen(last, true))
				return false;
		}

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "seen";
			if (all) {
				replication_query << " all";
			} 
			else {
				replication_query << " last ";
				last.output(replication_query);
			}
			replicator.add(replication_query);
		}
		return true;
	}

	//!messages count [unseen]
	//!	Count messages
	if (parser->current == "count") {
		stats.inc(cmd_prefix + "count");
		bool unseen_only = (parser->next() == "unseen");

		result.type = PHP_SERIALIZE;
		result.data << "i:" << (unseen_only ? count_unseen() : count()) << ";";
		return true;
	}

	//!messages clear
	//!	Clear messages
	if (parser->current == "clear") {
		stats.inc(cmd_prefix + "clear");
		clear();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "clear";
			replicator.add(replication_query);
		}
		return true;
	}

	if (parser->current == "debug") {
		debug();
	}

	return false;
}

//------ UserData

void UserData::serialize_php(std::stringstream &out) {
	size_t size = get_size() ;
	out << "s:" << size << ":\"";
	out.write((char *) ptr_begin(), size);
	out << "\";";
}

bool UserData::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser, std::stringstream &replication_query) {
	//!data set
	//!	Get data
	if (parser->current == "set") {
		stats.inc(cmd_prefix + "set");

		parser->next_char();
		std::string value = parser->until_end();
		memcpy((void *) value.data(), value.size());
		return true;
	}

	//!data get
	//!	Get data
	if (parser->current == "get") {
		stats.inc(cmd_prefix + "get");
		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		return true;
	}

	//!data clear
	//!	Clear data
	if (parser->current == "clear") {
		stats.inc(cmd_prefix + "clear");
		clear();
		return true;
	}
	return false;
}

//------ User

PMutex *User::lock() {
	PMutex *mutex = &user_locks.lock[id & 0x3FF];
	mutex->lock();
	return mutex;
}

void User::cleanup() {
	messages.cleanup();
}

void User::show(std::stringstream &out) {
	out << "Id: " << id << std::endl;
}

void User::serialize_php(std::stringstream &out) {
	out << "a:3:{";
	out << "s:2:\"id\";i:" << id << ";";
	out << "s:5:\"flags\";i:" << flags << ";";
	out << "s:8:\"messages\";";
	messages.serialize_php(out);
	out << "}";
}

bool User::parse_query(ClientResult &result, std::string const cmd_prefix, WordsParser *parser) {
	std::stringstream replication_query;
	if (replicator.opened and !result.replicated) {
		replication_query << "#user *" << id << " messages ";
	}

	if (parser->current == "messages") {
		parser->next();
		return messages.parse_query(result, cmd_prefix + "messages::", parser, replication_query); 
	}
/*
	if (parser->current == "data") {
		parser->next();
		return data.parse_query(result, cmd_prefix + "data::", parser, replication_query); 
	}
*/
	//!get
	//!	Get user
	if (parser->current == "get") {
		stats.inc(cmd_prefix + "get");
		result.type = PHP_SERIALIZE;
		serialize_php(result.data);
		return true;
	}

	//!show
	//!	Show user data
	if (parser->current == "show") {
		stats.inc(cmd_prefix + "show");
		show(result.data);
		return true;
	}

	//!delete
	//!	Delete user
	if (parser->current == "delete") {
		stats.inc(cmd_prefix + "delete");
		del();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "delete";
			replicator.add(replication_query);
		}
		return true;
	}

	//!clear
	//!	Clear user
	if (parser->current == "clear") {
		stats.inc(cmd_prefix + "clear");
		clear();

		//replication
		if (replicator.opened and !result.replicated) {
			replication_query << "clear";
			replicator.add(replication_query);
		}
		return true;
	}

	//!help
	//!	Show commands list
	if (parser->current == "help") {
		stats.inc("misc");
		result.data << HELP_USER;
		return true;
	}
	return false;
}

void User::clear() {
	flags = 0;
	messages.clear();
}

void User::del() {
	clear();
	_deleted = true;
}

void User::undel() {
	_deleted = false;
}

bool User::deleted() {
	return _deleted;
}

UserScore User::score() {
	return messages.score();
}

void User::dump(FILE *f) {
	fwrite(&id, 1, sizeof(id), f);
	fwrite(&flags, 1, sizeof(flags), f);

	messages.dump(f);
}

//TODO: stop restoring process if SANITARY_CHECK fault
#define SANITARY_CHECK(readen, size) \
	if (readen != size) { \
		log.msg(LOG_ERR, "Internal error in " __FILE__ " line " + StringUtils::to_string(__LINE__), 1); \
		return; \
	}

#define SANITARY_READ(var, f) { \
	size_t readen = fread(&var, 1, sizeof(var), f); \
	SANITARY_CHECK(readen, sizeof(var)); \
}

User::User(FILE *f) : _deleted(false) {
	SANITARY_READ(id, f);
	SANITARY_READ(flags, f);

	if (!messages.restore(f)) {
		log.msg(LOG_ERR, "Internal error in " __FILE__ " line " + StringUtils::to_string(__LINE__), 1);
	}
}

User::User(UserId const _id) : id(_id), flags(0), _deleted(false) {
}

void UsersReport::add(User &user) {
	count++;
	if (user.deleted()) {
		deleted_count++;
	}

	if (user.messages.empty()) {
		empty++;
	}
	else {
		int idx = user.messages.get_scale() - FLUXY_SCALE_MIN;
		if (idx >= 0 and idx < MEMSTATS_COUNT)
			memstats[idx]++;
	}
}

void UsersReport::serialize_php(std::stringstream &out) {
	out << "a:3:{";
	out << "s:5:\"count\";i:" << count << ";";

	out << "s:8:\"memstats\";";
	out << "a:" << MEMSTATS_COUNT + 1 << ":{";
	out << "i:0;i:" << empty << ";";
	for (int i = 0; i < MEMSTATS_COUNT; i++)
		out << "i:" << (1 << (i + FLUXY_SCALE_MIN)) << ";i:" << memstats[i] << ";";
	out << "}";

	out << "s:13:\"deleted_count\";i:" << deleted_count << ";";
	out << "}";
}

UsersReport::UsersReport() {
	for (int i = 0; i < MEMSTATS_COUNT; i++)
		memstats[i] = 0;
	empty = 0;
	count = 0;
	deleted_count = 0;
}

