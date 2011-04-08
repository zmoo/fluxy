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

#include <iostream>
#include <sstream>
#include <getopt.h>
#include <cstdlib>

#include "fluxy_client.hh"
#include "stringutils.hh"

typedef struct {
	std::string address;
	std::string port;
	time_t timeout;
} Args;

void version() {
	std::cout << PACKAGE "-query " VERSION << std::endl;
}

void help() {
	std::cout << "Usage:" << std::endl;
	std::cout << "  fluxy-query [OPTION...] query" << std::endl;
	std::cout << std::endl;
	std::cout << "Fluxy-query Options:" << std::endl;
	std::cout << "  -a, --address      IP adress of upd server (default value: \"localhost\")" << std::endl;
	std::cout << "  -p, --port         Port of upd server (default value: \"8888\")" << std::endl;
	std::cout << "  -t, --timeout      Set timeout (default value: 30s)" << std::endl;
	std::cout << "  -h, --help         Show help options and exit" << std::endl;
	std::cout << "  -V, --version      Show the version number and exit" << std::endl;
}

int parse_args(int argc, char **argv, Args &args) {
	struct option long_options[] = {
		{ "address", 1, NULL, 'a' },
		{ "port", 1, NULL, 'p' },
		{ "timeout", 1, NULL, 't' },
		{ "help", 0, NULL, 'h' },
		{ "version", 0, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	args.timeout = 30;
	args.address = "localhost";
	args.port = "8888";
	int option_index, c;
	while ((c = getopt_long(argc, argv, "a:p:t:hV", long_options, &option_index)) != -1) {
		switch (c) {
			case 'a':
				args.address = optarg;
				break;
			case 'p':
				args.port = optarg;
				break;
			case 't':
				args.timeout = StringUtils::to_time(optarg);
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
	return optind;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		help();
		return 0;
	}

	Args args;
	int count = parse_args(argc, argv, args);

	FluxyClient client;
	client.set(args.address, args.port, args.timeout);

	if (!client.connect()) {
		std::cerr << "Could not connect" << std::endl;
		return 2;
	}

	for (int i = count; i < argc; i++) {
		std::string result;
		FluxyClient::QueryResult res = client.query(result, argv[i]);
		if (res != FluxyClient::OK)
			return 3;

		std::cout << result << std::endl;
	}
}
