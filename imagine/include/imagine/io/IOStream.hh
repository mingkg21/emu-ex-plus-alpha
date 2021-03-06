#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/io/IO.hh>
#include <cstdio>
#include <cerrno>

template <class IO>
class IOStream
{
public:
	constexpr IOStream() {}

	IOStream(IO io, const char *opentype)
	{
		this->io = std::move(io);
		#if defined __ANDROID__ || __APPLE__
		f = funopen(this,
			[](void *cookie, char *buf, int size)
			{
				auto &s = *(IOStream*)cookie;
				return (int)s.io.read(buf, size);
			},
			[](void *cookie, const char *buf, int size)
			{
				auto &s = *(IOStream*)cookie;
				return (int)s.io.write(buf, size);
			},
			[](void *cookie, fpos_t offset, int whence)
			{
				auto &s = *(IOStream*)cookie;
				return (fpos_t)s.io.seek(offset, (IODefs::SeekMode)whence);
			},
			[](void *cookie)
			{
				auto &s = *(IOStream*)cookie;
				s.io.close();
				s.f = nullptr;
				return 0;
			});
		#else
		cookie_io_functions_t funcs{};
		funcs.read =
			[](void *cookie, char *buf, size_t size)
			{
				auto &s = *(IOStream*)cookie;
				return (ssize_t)s.io.read(buf, size);
			};
		funcs.write =
			[](void *cookie, const char *buf, size_t size)
			{
				auto &s = *(IOStream*)cookie;
				auto bytesWritten = s.io.write(buf, size);
				if(bytesWritten == -1)
				{
					bytesWritten = 0; // needs to return 0 for error
				}
				return (ssize_t)bytesWritten;
			};
		funcs.seek =
			[](void *cookie, off64_t *position, int whence)
			{
				auto &s = *(IOStream*)cookie;
				auto newPos = s.io.seek(*position, (IODefs::SeekMode)whence);
				if(newPos == -1)
				{
					return -1;
				}
				*position = newPos;
				return 0;
			};
		funcs.close =
			[](void *cookie)
			{
				auto &s = *(IOStream*)cookie;
				s.io.close();
				s.f = nullptr;
				return 0;
			};
		f = fopencookie(this, opentype, funcs);
		#endif
		assert(f);
	}

	~IOStream()
	{
		deinit();
	}

	IOStream(IOStream &&o)
	{
		*this = std::move(o);
	}

	IOStream &operator=(IOStream &&o)
	{
		deinit();
		io = std::move(o.io);
		f = std::exchange(o.f, {});
		return *this;
	}

	explicit operator FILE*()
	{
		return f;
	}

	explicit operator bool() const
	{
		return f;
	}

protected:
	IO io{};
	FILE *f{};

	void deinit()
	{
		if(f)
		{
			fclose(f);
		}
	}
};
