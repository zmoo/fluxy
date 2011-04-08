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

#include "config.h"

#include <getopt.h>
#include <iostream>
#include <sys/resource.h>
#include <cstdlib>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

int udp_open(std::string const address, std::string const port) {
	addrinfo hints, *res;
	int fd;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_addr = NULL;

	//get address/port
	int fodder;
	if ((fodder = getaddrinfo(address.c_str(), port.c_str(), &hints, &res)) != 0) {
		std::cerr << gai_strerror(fodder) << std::endl;
		return -1;
	}

	//create socket
	if ((fd = socket(res->ai_family, hints.ai_socktype, hints.ai_protocol)) == -1) {
		freeaddrinfo(res);
		std::cerr << "Can not create socket" << std::endl;
		return -1;
	}

	//bind
	if (bind(fd, (struct sockaddr *) res->ai_addr, (socklen_t) res->ai_addrlen) != 0) {
		std::cerr << "Can not bind socket" << std::endl;
		close(fd);
		freeaddrinfo(res);
		return -1;
	}

	return fd;
}

void udp_output(int const fd) {
	char buf[1024];
	size_t readen = read(fd, &buf, sizeof(buf));
	if (readen == 0 or readen == sizeof(buf)) {
		return;
	}
	fwrite(&buf, readen, 1, stdout);
	fprintf(stdout, "\n");
}

typedef struct {
	std::string address;
	std::string port;
} Args;

void version() {
	std::cout << PACKAGE "-listen " VERSION << std::endl;
}

void help() {
	std::cout << "Usage:" << std::endl;
	std::cout << "  fluxy-listen [OPTION...]" << std::endl;
	std::cout << std::endl;
	std::cout << "Fluxy-listen Options:" << std::endl;
	std::cout << "  -a, --address            IP adress of upd server" << std::endl;
	std::cout << "  -p, --port               Port of upd server" << std::endl;
	std::cout << "  -h, --help               Show help options and exit" << std::endl;
	std::cout << "  -V, --version            Show the version number and exit" << std::endl;
}

void parse_args(int argc, char **argv, Args &args) {
	struct option long_options[] = {
		{ "address", 1, NULL, 'a' },
		{ "port", 1, NULL, 'p' },
		{ "help", 0, NULL, 'h' },
		{ "version", 0, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	int option_index, c;
	while ((c = getopt_long(argc, argv, "a:p:hV", long_options, &option_index)) != -1) {
		switch (c) {
			case 'a':
				args.address = optarg;
				break;
			case 'p':
				args.port = optarg;
				break;
			case 'h':
				help();
				exit(0);
				break;
			case 'V':
				version();
				exit(0);
				break;
		}
        }
}

int main(int argc, char **argv) {
	Args args;
	parse_args(argc, argv, args);

	//Start udp server
	if (args.address != "") {
		int fd = udp_open(args.address, args.port == "" ? "8787" : args.port);
		if (fd != -1) {
			for (;;) {
				udp_output(fd);
			}
		}
	}
}


