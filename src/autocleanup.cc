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

#include "autocleanup.hh"

#include "log.hh"
#include "users.hh"
#include "stringutils.hh"

void AutoCleanup::main() {
	log.msg(LOG_NOTICE, "Start autocleanup (delay: " + StringUtils::to_string(wait) + "s)", true);

	for (;;) {	
		sleep(wait);

		log.msg(LOG_NOTICE, "Start autocleanup.");
		users.cleanup();
		log.msg(LOG_NOTICE, "Autocleanup finished.");
	}
}

AutoCleanup::AutoCleanup(int const _wait) : wait(_wait) {
}

