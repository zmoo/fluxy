#include "messages.hh"
#include <stdlib.h>
#include <stdio.h>

//-----FluxyMessageHead
FluxyMessageHead::FluxyMessageHead() {
	FLUXY_MESSAGE_HEAD_INIT(this);
}

void FluxyMessageHead::set_id(FluxyMessageId const value) {
	FLUXY_MESSAGE_HEAD_SET_ID(this, value);
}

FluxyMessageId FluxyMessageHead::get_id() const {
	return FLUXY_MESSAGE_HEAD_GET_ID(this);
}

void FluxyMessageHead::set_date(time_t const value) {
	FLUXY_MESSAGE_HEAD_SET_DATE(this, value);
}

time_t FluxyMessageHead::get_date() const {
	return FLUXY_MESSAGE_HEAD_GET_DATE(this);
}

void FluxyMessageHead::set_type(FluxyMessageType const value) {
	FLUXY_MESSAGE_HEAD_SET_TYPE(this, value);
}

FluxyMessageType FluxyMessageHead::get_type() const {
	return FLUXY_MESSAGE_HEAD_GET_TYPE(this);
}

void FluxyMessageHead::set_version(FluxyMessageVersion const value) {
	FLUXY_MESSAGE_HEAD_SET_VERSION(this, value);
}

FluxyMessageVersion FluxyMessageHead::get_version() const {
	return FLUXY_MESSAGE_HEAD_GET_SEEN(this);
}

void FluxyMessageHead::set_seen(bool const value) {
	FLUXY_MESSAGE_HEAD_SET_SEEN(this, value);
}

bool FluxyMessageHead::get_seen() const {
	return FLUXY_MESSAGE_HEAD_GET_SEEN(this);
}

void FluxyMessageHead::set_flags(FluxyMessageFlags const value) {
	FLUXY_MESSAGE_HEAD_SET_FLAGS(this, value);
}

FluxyMessageFlags FluxyMessageHead::get_flags() const {
	return FLUXY_MESSAGE_HEAD_GET_FLAGS(this);
}

//-----FluxyMessage
FluxyMessage *FluxyMessage::next() {
	return FLUXY_MESSAGE_NEXT(this);
}

std::string FluxyMessage::get_body() const {
	return FLUXY_MESSAGE_BODY_STR(this);
}

void FluxyMessage::debug() {
	printf("= MESSAGE =\n");
	printf("id: %i\n", FLUXY_MESSAGE_HEAD_GET_ID(&head));
	printf("date: %li\n", FLUXY_MESSAGE_HEAD_GET_DATE(&head));
	printf("type: %i\n", FLUXY_MESSAGE_HEAD_GET_TYPE(&head));
	printf("version: %i\n", FLUXY_MESSAGE_HEAD_GET_VERSION(&head));
	printf("seen: %s\n", FLUXY_MESSAGE_HEAD_GET_SEEN(&head) ? "true" : "false");
	printf("body: [");
	fwrite(&body, 1, body_len, stdout);
	printf("]\n");
}

//-----FluxyMessages
bool FluxyMessages::expand(FluxyMessage *from, int delta) {
	size_t buffer_size = get_size();
	size_t offset = offset_of((char *) from);
	size_t size_after = buffer_size - offset;

	if (!set_size(buffer_size + delta))
		return false;

	if (size_after) {
		from = (FluxyMessage *) ptr_offset(offset);
		char *dest = (char *) from + delta;
		memmove(dest, from, size_after);
	}
	return true;
}

bool FluxyMessages::reduce(FluxyMessage *from, int delta) {
	size_t buffer_size = get_size();
	if (delta > (int) buffer_size)
		return false;

	size_t size_after = ptr_end() - (char *) from;
	if (size_after) {
		char *dest = (char *) from - delta;
		memmove(dest, from, size_after);
	}

	if (!set_size(buffer_size - delta))
		return false;

	return true;
}

bool FluxyMessages::move(FluxyMessage *from, int delta) {
	if (delta > 0)
		return expand(from, delta);
	else if (delta < 0)
		return reduce(from, delta * -1);
	return true;
}

bool FluxyMessages::truncate(FluxyMessage *from) {
	char *begin = ptr_begin();
	char *end = ptr_end();
	if ((char *) from < begin || (char *) from > end)
		return false;
	
	size_t size = (char *) from - begin;
	if (!set_size(size))
		return false;

	return true;
}

FluxyMessage *FluxyMessages::insert(FluxyMessage *from, FluxyMessageHead *head, char const *body) {
	size_t body_len = strlen(body);
	size_t size = FLUXY_MESSAGE_SIZE_CALC(body_len);
	size_t offset = offset_of((char *) from);

	if (!expand(from, size))
		return NULL;

	FluxyMessage *message = (FluxyMessage *) (ptr_offset(offset));
	FLUXY_MESSAGE_SET(message, head, body_len, body);
	return message;
}

FluxyMessage *FluxyMessages::update(FluxyMessage *message, FluxyMessageHead *head, char const *body) {
	size_t body_len = strlen(body);
	size_t old_size = FLUXY_MESSAGE_SIZE(message);
	size_t new_size = FLUXY_MESSAGE_SIZE_CALC(body_len);
	size_t offset = offset_of((char *) message);

	if (!move(message->next(), new_size - old_size))
		return NULL;

	message = (FluxyMessage *) (ptr_offset(offset));
	FLUXY_MESSAGE_SET(message, head, body_len, body);
	return message;
}

FluxyMessage *FluxyMessages::update_body(FluxyMessage *message, char const *body) {
	return update(message, &message->head, body);
}

FluxyMessage *FluxyMessages::unshift(FluxyMessageHead *head, char const *body) {
	return insert((FluxyMessage *) ptr_begin(), head, body);
}

FluxyMessage *FluxyMessages::push(FluxyMessageHead *head, char const *body) {
	return insert((FluxyMessage *) ptr_end(), head, body);
}

bool FluxyMessages::del(FluxyMessage *message) {
	size_t size = FLUXY_MESSAGE_SIZE(message);
	return reduce(message + size, size);
}

void FluxyMessages::del_before(time_t const t) {
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		time_t date = it->head.get_date();
		if (date < t) {
			truncate(it);
			return;
		}
	}
}

void FluxyMessages::del_before(time_t const t, int const max_size) {
	int i = 0;
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		if (i >= max_size or it->head.get_date() < t) {
			truncate(it);
			return;
		}
		i++;
	}
}

int FluxyMessages::count() {
	int i = 0;
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		i++;
	}
	return i;
}

bool FluxyMessages::check() {
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		if ((char *) it > ptr_end())
			return false;
	}
	return true;
}

FluxyMessage *FluxyMessages::num(int n) {
	int i = 0;
	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		if (i == n)
			return it;
		i++;
	}
	return NULL;
}

void FluxyMessages::debug() {
	printf("== MESSAGES ==\n");
	printf("count: %i\n", count());

	FluxyMessage *it;
	FLUXY_MESSAGES_FOREACH(this, it) {
		it->debug();
	}
}

