#include <ctype.h>
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/stream_mem.h"

int stream_write(struct write_stream * stream, const void * buf, size_t size) {
    return stream->write(stream, buf, size);
}

int stream_do_flush_dummy(struct write_stream * stream) {
    return 0;
}

int stream_write_from_stream(struct write_stream * stream, read_stream_t source) {
    char buffer[4096];
    int writed_sz = 0;
    int read_rv;
    
READ_ONCE:
    read_rv = stream_read(source, buffer, sizeof(buffer));
    if (read_rv < 0) return read_rv;
    
    if (read_rv == 0) return writed_sz;

    int once_writed_sz = 0;
    while(read_rv > 0) {
        int write_rv = stream_write(stream, buffer + once_writed_sz, read_rv - once_writed_sz);
        if (write_rv < 0) return write_rv;
        if (write_rv == 0) return -1;
        once_writed_sz += write_rv;
        writed_sz += write_rv;
        read_rv -= write_rv;
    }

    goto READ_ONCE;
}

int stream_do_write_to_buffer(struct write_stream * input_stream, const void * buf, size_t size) {
    struct write_stream_buffer * stream = (struct write_stream_buffer *)input_stream;

    return (int)mem_buffer_append(stream->m_buffer, buf, size);
}

void write_stream_buffer_init(struct write_stream_buffer * stream, struct mem_buffer * buffer) {
    stream->m_stream.write = stream_do_write_to_buffer;
    stream->m_stream.flush = stream_do_flush_dummy;
    stream->m_buffer = buffer;
}

int stream_do_write_to_mem(struct write_stream * input_stream, const void * buf, size_t size) {
    struct write_stream_mem * stream = (struct write_stream_mem *) input_stream;

    if (size + stream->m_pos > stream->m_capacity) {
        size = stream->m_capacity - stream->m_pos;
    }

    memcpy((char*)stream->m_buffer + stream->m_pos, buf, size);
    stream->m_pos += size;
    return (int)size;
}

void write_stream_mem_init(struct write_stream_mem * stream, void * buf, size_t size) {
    stream->m_stream.write = stream_do_write_to_mem;
    stream->m_stream.flush = stream_do_flush_dummy;
    stream->m_buffer = buf;
    stream->m_capacity = size;
    stream->m_pos = 0;
}

int stream_do_read_from_mem(struct read_stream * input_stream, void * buf, size_t size) {
    struct read_stream_mem * stream = (struct read_stream_mem *) input_stream;

    if (size + stream->m_pos > stream->m_capacity) {
        size = stream->m_capacity - stream->m_pos;
    }

    memcpy(buf, (char*)stream->m_buffer + stream->m_pos, size);
    stream->m_pos += size;
    return (int)size;
}

void read_stream_mem_init(struct read_stream_mem * stream, const void * buf, size_t size) {
    stream->m_stream.read = stream_do_read_from_mem;
    stream->m_buffer = buf;
    stream->m_capacity = size;
    stream->m_pos = 0;
}

int stream_putc_count(struct write_stream * stream, char c, size_t n) {
    char buf[128];
    int rv;

    rv = 0;

    memset(buf, c, 128);
    while(n > 0) {
        size_t curN = n > 128 ? 128 : n;
        int curRv;
        n -= curN;

        curRv = stream->write(stream, buf, curN);
        if (curRv < 0) {
            rv = -1;
            break;
        }

        rv += curRv;
    }

    return rv;
}

int stream_tolower(struct write_stream * stream, const char * data) {
    char buf[128];
    int total;
    int i;

    total = 0;
    do {
        int rv;

        for(i = 0; i < sizeof(buf) && data[total]; ++i, ++total) {
            buf[i] = tolower(data[total]);
        }

        rv = stream_write(stream, buf, i);
        if (rv < 0) return rv;

        if (rv != i) return total - (i - rv);
    } while(data[total]);

    return total;
}

int stream_tolower_len(struct write_stream * stream, const char * data, size_t len) {
    char buf[128];
    int total;
    int i;

    total = 0;
    do {
        int rv;

        for(i = 0; i < sizeof(buf) && total < len; ++i, ++total) {
            buf[i] = tolower(data[total]);
        }

        rv = stream_write(stream, buf, i);
        if (rv < 0) return rv;

        if (rv != i) return total - (i - rv);
    } while(total < len);

    return total;
}

int stream_toupper(struct write_stream * stream, const char * data) {
    char buf[128];
    int total;
    int i;

    total = 0;
    do {
        int rv;

        for(i = 0; i < sizeof(buf) && data[total]; ++i, ++total) {
            buf[i] = toupper(data[total]);
        }

        rv = stream_write(stream, buf, i);
        if (rv < 0) return rv;

        if (rv != i) return total - (i - rv);
    } while(data[total]);

    return total;
}

int stream_toupper_len(struct write_stream * stream, const char * data, size_t len) {
    char buf[128];
    int total;
    int i;

    total = 0;
    do {
        int rv;

        for(i = 0; i < sizeof(buf) && total < len; ++i, ++total) {
            buf[i] = toupper(data[total]);
        }

        rv = stream_write(stream, buf, i);
        if (rv < 0) return rv;

        if (rv != i) return total - (i - rv);
    } while(total < len);

    return total;
}

static int stream_do_write_dummy(struct write_stream * stream, const void * buf, size_t size) { 
    return (int)size;
}

struct write_stream g_write_stream_noop = {
    stream_do_write_dummy
    , stream_do_flush_dummy
};

write_stream_t write_stream_noop = &g_write_stream_noop;

void stream_dump_data(write_stream_t ws, const void * i_ptr, size_t size, char nohex) {
    unsigned char const * ptr = (unsigned char const *)i_ptr;
    size_t i;
    size_t c;

    unsigned int width = 0x10;
 
    if(nohex) {
        /* without the hex output, we can fit more on screen */ 
        width = 0x40;
    }
    
    for(i = 0; i < size; i += width) {
        if (i != 0) stream_printf(ws, "\n");

        stream_printf(ws, "%4.4lx: ", (unsigned long)i);
 
        if(!nohex) {
            /* hex not disabled, show it */ 
            for(c = 0; c < width; c++) {
                if(i + c < size) {
                    stream_printf(ws, "%02x ", ptr[i + c]);
                }
                else {
                    stream_printf(ws, "   ");
                }
            }
        }
 
        for (c = 0; (c < width) && (i + c < size); c++) {
            /* check for 0D0A; if found, skip past and start a new line of output */ 
            if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D && ptr[i + c + 1] == 0x0A) {
                i += (c + 2 - width);
                break;
            }
            stream_printf(ws, "%c", (ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)?ptr[i + c]:'.');
            /* check again for 0D0A, to avoid an extra \n if it's at width */ 
            if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D && ptr[i + c + 2] == 0x0A) {
                i += (c + 3 - width);
                break;
            }
        }
    }
}
 
