#ifndef _FLUXY_BUFFER_HH
#define _FLUXY_BUFFER_HH

#include <stdbool.h>
#include <stdio.h>

#include "scale.hh"

struct FluxyBufferHeader {
	FluxyScale scale;
	size_t size;
} __attribute__ ((packed));

struct FluxyBuffer {
private:
	char *data;

public:
	void clear();
	bool empty() const;
	void set_scale(FluxyScale const scale);
	FluxyScale get_scale() const;
	size_t get_capacity() const;
	bool set_size(size_t size);
	size_t get_size() const;
	void debug() const;
	bool memcpy(void *data, size_t size);
	char *ptr_begin() const;
	char *ptr_end() const;
	char *ptr_offset(size_t const offset) const;
	size_t offset_of(char *ptr) const;

	void fwrite(FILE *f) const;
	bool fread(FILE *f);

	FluxyBuffer();
	~FluxyBuffer();
};

#define FLUXY_BUFFER_GET_SCALE(data) (data == NULL ? 0 : ((FluxyBufferHeader *) data)->scale)
#define FLUXY_BUFFER_SET_SCALE(data, value) if (data) ((FluxyBufferHeader *) data)->scale = value;
#define FLUXY_BUFFER_GET_SIZE(data) (data == NULL ? 0 : ((FluxyBufferHeader *) data)->size)
#define FLUXY_BUFFER_SET_SIZE(data, value) if (data) ((FluxyBufferHeader *) data)->size = value;

#endif
