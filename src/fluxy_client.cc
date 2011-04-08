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

#include "fluxy_client.hh"

#include <sys/socket.h>

#include <errno.h>
#include <netdb.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdio.h>

void FluxyClient::set(std::string const _address, std::string const _port, long int const _timeout_sec) {
	address = _address;
	port = _port;
	timeout_sec = _timeout_sec;
}

bool FluxyClient::connect() {
	struct addrinfo hints, *res;

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_addr = NULL;

	//fetch address/port
	int fodder;
	if ((fodder = getaddrinfo(address.c_str(), port.c_str(), &hints, &res)) != 0) {
		return false;
	}

	//create socket
	if ((fd = socket(res->ai_family, hints.ai_socktype, hints.ai_protocol)) == -1) {
		freeaddrinfo(res);
		return false;
	}

	//set socket options
	struct timeval timeout;
	timeout.tv_sec = timeout_sec;
	timeout.tv_usec = 0;
	if (
		(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) != 0) | 
		(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) != 0)
	) {
		::close(fd);
		return false;
	}

	//bind
	if (::connect(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		::close(fd);
		freeaddrinfo(res);
		return false;
	}
	return true;
}

void FluxyClient::close() {
	::close(fd);
}

FluxyClient::QueryResult FluxyClient::query(std::string &result, char const *str) {
	if (fd == -1) {
		return INTERNAL_ERROR;
	}

	//prepare query
	tid = (tid + 1) % 65535;
	char query[4096];

	int query_len = sprintf(query, "TID %i %s", tid, str);
	if (query_len == -1) {
		return INTERNAL_ERROR;
	}

	//send
	size_t size = send(fd, query, query_len, MSG_NOSIGNAL);
	if (size != (size_t) query_len) {
		if (errno == EPIPE) {
			//std::cerr << "end of file" << std::endl;
			return CONNECTION_CLOSED;
		}
		return CONNECTION_ERROR;
	}

	//get result
	std::stringstream data;
	char buffer[4096];
	bool maybe_last = false;
	for (;;) {
		size_t readen = read(fd, buffer, sizeof(buffer));

		//end of file
		if (readen == 0) {
			//std::cerr << "end of file" << std::endl;
			return CONNECTION_CLOSED;
		}

		//error
		if (readen == (size_t) -1) { 
			//std::cerr << "read error " << errno << " " << EINTR << " " << EPIPE << std::endl;
			return CONNECTION_ERROR;
		}
		data.write((char *) &buffer, readen - 2);

		//end detection
		if (readen >= 2 and buffer[readen - 2] == '\r' and buffer[readen - 1] == '\n') {
			break;
		}
		if (maybe_last and readen == 1 and buffer[readen - 1] == '\n') {
			break;
		}
		maybe_last = buffer[readen - 1] == '\r';
	}
	result = data.str();
	return OK;
}

FluxyClient::FluxyClient() : tid(0), fd(-1) {
}
