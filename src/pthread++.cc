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

#include "pthread++.hh"

void PThread::exit() {
	delete this;
	pthread_exit(NULL);
}

void *pthread_run(void *data) {
	PThread *thread = (PThread *) data;
	thread->main();
	thread->exit();
	return NULL;
}	

void PThread::run() {
       pthread_create(&handle, &attr, pthread_run, this);
}

void PThread::join() {
	pthread_join(handle, NULL);
}

PThread::PThread(bool const joinable) {
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, (joinable) ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED);
}

PThread::~PThread() {
	pthread_attr_destroy(&attr);
}

bool PMutex::trylock() {
	return (pthread_mutex_trylock(&handle) == 0);
}

void PMutex::lock() {
	pthread_mutex_lock(&handle);
}

void PMutex::unlock() {
	pthread_mutex_unlock(&handle);
}

PMutex::PMutex() {
	pthread_mutex_init(&handle, NULL);
}

PMutex::~PMutex() {
	pthread_mutex_destroy(&handle);
}

