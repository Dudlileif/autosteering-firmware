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
    obj.printTo(*_stream1);
    _stream1->println();
    return obj.printTo(*_stream2) + _stream2->println();
}
