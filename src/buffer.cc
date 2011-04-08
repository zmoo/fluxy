#include "buffer.hh"

#include <stdlib.h>
#include <string.h>

#include "recycler.hh"

FluxyBuffer::FluxyBuffer() : data(NULL) {
}

FluxyBuffer::~FluxyBuffer() {
	set_scale(0);
}

void FluxyBuffer::clear() {
	set_scale(0);
}

bool FluxyBuffer::empty() const {
	return data == NULL;
}

void FluxyBuffer::set_scale(FluxyScale const value) {
	FluxyScale old = get_scale();
	if (old == value)
		return;

	if (value == 0) {
//		free(data);
		recycler.free(data, old);
		data = NULL;
	}
	else if (old == 0) {
//		data = (char *) malloc(flux_scale_size(value));
		data = recycler.alloc(value);
	}
	else {
//		data = (char *) realloc(data, flux_scale_size(value));
		data = recycler.realloc(data, old, value);
	}

	FLUXY_BUFFER_SET_SCALE(data, value);
}

FluxyScale FluxyBuffer::get_scale() const {
	return FLUXY_BUFFER_GET_SCALE(data);
}

size_t FluxyBuffer::get_capacity() const {
	return flux_scale_size(get_scale()) - sizeof(FluxyBufferHeader);
}

size_t FluxyBuffer::get_size() const {
	return FLUXY_BUFFER_GET_SIZE(data);
}

char *FluxyBuffer::ptr_begin() const {
	return (data == NULL) ? NULL : data  + sizeof(FluxyBufferHeader);
}

char *FluxyBuffer::ptr_end() const {
	return (data == NULL) ? NULL : ptr_begin() + get_size();
}

char *FluxyBuffer::ptr_offset(size_t const offset) const {
	return (data == NULL) ? NULL : ptr_begin() + offset;
}

size_t FluxyBuffer::offset_of(char *ptr) const {
	return (data == NULL) ? 0 : ptr - ptr_begin();
}

bool FluxyBuffer::set_size(size_t size) {
	FluxyScale scale = flux_scale_find(size + sizeof(FluxyBufferHeader));
	if (scale == FLUXY_SCALE_OVERFLOW)
		return false;

	set_scale(scale);
	FLUXY_BUFFER_SET_SIZE(data, size);
	return true;
}

bool FluxyBuffer::memcpy(void *_data, size_t size) {
	if (!set_size(size))
		return false;

	::memcpy(ptr_begin(), _data, size);
	return true;
}

void FluxyBuffer::fwrite(FILE *f) const {
	size_t size = get_size();
	::fwrite(&size, 1, sizeof(size), f);
	if (size) {
		::fwrite(ptr_begin(), 1, size, f);
	}
}

bool FluxyBuffer::fread(FILE *f) {
	size_t size = 0;
	::fread(&size, 1, sizeof(size), f);
	if (size != 0) {
		if (!set_size(size))
			return false;

		::fread(ptr_begin(), 1, size, f);
	}
	return true;
}

void FluxyBuffer::debug() const {
	printf("== BUFFER ==\n");
	printf("size: %i\n", get_size());
	printf("capacity: %i\n", get_capacity());
	printf("data: ");
	if (data) {
		printf("[");
		char *c;
		for (c = ptr_begin(); c != ptr_end(); c++) {
			printf("%c", (32 < *c and *c < 127) ? *c : '.');
		}
		printf("]");
	}
	else {
		printf("(NULL)");
	}
	printf("\n");
}

