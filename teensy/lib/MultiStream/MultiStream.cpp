#include "MultiStream.h"

size_t MultiStream::print(const Printable &obj)
{
    obj.printTo(*_stream1);
    return obj.printTo(*_stream2);
}

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

size_t MultiStream::println(const Printable &obj)
{
    obj.printTo(*_stream1) + _stream1->println();
    return obj.printTo(*_stream2) + _stream2->println();
}
