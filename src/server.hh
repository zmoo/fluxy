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

#ifndef _SERVER_HH
#define _SERVER_HH

#include <sstream>
#include <string>

#include "events.hh"
#include "stats.hh"
#include "words_parser.hh"
#include "result.hh"

#include "config.h"

class ClientFluxy : public Client {
private:
	OutputType mode;

public:
	void parse_query(ClientResult &result, WordsParser *parser);
	void receive();

	ClientFluxy(int const client_fd);
};

class ServerFluxy : public Server {
protected:
	void new_client(int const fd);

public:
	ServerFluxy();
};

#ifdef _SERVER_CC
ServerFluxy server;
#else
extern ServerFluxy server;
#endif

#endif
