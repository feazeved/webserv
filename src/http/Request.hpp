#include "core.hpp"
#include "Buffer.hpp"
#include <cstdlib>
#include <cstring>

namespace HTTP {

// Class has a read call that consumes lines
class Request
{
private:
	Request();

public:
	Buffer buffer;
	u8	type;	// bitfield: (CHUNKED) () () () (HTTP VERSION) (DELETE) (POST) (GET)
	u32	readOffset, curOffset, writeOffset, line_count;
	u32 requestSize;
	i32	fd;

// Methods
isize read(usize bytes)
{
	isize bytesRead = buffer.read(fd, bytes);	// TODO: check for errors
	const char *str = (const char*) buffer.data;

	if (bytesRead < 0)
		return (bytesRead);
	writeOffset += (usize) bytesRead;
	while (curOffset < writeOffset - 1)
	{
		if (str[curOffset] == '\r' && str[curOffset + 1] == '\n')
		{
			parseLine(curOffset - readOffset);	// TODO: check for errors
			curOffset += 2;
			readOffset = curOffset;
			line_count++;
		}
	}
	return (bytesRead);
}

// Split path/query at the first ?.
// Reject whitespace and control characters.
// Reject malformed encodings such as %, %2, or %GG.
// Reject decoded NUL bytes such as %00.
// Normalize or reject . and .. before filesystem access.
// Ensure the final resolved filesystem path remains inside the configured root.
// Do not blindly convert %2F into /; that can change path structure.
// Apply a request-target length limit and return 414 URI Too Long when exceeded
i32 parseArg(const char *str, const char *end);	// Needs to validate the target based on above reqs
i32 parseHost(const char *str, const char *end); // Host doesnt need to be stored if its resolved immediately

i32 parseFirstLine(usize length)
{
	const char *str = (const char*) buffer.data;
	const char *end = str + length;

	if (length < 14)
		return -1;	// Bad request "GET / HTTP/1.0" shortest possible
	if (std::memcmp(str, "GET ", 4) == 0)
	{
		type |= 1;	// TODO: create enum
		str += 4;
	}
	else if (std::memcmp(str, "POST ", 5) == 0)
	{
		type |= 2;	// TODO: create enum
		str += 5;
	}
	else if (std::memcmp(str, "DELETE ", 7) == 0)
	{
		type |= 4;	// TODO: create enum
		str += 7;
	}
	else
		return -1;	// Invalid method

	const char *arg = str;
	while(str < end && *str != ' ')
		str++;
	i32 rvalue = parseArg(arg, str);
	if (rvalue < 0)
		return rvalue;

	if (str + 9 == end 
		&& std::memcmp(str, " HTTP/1.", 8) == 0 
		&& (str[8] == '0' || str[8] == '1'))
	{
		type |= (str[8] - '0') << 3;	// TODO: get proper enum bitfield representation
	}
	else
		return -1; // Invalid version, TODO: what happens to the class once it is recognized as bad?
	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

i32 parseLine(usize length)
{
	if (line_count == 0)
		return parseFirstLine(length);

	char *str = (char*) buffer.data + readOffset;
	char *end = str + length;

	while (str < end && *str != ':')
	{
		if (*str >= 'A' && *str <= 'Z')
			*str += 32;
		str++;
	}

	if (std::memcmp(str, "host:", 5) == 0)
	{
		str += 5;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;
		// Insert check here to see if host was already resolved
		return (parseHost(str, end));
	}
	else if (std::memcmp(str, "content-length:", 15) == 0)	// needs length checks, or could pad
	{
		if (type >= 128 || requestSize != UINT32_MAX)
			return -1; // bad request, transfer method had already been set
		str += 15;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;

		usize numberLength = 0;
		u32 value = 0;
		while (str[numberLength] >= '0' && str[numberLength] <= '9')
		{
			value = value * 10 + (u32)(str[numberLength] - '0');
			numberLength++;
		}
		if (numberLength == 0 || numberLength > 9)
			return -1;	// Request is too large or invalid
		str += numberLength;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;
		if (str != end)
			return -1;	// Garbage after request
	}
	else if (std::memcmp(str, "transfer-encoding:", 18) == 0)	// TODO: what if its empty?
	{
		if (type >= 128 || requestSize != UINT32_MAX)
			return -1; // bad request, transfer method had already been set
		str += 18;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;

		char *ostr = str;
		while ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z'))
			*str++ |= 32;
		if (std::memcmp(ostr, "chunked", 7) != 0)
			return -1; // bad request, transfer encoding isnt chunked
		str = ostr + 7;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;
		if (str != end)
			return -1; // bad request, garbage after field value
		type |= 128;	// TODO: get proper enum for bitfield
	}
	return 0;
}

// Constructors
Request(i32 fd) : buffer(), type(0), readOffset(0), curOffset(0), writeOffset(0), line_count(0), requestSize(UINT32_MAX), fd(fd)
{

}
};
}

