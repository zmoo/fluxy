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

#ifndef _FLUXY_CLIENT_HH
#define _FLUXY_CLIENT_HH

#include <string>

class FluxyClient {
private:
	int tid;
	int fd;

	std::string address;
	std::string port;
	time_t timeout_sec;

public:
	typedef enum {
		OK,
		INTERNAL_ERROR,
		QUERY_ERROR,
		CONNECTION_ERROR,
		CONNECTION_CLOSED
	} QueryResult;


	void set(std::string const address, std::string const port, long int const timeout_sec = 30);
	bool connect();
	void close();
	QueryResult query(std::string &result, char const *str);

	FluxyClient();
};

#endif
