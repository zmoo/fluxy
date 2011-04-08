#include <iostream>
#include "messages.hh"

int main(int argc, char *argv[]) {

	FluxyMessages messages;

	FluxyMessageHead head;
	int id = 0;
	int time = 4549872;
	int type = 1;

	for (int i = 0; i < 2000; i++) {
		head.set_id(id++);
		head.set_date(time++);
		head.set_type(type = (type + 1) % 16);
		head.set_version(2);
		messages.del_before(1561, 100 - 1);
		FluxyMessage *message = messages.unshift(&head, "coucou");
		if (message) { 
			messages.update_body(message, "plopik voiturette");
			message->head.set_id(id++);
		}
	}

	messages.debug();
	return 0;
}
