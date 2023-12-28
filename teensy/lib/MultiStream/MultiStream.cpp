#include "MultiStream.h"

int MultiStream::printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    _stream1->printf(format, ap);
    int retval = _stream2->printf(format, ap);

    va_end(ap);
    return retval;
}

int MultiStream::printf(const __FlashStringHelper *format, ...)
{
    va_list ap;
    va_start(ap, format);

    _stream1->printf(format, ap);
    int retval = _stream2->printf(format, ap);

    va_end(ap);
    return retval;
}

size_t MultiStream::println(const char s[])
{
    _stream1->println(s);
    return _stream2->println(s);
}