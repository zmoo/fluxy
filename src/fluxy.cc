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
#include <fstream>
#include <sys/resource.h>
#include <cstdlib>
#include <csignal>

#include "log.hh"
#include "stringutils.hh"
#include "server.hh"
#include "autodump.hh"
#include "autocleanup.hh"
#include "replicator.hh"
#include "udp.hh"
#include "users.hh"
#include "user.hh"

typedef struct {
	std::string address;
	std::string port;
	std::string udp_address;
	std::string udp_port;
	std::string slave_address;
	std::string slave_port;
	std::string restore;
	std::string pidfile;
	std::string autodump_target;
	int autodump_delay;
	bool restore_autodump;
	int autocleanup_delay;
	int messages_expiration;
	int messages_size;
	bool verbose;
} Args;

void version() {
	std::cout << PACKAGE " " VERSION << std::endl;
}

void help() {
	std::cout << "Usage:" << std::endl;
	std::cout << "  fluxy [OPTION...]" << std::endl;
	std::cout << std::endl;
	std::cout << "Fluxy Options:" << std::endl;
	std::cout << "  -a, --address             IP adress of tcp server" << std::endl;
	std::cout << "  -p, --port                Port of tcp server" << std::endl;
	std::cout << "  -A, --udp-address         IP adress of udp server" << std::endl;
	std::cout << "  -P, --udp-port            Port of udp server" << std::endl;
	std::cout << "  -z, --slave-address       IP adress of slave server" << std::endl;
	std::cout << "  -Z, --slave-port          Port of slave server" << std::endl;
	std::cout << "  -r, --restore             Restore memory from dump file" << std::endl;
	std::cout << "  -R, --restore-autodump    Restore memory from autodump file" << std::endl;
	std::cout << "  -T, --autodump-target     Set path for autodump target" << std::endl;
	std::cout << "  -D, --autodump-delay      Set pause after each autodump (d/h/m/s)" << std::endl;
	std::cout << "  -c, --autocleanup-delay   Set pause after each autocleanup (d/h/m/s)" << std::endl;
	std::cout << "  -e, --messages-expiration Set messages expiration (d/h/m/s)" << std::endl;
	std::cout << "  -s, --messages-size       Max number of messages by user" << std::endl;
	std::cout << "  -i, --pidfile             Write the pid of the process to the given file" << std::endl;
	std::cout << "  -v, --verbose             Turn on verbose output" << std::endl;
	std::cout << "  -h, --help                Show help options and exit" << std::endl;
	std::cout << "  -V, --version             Show the version number and exit" << std::endl;
}

void parse_args(int argc, char **argv, Args &args) {
	struct option long_options[] = {
		{ "address", 1, NULL, 'a' },
		{ "port", 1, NULL, 'p' },
		{ "udp-address", 1, NULL, 'A' },
		{ "udp-port", 1, NULL, 'P' },
		{ "slave-address", 1, NULL, 'z' },
		{ "slave-port", 1, NULL, 'Z' },
		{ "restore", 1, NULL, 'r' },
		{ "restore-autodump", 0, NULL, 'R' },
		{ "autodump-target", 1, NULL, 'T' },
		{ "autodump-delay", 1, NULL, 'D' },
		{ "autocleanup-delay", 1, NULL, 'c' },
		{ "messages-expiration", 1, NULL, 'e' },
		{ "messages-size", 1, NULL, 's' },
		{ "pidfile", 1, NULL, 'i' },
		{ "verbose", 0, NULL, 'v' },
		{ "help", 0, NULL, 'h' },
		{ "version", 0, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	args.verbose = false;
	args.restore_autodump = false;
	args.autodump_delay = 0;
	args.autocleanup_delay = 0;
	args.messages_expiration = 0;
	args.messages_size = 0;

	int option_index, c;
	while ((c = getopt_long(argc, argv, "a:p:A:P:z:Z:r:RT:D:c:e:s:i:vhV", long_options, &option_index)) != -1) {
		switch (c) {
			case 'a':
				args.address = optarg;
				break;
			case 'p':
				args.port = optarg;
				break;
			case 'A':
				args.udp_address = optarg;
				break;
			case 'P':
				args.udp_port = optarg;
				break;
			case 'z':
				args.slave_address = optarg;
				break;
			case 'Z':
				args.slave_port = optarg;
				break;
			case 'r':
				args.restore = optarg;
				break;
			case 'R':
				args.restore_autodump = true;
				break;
			case 'T':
				args.autodump_target = optarg;
				break;
			case 'D':
				args.autodump_delay = StringUtils::to_time(optarg);
				break;
			case 'c':
				args.autocleanup_delay = StringUtils::to_time(optarg);
				break;
			case 'e':
				args.messages_expiration = StringUtils::to_time(optarg);
				break;
			case 's':
				args.messages_size = StringUtils::to_uint(optarg);
				break;
			case 'i':
				args.pidfile = optarg;
				break;
			case 'v':
				args.verbose = true;
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

void check_ulimit() {
	rlimit rlim;
	getrlimit(RLIMIT_NOFILE, &rlim);
	log.msg(LOG_NOTICE, "Max number of file descriptors: " + StringUtils::to_string((int) rlim.rlim_max), true);
}

void signal_handler(int sig) {
	std::string name;
	switch (sig) {
		case SIGINT: 
			name = "SIGINT"; 
			break;
		case SIGKILL: 
			name = "SIGKILL"; 
			break;
		case SIGTERM:
			name = "SIGTERM"; 
			break;
		default:
			name = "unknown";
	}	
	log.msg(LOG_NOTICE, "Received signal number: " + StringUtils::to_string(sig) + " (" + name +")", true);
	autodump.force();
	exit(0);
}

void signals_handle() {
	struct sigaction sa;
	sa.sa_handler = signal_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

void write_pid(std::string const path) {
	std::fstream f(path.c_str(), std::ios_base::out);
	if (f.is_open()) {
		f << getpid();
		f.close();
	}
	else
		log.msg(LOG_ERR, "Could not open output file '" + path + "'", true);
}

int main(int argc, char **argv) {
	Args args;
	parse_args(argc, argv, args);

	//Log
	log.open();
	log.verbose = args.verbose;

	//Save pid
	if (args.pidfile != "")
		write_pid(args.pidfile);

	//Restore data
	if (args.restore_autodump)
		args.restore = args.autodump_target;
	
	if (args.restore != "") {
		log.msg(LOG_NOTICE, "Start restoring data", true);
		bool result = users.restore(args.restore);
		if (!result) {
			log.msg(LOG_ERR, "Restore has failed", true);
			return -1;
		}
	}

	signals_handle();

	//Start Autodump
	autodump.data.set(args.autodump_target != "", args.autodump_target, args.autodump_delay != 0 ? args.autodump_delay : 3600);
	autodump.run();

	//Messages expiration
	user_messages_expiration = args.messages_expiration != 0 ? args.messages_expiration : 720 * 60 * 60;
	log.msg(LOG_NOTICE, "Messages expiration: " + StringUtils::to_string((int) user_messages_expiration) + "s", true);

	//Max number of messages by user
	user_messages_size = args.messages_size != 0 ? args.messages_size : 50;
	log.msg(LOG_NOTICE, "Max number of messages by user: " + StringUtils::to_string((int) user_messages_size), true);

	//Start Autocleanup
	AutoCleanup *auto_cleanup = new AutoCleanup(args.autocleanup_delay != 0 ? args.autocleanup_delay : 3600 * 6);
	auto_cleanup->run();

	//Start replicator
	if (args.slave_address != "" and args.slave_port != "") {
		replicator.open(args.slave_address, args.slave_port);
	}

	//Init libevent
	event_init();

	//Start udp server
	UdpServer udp_server;
	if (args.udp_address != "" and args.udp_port != "" and udp_server.open(args.udp_address, args.udp_port)) {
		udp_server.listen();
	}

	//Start tcp server
	check_ulimit();
	if (!server.open(args.address == "" ? "0.0.0.0" : args.address, args.port == "" ? "8888" : args.port))
		return -1;

	server.listen();

	//Start libevent main loop
	event_dispatch();

	//Close
	log.msg(LOG_NOTICE, "Fluxy was stopped");
	log.close();
}
