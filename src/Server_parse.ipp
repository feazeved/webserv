#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "Server.hpp"

namespace HTTP {
//
SERVER_INL
(usize) find_scope_end(const Array32<Token> &tokens, usize begin, usize end) {
	usize it = begin;
	bool startedCount = false;
	int braces = 0;
	usize distance = 0;

	while (it != end) {
		if (tokens[it].type == Token::OPEN_BRACKET) {
			startedCount = true;
			braces++;
		}
		else if (tokens[it].type == Token::CLOSE_BRACKET) {
			startedCount = true;
			braces--;
		}
		if (!braces && startedCount)
			break;
		it++;
		distance++;
	}
	if (it == end)
		PERR_EXIT(cleanup(), "Error: Invalid block");
	return distance;
}

SERVER_INL
(usize) count_locations(const Array32<Token> &tokens, usize cursor, usize end) {
	usize locationCount = 0;
	usize locationSize;

	while (cursor < end) {
		if (tokens[cursor].value == "location") {
			usize distance = find_scope_end(tokens, cursor, end);
			locationSize = tokens[cursor + distance].value.offset - tokens[cursor].value.offset + 1;
			if (locationSize > MAX_LOCATION_BLOCK_SIZE)
				PERR_EXIT(cleanup(), "Error: Location block exceeds 64 KiB");
			locationCount++;
			cursor += distance + 1;
		}
		else {
			while (cursor < end && tokens[cursor].type != Token::SEMICOLON)
				cursor++;
			if (cursor < end)
				cursor++;
		}
	}
	return locationCount;
}

SERVER_INL
(isize) parse_directive(const Array32<Token> &tokens, usize &cursor, usize end, Directive &dir) {
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	dir.name = tokens[cursor].value;
	cursor++;
	usize argumentStart = cursor;
	while (cursor != end && tokens[cursor].type == Token::WORD)
		cursor++;
	if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	if (dir.args.alloc((u32)(cursor - argumentStart)) == true)
		_exit(1);
	for (u32 index = 0; index < dir.args.count; index++)
		dir.args[index] = tokens[argumentStart + index].value;
	return 0;
}

SERVER_INL
(isize) set_location_directive(Directive &dir, HTTP::Location &location) {
	if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid root");
		location.root = dir.args[0];
	}
	else if (dir.name == "autoindex") {
		if (dir.args.count != 1 || (!(dir.args[0] == "on") && !(dir.args[0] == "off")))
			PERR_EXIT(cleanup(), "Error: Invalid autoindex");
		location.autoindex = dir.args[0] == "on" ? true : false;
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.count == 0)
			PERR_EXIT(cleanup(), "Error: No allowed methods defined");
		set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid index");
		location.index = dir.args[0];
	}
	else if (dir.name == "upload_store") {
		if (dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid upload store");
		location.uploadStore = dir.args[0];
	}
	else if (dir.name == "return") {
		if (dir.args.count != 2)
			PERR_EXIT(cleanup(), "Error: Invalid redirect");
		usize status = s_strtol10(dir.args[0].get(), 3);
		location.redirectStatus = status;
		if (dir.args[0].length != 3 || status < 300 || status > 399
			|| !location.redirectStatus.is_valid())
			PERR_EXIT(cleanup(), "Error: Invalid redirect status");
		location.redirectTarget = dir.args[1];
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid location directive");
	return 0;
}

SERVER_INL
(isize) parse_cgi(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc) {
	if (cursor == end || tokens[cursor].type != Token::WORD
		|| !(tokens[cursor].value == "cgi"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Invalid CGI block");

	cursor++;
	const usize definitionStart = cursor;
	while (cursor != end && tokens[cursor].type != Token::CLOSE_BRACKET) {
		const usize extensionIndex = cursor;
		const StringView &extension = tokens[cursor].value;
		if (tokens[cursor].type != Token::WORD || extension.length < 2
			|| extension.get()[0] != '.')
			PERR_EXIT(cleanup(), "Error: Invalid CGI extension");
		for (usize index = definitionStart; index < extensionIndex; index += 4) {
			const StringView &previous = tokens[index].value;
			if (previous.length == extension.length
				&& MEMCMP(previous.get(), extension.get(), extension.length) == 0)
				PERR_EXIT(cleanup(), "Error: Duplicate CGI extension");
		}
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::WORD
			|| !(tokens[cursor].value == "="))
			PERR_EXIT(cleanup(), "Error: Expected '=' in CGI definition");
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::WORD)
			PERR_EXIT(cleanup(), "Error: Invalid CGI interpreter");
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
			PERR_EXIT(cleanup(), "Error: Expected ';' after CGI definition");
		cursor++;
	}
	if (cursor == end || tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Invalid CGI block");
	if (cursor != definitionStart) {
		usize blockLength = 2;
		for (usize index = definitionStart; index < cursor; index += 4)
			blockLength += tokens[index].value.length
				+ tokens[index + 2].value.length + 4;
		u32 blockOffset = Arena::alloc_index(blockLength + 1);
		if (blockOffset == UINT32_MAX)
			_exit(1);

		char *writePtr = (char*)Arena::data + blockOffset;
		*writePtr++ = '{';
		for (usize index = definitionStart; index < cursor; index += 4) {
			const StringView extension = tokens[index].value;
			const StringView interpreter = tokens[index + 2].value;
			MEMCPY(writePtr, extension.get(), extension.length);
			writePtr += extension.length;
			MEMCPY_INLINE(writePtr, " = ", 3);
			writePtr += 3;
			MEMCPY(writePtr, interpreter.get(), interpreter.length);
			writePtr += interpreter.length;
			*writePtr++ = ';';
		}
		*writePtr++ = '}';
		*writePtr = '\0';
		loc.cgiBlock = StringView((u32)blockLength, blockOffset);
	}
	return 0;
}

SERVER_INL
(void) set_methods(Array32<StringView> &methods, HTTP::Location &location) {
	for (u32 index = 0; index < methods.count; index++)
	{
		if (methods[index] == "GET")
			location.methods |= HTTP::Mode::GET;
		else if (methods[index] == "POST")
			location.methods |= HTTP::Mode::POST;
		else if (methods[index] == "DELETE")
			location.methods |= HTTP::Mode::DELETE;
		else
			PERR_EXIT(cleanup(), "Error: Invalid method");
	}
}

SERVER_INL
(isize) parse_location(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "location"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Expected location");
	loc.url = tokens[cursor].value;
	if (loc.url.length == 0 || loc.url.get()[0] != '/')
		PERR_EXIT(cleanup(), "Error: Invalid location path");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	const usize locationEnd = find_scope_end(tokens, cursor, end) + cursor;
	cursor++;
	bool cgiDefined = false;
	while (cursor != locationEnd) {
		if (tokens[cursor].type == Token::WORD && tokens[cursor].value == "cgi") {
			if (cgiDefined)
				PERR_EXIT(cleanup(), "Error: Duplicate CGI block");
			cgiDefined = true;
			parse_cgi(tokens, cursor, locationEnd, loc);
		}
		else {
			Directive dir;
			parse_directive(tokens, cursor, locationEnd, dir);
			set_location_directive(dir, loc);
		}
		cursor++;
	}
	if (tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	return 0;
}

SERVER_INL
(isize) set_server_directive(Directive &dir, HTTP::ServerConfig &server) {
	if (dir.name == "listen") {
		if (server.port != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid port definition");
		const StringView &listen = dir.args[0];
		const char *port = listen.get();
		usize portLength = listen.length;
		char *separator = (char*)MEMCHR(port, ':', portLength);
		if (separator != NULL) {
			usize hostLength = (usize)(separator - port);
			if (hostLength == 0 || hostLength == listen.length - 1
				|| server.host.length != 0)
				PERR_EXIT(cleanup(), "Error: Invalid listen address");
			server.host = StringView((u32)hostLength, listen.offset);
			*separator = '\0';
			port = separator + 1;
			portLength -= hostLength + 1;
		}
		server.port = s_strtol10(port, portLength);
		if (server.port < 1 || server.port > 65535)
			PERR_EXIT(cleanup(), "Error: Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host.length != 0 || dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid host definition");
		server.host = dir.args[0];
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol10(dir.args[0].get(), dir.args[0].length);
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(cleanup(), "Error: Invalid max body size");
		server.maxBodySize <<= 20;	// TODO: add M check for size
	}
	else if (dir.name == "error_page") {
		if (dir.args.count < 2)
			PERR_EXIT(cleanup(), "Error: Invalid error page");
		StringView path = dir.args[dir.args.count - 1];
		for (u32 index = 0; index + 1 < dir.args.count; index++) {
			usize error = s_strtol10(dir.args[index].get(), dir.args[index].length);
			const bool validError = (error >= 400 && error <= 431) || (error >= 500 && error <= 511);
			if (dir.args[index].length != 3 || !validError)
				PERR_EXIT(cleanup(), "Error: Invalid error number");
			if (error < 500)
				server.clientErrors[error - 400] = path;
			else
				server.serverErrors[error - 500] = path;
		}
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid server directive");
	return 0;
}

SERVER_INL
(isize) parse_server(const Array32<Token> &tokens, usize cursor, usize end, HTTP::ServerConfig &server) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "server"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type == Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Empty server block");

	usize locationCount = count_locations(tokens, cursor, end);
	if (server.locations.alloc(locationCount) == true)
		_exit(1);

	usize locationIndex = 0;
	while (cursor != end) {
		if (tokens[cursor].value == "location") {
			HTTP::Location loc;
			parse_location(tokens, cursor, end, loc);
			for (usize index = 0; index < locationIndex; index++) {
				const StringView &path = server.locations[index].url;
				if (path.length == loc.url.length && MEMCMP(path.get(), loc.url.get(), path.length) == 0)
					PERR_EXIT(cleanup(), "Error: Duplicate location");
			}
			server.locations[locationIndex] = loc;
			locationIndex++;
		}
		else {
			Directive dir;
			parse_directive(tokens, cursor, end, dir);
			set_server_directive(dir, server);
		}
		cursor++;
	}
	if (tokens[end].type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	if (server.port == SIZE_MAX)
		PERR_EXIT(cleanup(), "Error: Missing listen directive");
	if (server.host.length == 0)
		server.host = StringView(sizeof("localhost") - 1, (u32)(fileOffset + fileSize + 4));
	return 0;
}

SERVER_INL
(void) parse_config(VirtualServer (&servers)[MAX_VIRTUAL_SERVERS]) {
	Array32<Token> tokArray = tokenize();
	usize cursor = 0;
	usize end = tokArray.count;
	usize serverIndex = 0;

	while (cursor != end) {
		if (tokArray[cursor].value == "server") {
			usize distance = find_scope_end(tokArray, cursor, end);
			HTTP::ServerConfig serverConf;
			parse_server(tokArray, cursor, cursor + distance, serverConf);
			servers[serverIndex].cfg = serverConf;
			serverIndex++;
			cursor += distance;
		}
		else
			PERR_EXIT(cleanup(), "Error: Unexpected token");
		cursor++;
	}
}

//
}	// Namespace HTTP
