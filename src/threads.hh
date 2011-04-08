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

#ifndef _THREADS_HH
#define _THREADS_HH

#include "client_thread.hh"

#include "users.hh"

class ReportThread : public ClientThread {
public:
	void main();
	ReportThread(Client *client);
};

class CleanupThread : public ClientThread {
public:
	void main();
	CleanupThread(Client *client);
};

class DumpThread : public ClientThread {
private:
	std::string target;
public:
	void main();
	DumpThread(Client *client, std::string const target);
};

class TopThread : public ClientThread {
private:
	int size;
public:
	void main();
	TopThread(Client *client, int const size);
};

#endif
