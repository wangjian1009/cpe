#include <limits.h>
#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream.h"

#define MAXBUFFER 512

// Flag definitions

#define FL_SIGN       0x00001   // Put plus or minus in front
#define FL_SIGNSP     0x00002   // Put space or minus in front
#define FL_LEFT       0x00004   // Left justify
#define FL_LEADZERO   0x00008   // Pad with leading zeros
#define FL_LONG       0x00010   // Long value given
#define FL_SHORT      0x00020   // Short value given
#define FL_SIGNED     0x00040   // Signed data given
#define FL_ALTERNATE  0x00080   // Alternate form requested
#define FL_NEGATIVE   0x00100   // Value is negative
#define FL_FORCEOCTAL 0x00200   // Force leading '0' for octals

extern char *ecvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf, size_t buf_capacity);
extern char *fcvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf, size_t buf_capacity);

static void cfltcvt(double value, char *buffer, char fmt, int precision, int capexp) {
    int decpt, sign, exp, pos;
    char *digits = NULL;
    char cvtbuf[MAXBUFFER];
    int magnitude;

    if (fmt == 'g') {
        digits = ecvtbuf(value, precision, &decpt, &sign, cvtbuf, sizeof(cvtbuf));
        magnitude = decpt - 1;
        if (magnitude < -4  ||  magnitude > precision - 1) {
            fmt = 'e';
            precision -= 1;
        }
        else {
            fmt = 'f';
            precision -= decpt;
        }
    }

    if (fmt == 'e') {
        digits = ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf, sizeof(cvtbuf));

        if (sign) *buffer++ = '-';
        *buffer++ = *digits;
        if (precision > 0) *buffer++ = '.';
        memcpy(buffer, digits + 1, precision);
        buffer += precision;
        *buffer++ = capexp ? 'E' : 'e';

        if (decpt == 0)
        {
            if (value == 0.0)
                exp = 0;
            else
                exp = -1;
        }
        else
            exp = decpt - 1;

        if (exp < 0)
        {
            *buffer++ = '-';
            exp = -exp;
        }
        else
            *buffer++ = '+';

        buffer[2] = (exp % 10) + '0';
        exp = exp / 10;
        buffer[1] = (exp % 10) + '0';
        exp = exp / 10;
        buffer[0] = (exp % 10) + '0';
        buffer += 3;
    }
    else if (fmt == 'f')
    {
        digits = fcvtbuf(value, precision, &decpt, &sign, cvtbuf, sizeof(cvtbuf));
        if (sign) *buffer++ = '-';
        if (*digits)
        {
            if (decpt <= 0)
            {
                *buffer++ = '0';
                *buffer++ = '.';
                for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
                while (*digits) *buffer++ = *digits++;
            }
            else
            {
                pos = 0;
                while (*digits)
                {
                    if (pos++ == decpt) *buffer++ = '.';
                    *buffer++ = *digits++;
                }
            }
        }
        else
        {
            *buffer++ = '0';
            if (precision > 0)
            {
                *buffer++ = '.';
                for (pos = 0; pos < precision; pos++) *buffer++ = '0';
            }
        }
    }

    *buffer = '\0';
}

static void forcdecpt(char *buffer) {
    while (*buffer) {
        if (*buffer == '.') return;
        if (*buffer == 'e' || *buffer == 'E') break;
        buffer++;
    }

    if (*buffer) {
        int n = (int)strlen(buffer);
        while (n > 0) {
            buffer[n + 1] = buffer[n];
            n--;
        }

        *buffer = '.';
    }
    else {
        *buffer++ = '.';
        *buffer = '\0';
    }
}

static void cropzeros(char *buffer) {
    char *stop;

    while (*buffer && *buffer != '.') buffer++;
    if (*buffer++)
    {
        while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
        stop = buffer--;
        while (*buffer == '0') buffer--;
        if (*buffer == '.') buffer--;
        while ((*++buffer = *stop++));
    }
}

int stream_printf(struct write_stream * stream, const char *format, ...) {
  int rc;
  va_list args;

  va_start(args, format);

  rc = stream_vprintf(stream, format, args);

  va_end(args);

  return rc;
}

int stream_vprintf(struct write_stream * stream, const char *format, va_list args) {
    char buffer[4096];
    int n = vsnprintf(buffer, sizeof(buffer), format, args);
    return stream_write(stream, buffer, n);
}
