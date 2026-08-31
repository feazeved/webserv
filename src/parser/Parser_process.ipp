#pragma once
#include "Parser.hpp"

static inline
Span16 s_store_location_span(Location &location, char* &wptr, const Span &source) {
	Span16 result = {(u16)(wptr - (char*)&location.uri), (u16)source.size};
	if (source.size != 0)
		MEMCPY(wptr, source.ptr, source.size);
	wptr += source.size;
	*wptr++ = '\0';
	return result;
}

static inline
void s_store_cgi(char* &wptr, const Parser::ParsedCgi &cgiBlock, Location &location) {
		location.cgiBlock.index = (u16)(wptr - (char*)&location.uri);
		location.cgiBlock.size = (u16)cgiBlock.size;
		for (usize index = 0; index < cgiBlock.definitions.count; index += 4) {
			Parser::Token *definition = cgiBlock.definitions.ptr + index;
			const Span &extension = definition[0].value;
			const Span &interpreter = definition[2].value;
			const u16 lengths[2] = {(u16)extension.size, (u16)interpreter.size};
			MEMCPY_INLINE(wptr, lengths, sizeof(lengths));
			wptr += sizeof(lengths);
			MEMCPY(wptr, extension.ptr, extension.size);
			wptr += extension.size;
			MEMCPY(wptr, interpreter.ptr, interpreter.size);
			wptr += interpreter.size;
		}
		*wptr++ = '\0';	
}

static inline
void s_store_location(char* &wptr, const Parser::ParsedLocation &ploc, Location &location) {
		location.uri = s_store_location_span(location, wptr, ploc.uri);
		location.root = s_store_location_span(location, wptr, ploc.root);
		location.index = s_store_location_span(location, wptr, ploc.index);
		location.uploadStore = s_store_location_span(location, wptr, ploc.uploadStore);
		s_store_cgi(wptr, ploc.cgiBlock, location);
		location.redirectTarget = s_store_location_span(location, wptr, ploc.redirectTarget);
		location.redirectStatus = ploc.redirectStatus;
		location.methods = ploc.methods;
		location.autoindex = ploc.autoindex;	
}

static inline
usize s_location_size(const Parser::ParsedLocation &loc) {
	usize packSize = 6 + loc.uri.size + loc.root.size + loc.index.size;
	packSize += loc.uploadStore.size + loc.cgiBlock.size + loc.redirectTarget.size;
	return packSize;
}

PARSER_INL
(Array<Location>) store_locations(const Array<ParsedLocation> &source) {
	usize allocationSize = source.count * sizeof(Location);
	for (usize index = 0; index < source.count; index++)
		allocationSize += s_location_size(source[index]);
	const u32 allocation = beta.alloc(allocationSize, 0, __alignof__(Location));
	if (allocation == UINT32_MAX)
		std::exit(1);
	Array<Location> locations((Location*)beta.mptr(allocation), source.count);
	char *wptr = (char*)(locations.ptr + locations.count);
	for (usize locationIndex = 0; locationIndex < locations.count; locationIndex++)
		s_store_location(wptr, source[locationIndex], locations[locationIndex]);
	return locations;
}

static inline
void s_build_error_page_path(char *out, const Span &root, const Span &path) {
	usize length = 0;
	if (root.size != 0) {
		MEMCPY(out, root.ptr, root.size);
		length = root.size;
	}

	usize pathOffset = 0;
	if (root.size != 0 && path.size != 0) {
		const bool rootHasSlash = out[length - 1] == '/';
		const bool pathHasSlash = path.ptr[0] == '/';
		if (rootHasSlash && pathHasSlash)
			pathOffset = 1;
		else if (!rootHasSlash && !pathHasSlash)
			out[length++] = '/';
	}

	const usize pathLength = path.size - pathOffset;
	MEMCPY(out + length, path.ptr + pathOffset, pathLength);
	length += pathLength;
	out[length] = '\0';
}

PARSER_INL
(void) cache_error_pages(VirtualServer &server) {
	char pathBuffer[4 * MAX_PATH_SIZE];
	Span configuredPaths[Status::errorPageCount];

	for (usize index = 0; index < Status::errorPageCount; index++)
		configuredPaths[index] = server.errorPages[index];

	for (usize index = 0; index < Status::errorPageCount; index++) {
		Span &page = server.errorPages[index];
		const Span &path = configuredPaths[index];
		if (path.size == 0) {
			page = Status::s_error_str(index);
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const Span &previousPath = configuredPaths[duplicate];
			if (path.size == previousPath.size && previousPath.size != 0
				&& MEMCMP(path.ptr, previousPath.ptr, path.size) == 0)
				break;
		}
		if (duplicate != index) {
			page = server.errorPages[duplicate];
			continue;
		}
		s_build_error_page_path(pathBuffer, server.serverRoot, path);
		if (s_read_whole_file(beta, pathBuffer, page, 0, 0))
			std::exit(1);
	}
}
