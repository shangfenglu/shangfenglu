#ifndef __STREAM_UTILS_H
#define __STREAM_UTILS_H

#include <string.h>
#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct _STREAM
{
	int size;
	uint8* p;
	uint8* data;
};

typedef struct _STREAM STREAM;

#define stream_attach(_s, _buf, _size) do { \
	(_s)->size = _size; \
	(_s)->data = _buf; \
	(_s)->p = _buf; } while (0)
#define stream_detach(_s) memset(_s, 0, sizeof(STREAM))
#define stream_clear(_s) memset((_s)->data, 0, (_s)->size)

#define stream_check_size(_s, _n) \
	while (_s->p - _s->data + (_n) > _s->size) \
		stream_extend(_s, _n)

#define stream_get_pos(_s) (_s->p - _s->data)
#define stream_set_pos(_s,_m) _s->p = _s->data + (_m)
#define stream_seek(_s,_offset) _s->p += (_offset)
#define stream_rewind(_s,_offset) _s->p -= (_offset)
#define stream_seal(_s) _s->size = (_s->p - _s->data)
#define stream_get_mark(_s,_mark) _mark = _s->p
#define stream_set_mark(_s,_mark) _s->p = _mark
#define stream_get_head(_s) _s->data
#define stream_get_tail(_s) _s->p
#define stream_get_length(_s) (_s->p - _s->data)
#define stream_get_data(_s) (_s->data)
#define stream_get_size(_s) (_s->size)
#define stream_get_left(_s) (_s->size - (_s->p - _s->data))

#define stream_read_uint8(_s, _v) do { _v = *_s->p++; } while (0)

#define stream_read_uint16(_s, _v) do { _v = \
	(uint16)(*_s->p) + \
	(((uint16)(*(_s->p + 1))) << 8); \
	_s->p += 2; } while (0)

#define stream_read_uint32(_s, _v) do { _v = \
	(uint32)(*_s->p) + \
	(((uint32)(*(_s->p + 1))) << 8) + \
	(((uint32)(*(_s->p + 2))) << 16) + \
	(((uint32)(*(_s->p + 3))) << 24); \
	_s->p += 4; } while (0)

#define stream_read_uint64(_s, _v) do { _v = \
	(uint64)(*_s->p) + \
	(((uint64)(*(_s->p + 1))) << 8) + \
	(((uint64)(*(_s->p + 2))) << 16) + \
	(((uint64)(*(_s->p + 3))) << 24) + \
	(((uint64)(*(_s->p + 4))) << 32) + \
	(((uint64)(*(_s->p + 5))) << 40) + \
	(((uint64)(*(_s->p + 6))) << 48) + \
	(((uint64)(*(_s->p + 7))) << 56); \
	_s->p += 8; } while (0)

#define stream_read(_s, _b, _n) do { \
	memcpy(_b, (_s->p), (_n)); \
	_s->p += (_n); \
	} while (0)

#define stream_write_uint8(_s, _v) do { \
	*_s->p++ = (uint8)(_v); } while (0)

#define stream_write_uint16(_s, _v) do { \
	*_s->p++ = (_v) & 0xFF; \
	*_s->p++ = ((_v) >> 8) & 0xFF; } while (0)

#define stream_write_uint32(_s, _v) do { \
	*_s->p++ = (_v) & 0xFF; \
	*_s->p++ = ((_v) >> 8) & 0xFF; \
	*_s->p++ = ((_v) >> 16) & 0xFF; \
	*_s->p++ = ((_v) >> 24) & 0xFF; } while (0)

#define stream_write_uint64(_s, _v) do { \
	*_s->p++ = (uint64)(_v) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 8) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 16) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 24) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 32) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 40) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 48) & 0xFF; \
	*_s->p++ = ((uint64)(_v) >> 56) & 0xFF; } while (0)

#define stream_write(_s, _b, _n) do { \
	memcpy(_s->p, (_b), (_n)); \
	_s->p += (_n); \
	} while (0)

#define stream_write_zero(_s, _n) do { \
	memset(_s->p, '\0', (_n)); \
	_s->p += (_n); \
	} while (0)

#define stream_set_byte(_s, _v, _n) do { \
	memset(_s->p, _v, (_n)); \
	_s->p += (_n); \
	} while (0)

#define stream_peek_uint8(_s, _v) do { _v = *_s->p; } while (0)

#define stream_peek_uint16(_s, _v) do { _v = \
	(uint16)(*_s->p) + \
	(((uint16)(*(_s->p + 1))) << 8); \
	} while (0)

#define stream_peek_uint32(_s, _v) do { _v = \
	(uint32)(*_s->p) + \
	(((uint32)(*(_s->p + 1))) << 8) + \
	(((uint32)(*(_s->p + 2))) << 16) + \
	(((uint32)(*(_s->p + 3))) << 24); \
	} while (0)
#define stream_peek_uint64(_s, _v) do { _v = \
	(uint64)(*_s->p) + \
	(((uint64)(*(_s->p + 1))) << 8) + \
	(((uint64)(*(_s->p + 2))) << 16) + \
	(((uint64)(*(_s->p + 3))) << 24) + \
	(((uint64)(*(_s->p + 4))) << 32) + \
	(((uint64)(*(_s->p + 5))) << 40) + \
	(((uint64)(*(_s->p + 6))) << 48) + \
	(((uint64)(*(_s->p + 7))) << 56); \
	} while (0)

#define stream_seek_uint8(_s)	stream_seek(_s, 1)
#define stream_seek_uint16(_s)	stream_seek(_s, 2)
#define stream_seek_uint32(_s)	stream_seek(_s, 4)
#define stream_seek_uint64(_s)	stream_seek(_s, 8)

#define stream_read_uint16_be(_s, _v) do { _v = \
	(((uint16)(*_s->p)) << 8) + \
	(uint16)(*(_s->p + 1)); \
	_s->p += 2; } while (0)

#define stream_read_uint32_be(_s, _v) do { _v = \
    (((uint32)(*((_s)->p + 0))) << 24) + \
    (((uint32)(*((_s)->p + 1))) << 16) + \
    (((uint32)(*((_s)->p + 2))) <<  8) + \
    (((uint32)(*((_s)->p + 3))) <<  0);  \
    (_s)->p += 4; } while (0)

#define stream_write_uint16_be(_s, _v) do { \
	*_s->p++ = ((_v) >> 8) & 0xFF; \
	*_s->p++ = (_v) & 0xFF; } while (0)

#define stream_write_uint32_be(_s, _v) do { \
	stream_write_uint16_be(_s, ((_v) >> 16 & 0xFFFF)); \
	stream_write_uint16_be(_s, ((_v) & 0xFFFF)); \
	} while (0)

#define stream_copy(_dst, _src, _n) do { \
	memcpy(_dst->p, _src->p, _n); \
	_dst->p += _n; \
	_src->p += _n; \
	} while (0)

#endif /* __STREAM_UTILS_H */

