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

#ifndef _RESULT_HH
#define _RESULT_HH

#include <sstream>

#include <sys/types.h>
#include "events.hh"

#define ERROR_DEFAULT 1

typedef enum {
	TEXT,
	PHP_SERIALIZE,
	NONE
} OutputType;

class ClientResult {
private:
	Client *client;

public:
	int tid;
	OutputType type;
	int error_code;
	std::stringstream data;
	bool replicated;

public:
	Client *get_client();
	void error(std::string const msg = "", int const code = ERROR_DEFAULT);
	void msg(std::string const msg);
	void send();

	ClientResult(Client *client);
};

#endif
