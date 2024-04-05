// Copyright (C) 2024 Gaute Hagen
//
// This file is part of Autosteering Firmware.
//
// Autosteering Firmware is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Autosteering Firmware is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Autosteering Firmware.  If not, see <https://www.gnu.org/licenses/>.

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