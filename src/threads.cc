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

#include "threads.hh"

#include "result.hh"

void ReportThread::main() {
	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	users.report(result.data);
	result.send();
}

ReportThread::ReportThread(Client *_client) : ClientThread(_client) {
}

void CleanupThread::main() {
	ClientResult result(client);
	users.cleanup();
	result.send();
}

CleanupThread::CleanupThread(Client *_client) : ClientThread(_client) {
}

void DumpThread::main() {
	ClientResult result(client);
	if (!users.dump(target))
		result.error("Could not dump data into \"" + target + "\"");
	result.send();
}

DumpThread::DumpThread(Client *_client, std::string const _target) : ClientThread(_client), target(_target) {
}

void TopThread::main() {
	ClientResult result(client);
	result.type = PHP_SERIALIZE;
	users.top(result.data, size);
	result.send();
}

TopThread::TopThread(Client *_client, int const _size) : ClientThread(_client), size(_size) {
}

