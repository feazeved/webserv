#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Status::Code) match_location() {
	ArrayView<Location> &locations = cfg->locations;
	usize matchLength = 0;

	for (usize i = 0; i < locations.count; i++) {
		Span srcUri = locations[i].get_uri();
		if (srcUri.size <= matchLength || srcUri.size > req.target.size)	// TODO: Review and write what it is suppsoed to do
			continue;
		if (MEMCMP(req.target.ptr, srcUri.ptr, srcUri.size) != 0)
			continue;
		if (srcUri.size != req.target.size && srcUri.ptr[srcUri.size - 1] != '/' && req.target.ptr[srcUri.size] != '/')
			continue;
		matchLength = srcUri.size;
		req.location = &locations[i];
	}
	if (req.location == NULL)
		return Status::i404;
	if (!((req.location->methods & (options & 7))))
		return Status::i405;
	return Status::unset;
}

// REVIEW: CGI cant fail?
CONNECTION_INL
(Span) check_cgi() {
	char* cgiEnd = req.cgi.end();
	char* targetEnd = req.target.end();
	u16 lengths[2];
	Span result = {};

	req.targetExt.ptr = targetEnd;
	req.targetName.ptr[-1] = '.';
	while (req.targetExt[-1] != '.')
		req.targetExt.ptr--;
	req.targetName.ptr[-1] = '/';
	if (req.targetExt.ptr == req.targetName.ptr)
		return result;
	req.targetExt.ptr--;
	req.targetExt.size = (usize)(targetEnd - req.targetExt.ptr);

	while (req.cgi.ptr < cgiEnd) {
		MEMCPY_INLINE(lengths, req.cgi.ptr, sizeof(lengths));
		const char* ext = req.cgi.ptr + sizeof(lengths);
		if (req.targetExt.size == lengths[0] && MEMCMP(ext, req.targetExt.ptr, req.targetExt.size) == 0) {
			result.ptr = req.cgi.ptr + sizeof(lengths) + lengths[0];
			result.size = (u16)(lengths[1]);
			options |= Options::CGI;
			return result;
		}
		req.cgi.ptr += sizeof(lengths) + lengths[0] + lengths[1];
	}
	return result;
}

CONNECTION_INL
(Status::Code) validate_target(char* str, char* end) {
	char* writePtr = str;
	while (str < end) {
		u8 value = (u8)*str;
		if (*str == '%') {
			if (g_asciiLut[(u8)str[1]] > ASCII_HEX)
				return Status::i400;
			if (g_asciiLut[(u8)str[2]] > ASCII_HEX)
				return Status::i400;
			value = (u8)(g_asciiLut[(u8)str[1]] * 16 + g_asciiLut[(u8)str[2]]);
			if (value == 0)
				return Status::i400;
			str += 2;
		}
		else if (g_asciiLut[value] > ASCII_RFC_SYMBOLS)
			return Status::i400;
		*writePtr++ = (char)value;
		str++;
	}
	req.target.size = (usize)(writePtr - req.target.ptr);
	*writePtr = '\0';
	char* ptr = req.target.ptr;
	if (ptr[1] == '/' || (ptr[1] == '.' && ptr[2] == '/'))
		return Status::i400;
	for (char* ptr = req.target.ptr; ptr < writePtr; ptr++) {
		if (LITCMP(ptr, "/../") == 0 || LITCMP(ptr, "/..\0") == 0)
			return Status::i400;
	}
	return match_location();
}

CONNECTION_INL
(Status::Code) parse_validate(char* str, char* end) {
	const usize targetLength = (usize)(end - str);

	if (*str != '/')	// /images/cats/meow.jpg?FILTER=yes,ORDER=ascending\0
		return Status::i400;
	char* queryPtr = (char*) MEMCHR(str, '?', targetLength);
	char* targetEnd = queryPtr == NULL ? end : queryPtr;
	char* queryStart = queryPtr == NULL ? end : queryPtr + 1;
	req.target = Span::create(str, (usize)(targetEnd - str));		// /images/cats/meow.jpg
	Status::Code code = validate_target(req.target, targetEnd);
	if (code != Status::unset)
		return code;
	targetEnd = req.target.end();
	req.query = Span::create(queryStart, (usize)(end - queryStart));	// FILTER=yes,ORDER=ascending\0
	req.targetName = Span::create(targetEnd, 0);						// /meow.jpg?FILTER=yes,ORDER=ascending\0
	while (req.targetName.ptr[-1] != '/')								// meow.jpg
		req.targetName.ptr--;
	req.targetName.size = (usize)(targetEnd - req.targetName.ptr);
	req.uri = req.location->get_uri();
	req.cgi = req.location->get_cgi_block();
	*targetEnd = '\0';
	*end = '\0';
	req.relativeTarget.ptr = req.target.ptr + req.uri.size;			// /images/cats/meow.jpg
	req.relativeTarget.size = req.target.size - req.uri.size;		// cats/meow.jpg
	req.interpreter = check_cgi();
	return Status::unset;
}
