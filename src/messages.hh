#ifndef _FLUXY_MESSAGES
#define _FLUXY_MESSAGES

#include <time.h>
#include <stdint.h>
#include <string>

#include "buffer.hh"

typedef uint8_t FluxyMessageType;
typedef uint32_t FluxyMessageFlags;
typedef uint8_t FluxyMessageVersion;
typedef uint32_t FluxyMessageId;

struct FluxyMessageHead {
public:
	FluxyMessageId id;
	time_t date;
	FluxyMessageType type;
	FluxyMessageVersion version;
	FluxyMessageFlags flags;

public:
	void set_id(FluxyMessageId const value);
	FluxyMessageId get_id() const;
	void set_date(time_t const value);
	time_t get_date() const;
	void set_type(FluxyMessageType const value);
	FluxyMessageType get_type() const;
	void set_version(FluxyMessageVersion const value);
	FluxyMessageVersion get_version() const;
	void set_seen(bool const value);
	bool get_seen() const;
	void set_flags(FluxyMessageFlags const value);
	FluxyMessageFlags get_flags() const;

	FluxyMessageHead();
} __attribute__ ((packed));

struct FluxyMessage {
public:
	FluxyMessageHead head;
	uint16_t body_len;
	char body;

public:
	FluxyMessage *next();
	std::string get_body() const;
	void debug();
} __attribute__ ((packed));

struct FluxyMessages : public FluxyBuffer {
public:
	bool expand(FluxyMessage *from, int delta);
	bool reduce(FluxyMessage *from, int delta);
	bool move(FluxyMessage *from, int delta);
	bool truncate(FluxyMessage *from);

	FluxyMessage *insert(FluxyMessage *from, FluxyMessageHead *head, char const *body);
	FluxyMessage *push(FluxyMessageHead *head, char const *body);
	FluxyMessage *unshift(FluxyMessageHead *head, char const *body);
	FluxyMessage *update(FluxyMessage *message, FluxyMessageHead *head, char const *body);
	FluxyMessage *update_body(FluxyMessage *message, char const *body);

	bool del(FluxyMessage *message);
	void del_before(time_t const t, int const max_size);
	void del_before(time_t const t);
	int count();
	FluxyMessage *num(int n);
	bool check();
	void debug();
};

#define FLUXY_MESSAGE_HEAD_SET_ID(head, value) \
	(head)->id = value; 

#define FLUXY_MESSAGE_HEAD_GET_ID(head) \
	(head)->id

#define FLUXY_MESSAGE_HEAD_SET_DATE(head, value) \
	(head)->date = value;

#define FLUXY_MESSAGE_HEAD_GET_DATE(head) \
	(head)->date

#define FLUXY_MESSAGE_HEAD_SET_TYPE(head, value) \
	(head)->type = value;

#define FLUXY_MESSAGE_HEAD_GET_TYPE(head) \
	(head)->type

#define FLUXY_MESSAGE_HEAD_SET_VERSION(head, value) \
	(head)->version = (((head)->version & 0x80) | (value & 0x7f));

#define FLUXY_MESSAGE_HEAD_GET_VERSION(head) \
	((head)->version & 0x7f)

#define FLUXY_MESSAGE_HEAD_SET_SEEN(head, value) \
	(head)->version = (((head)->version & 0x7f) | (value ? 0x80 : 0));

#define FLUXY_MESSAGE_HEAD_GET_SEEN(head) \
	(((head)->version & 0x80) == 0x80)

#define FLUXY_MESSAGE_HEAD_SET_FLAGS(head, value) \
	(head)->flags = value;

#define FLUXY_MESSAGE_HEAD_GET_FLAGS(head) \
	(head)->flags

#define FLUXY_MESSAGE_HEAD_INIT(head) { \
	(head)->id = 0; \
	(head)->date = 0; \
	(head)->type = 0; \
	(head)->version = 0; \
	(head)->flags = 0; \
}

#define FLUXY_MESSAGE_BODY_LEN(pmessage) (pmessage->body_len)
#define FLUXY_MESSAGE_BODY(pmessage) (pmessage->body)
#define FLUXY_MESSAGE_BODY_STR(pmessage) (std::string(&pmessage->body, pmessage->body_len))

#define FLUXY_MESSAGE_SET(pmessage, phead, body_len, body) { \
	pmessage->head = *phead; \
	pmessage->body_len = body_len; \
	::memcpy(&(pmessage->body), body, body_len); \
}

#define FLUXY_MESSAGE_NEXT(message) (FluxyMessage *) ((char *) message + FLUXY_MESSAGE_SIZE(message))

#define FLUXY_MESSAGE_SIZE_CALC(body_len) (sizeof(FluxyMessageHead) + sizeof(uint16_t) + body_len)
#define FLUXY_MESSAGE_SIZE(message) FLUXY_MESSAGE_SIZE_CALC(message->body_len)

#define FLUXY_MESSAGES_FOREACH2(begin, end, it) for (it = (begin); it != (end); it = FLUXY_MESSAGE_NEXT(it))
#define FLUXY_MESSAGES_FOREACH(messages, it) FLUXY_MESSAGES_FOREACH2((FluxyMessage *) (messages)->ptr_begin(), (FluxyMessage *) (messages)->ptr_end(), it)

#endif

