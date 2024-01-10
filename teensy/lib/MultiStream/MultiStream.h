#ifndef MULTISTREAM_H
#define MULTISTREAM_H

#include <Stream.h>

class MultiStream : public Stream
{
private:
    Stream *_stream1;
    Stream *_stream2;

public:
    MultiStream(Stream *stream1, Stream *stream2)
    {
        _stream1 = stream1;
        _stream2 = stream2;
    };
    ~MultiStream(){};
    size_t print(const Printable &obj);

    int printf(const char *format, ...);
    int printf(const __FlashStringHelper *format, ...);

    size_t println(const char s[]);
    size_t println(const Printable &obj);

    // Needed overrides to be a Stream/Print sub type.
    int available() { return 0; };
    int read() { return 0; };
    int peek() { return 0; };
    size_t write(uint8_t) { return 0; };
};

#endif