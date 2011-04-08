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

#define _SERVER_CC

#include "config.h"
#include "server.hh"

#include <cstdio>
#include <fstream>

#include "users.hh"
#include "log.hh"
#include "stringutils.hh"
#include "help.hh"
#include "recycler.hh"
#include "autodump.hh"
#include "addstats.hh"

void ClientFluxy::parse_query(ClientResult &result, WordsParser *parser) {
	//!TID <n> <command> 
	//!	Give a Transaction Id that will be returned with the command's result
	if (parser->current == "TID") {
		result.tid = StringUtils::to_uint(parser->next());
		parser->next();
	}

	if (users.parse_query(result, parser, mode)) {
		return;
	}

	//!info 
	//!	Get server information
	if (parser->current == "info") {
		stats.inc("info");
		result.data << "Fluxy version: " VERSION << std::endl;
		result.data << "Libevent version: " << event_get_version() << std::endl;
		result.send();
		return;
	}

	//!quit
	//!	Close connection with client
	if (parser->current == "quit") {
		stats.inc("misc");
		exit();
		return;
	}

	//!stats 
	//!	Get statistics about server
	if (parser->current == "stats") {
		stats.inc("stats");

		result.type = mode;
		switch (mode) {
			case TEXT:
				result.data << "STAT uptime " << server.uptime() << std::endl;
				result.data << "STAT users " << users.count() << std::endl;			
				stats.show(result.data, "commands::");
				break;
			default:
				result.data << "a:3:{";
				result.data << "s:6:\"uptime\";i:" << server.uptime() << ";";
				result.data << "s:5:\"users\";i:" << users.count() << ";";
				result.data << "s:8:\"commands\";";
				stats.serialize_php(result.data);
				result.data << "}";
				break;
		}
		result.send();
		return;
	}

	//!addstats 
	//!	Get statistics about type of added messages
	if (parser->current == "addstats") {
		stats.inc("addstats");
		result.type = PHP_SERIALIZE;
		addstats.serialize_php(result.data);
		result.send();
		return;
	}

	//!recycler stats 
	//!	Get statistics about memory recycler
	if (parser->current == "recycler") {
		parser->next();
		if (parser->current == "stats") {
			stats.inc("recycler::stats");
			result.type = PHP_SERIALIZE;
			recycler.stats(result.data);
			result.send();
			return;
		}
	}

	//!autodump <command>
	//!	Execute a command on autodump
	//!	See "autodump help" for more information
	if (parser->current == "autodump") {
		parser->next();
		autodump.data.lock();
		autodump.data.parse_query(result, "autodump::", parser, mode);
		autodump.data.unlock();	
		return;
	}

	//!help
	//!	Show commands list
	if (parser->current == "help") {
		stats.inc("misc");
		result.data << HELP_SERVER;
		result.send();
		return;
	}

	//!mode <text|php_serialize|none> 
	//!	Set default output format
	if (parser->current == "mode") {
		stats.inc("misc");
		parser->next();
		if (parser->current == "text")
			mode = TEXT;
		else if (parser->current == "php_serialize")
			mode = PHP_SERIALIZE;
		else
			result.error("Unknown output mode");
		result.send();
		parser->next();			
		return;
	}

	//not a valid command
	stats.inc("unvalid");
	result.error("Not a valid command. Send command 'help' for more information.");
	result.send();
}

void ClientFluxy::receive() {
	try {
		std::stringstream stream;
		read(stream);

		WordsParser parser(&stream);
		parser.next();
		ClientResult result(this);
		parse_query(result, &parser);
	}
	catch (...) {
		log.msg(LOG_ERR, "Not a valid command buffer");
	}
}

ClientFluxy::ClientFluxy(int const _client_fd) : Client(_client_fd) {
	mode = PHP_SERIALIZE;
}

void ServerFluxy::new_client(int const _fd) {
	new ClientFluxy(_fd);
}

ServerFluxy::ServerFluxy() {
}
