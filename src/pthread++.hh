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

#ifndef PTHREADS_PP
#define PTHREADS_PP

#include <pthread.h>

class PThread {
private:
	pthread_t handle;
	pthread_attr_t attr;

protected:
	void exit();

public:
	friend void *pthread_run(void *data);

	virtual void main() = 0;
	void run();
	void join();

	PThread(bool const joinable = false);
	virtual ~PThread();
};

class PMutex {
private:
	 pthread_mutex_t handle;

public:
	void lock();
	void unlock();
	bool trylock();

	PMutex();
	~PMutex();
};

#endif
