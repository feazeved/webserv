#pragma once
#include "Connection.hpp"
#include "Connection_helpers.ipp"
#include "HTTP.hpp"
#include "core.hpp"
#include "core_builtins.ipp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <locale>
#include <vector>
#include <cstring>
#include <sys/stat.h>

namespace HTTP {

static inline
bool s_resolve_location(ServerConfig* cfg, const char* reqPath, usize reqPathLen, Location** outLocation, std::string& outRelative) {
	std::vector<Location>::iterator it = cfg->locations.begin();
	for (; it != cfg->locations.end(); it++) {
		usize	locLen = it->path.size();
		if (reqPathLen >= locLen && MEMCMP(reqPath, it->path.c_str(), locLen) == 0) {
			*outLocation = &(*it);
			outRelative.assign(reqPath + locLen, reqPathLen - locLen);
			return true;
		}
	}
	return false;
}

static inline
void	s_join_path(const std::string &base, const std::string &relative, std::string &out) {
	out = base;
	if (!out.empty() && out[out.size() - 1] != '/' && (relative.empty() || relative[0] != '/'))
		out += '/';
	out += relative;
}

static inline
const char* s_get_mime_type(const std::string& path) {
	usize	dot_pos = path.find_last_of('.');

	if (dot_pos == std::string::npos)
		return "application/octet-stream";
	std::string extension = path.substr(dot_pos + 1);
	if (extension == "html" || extension == "htm")	return "text/html";
	if (extension == "css")                 		return "text/css";
	if (extension == "js")                  		return "application/javascript";
	if (extension == "json")                 		return "application/json";
	if (extension == "png")                  		return "image/png";
	if (extension == "jpg" || extension == "jpeg")	return "image/jpeg";
	if (extension == "gif")                  		return "image/gif";
	if (extension == "txt")                  		return "text/plain";
	return "application/octet-stream";
}

template <usize bufferSize> static inline
void	s_append_cstr(Buffer<bufferSize>& buf, const char* str) {
	buf.append((const u8*)str, (usize)strlen(str));
}

template <usize bufferSize> static inline
void	s_append_status_line(Buffer<bufferSize>& buf, u16 code, const char* reason) {
	char	line[16];
	i32		len = snprintf(line, sizeof(line), "HTTP/1.1 %u ", (unsigned)code);
	buf.append((const u8*)line, (usize)len);
	s_append_cstr(buf, reason);
	s_append_cstr(buf, "\r\n");
}

template <usize bufferSize> static inline
void	s_append_content_length(Buffer<bufferSize>& buf, usize value) {
	char digits[24];
	int len = snprintf(digits, sizeof(digits), "%zu", value);
	buf.append((const u8*)digits, (usize)len);
}

template <usize bufferSize> inline
isize Connection<bufferSize>::get_first_run() {
	const char*	reqPath = (const char*)clientOutput.data + vars.path.index;
	Location*	location = NULL;
	std::string	relative;
	std::string	fullpath;

	if (!s_resolve_location(cfg, reqPath, vars.path.size, &location, relative)) {
		status = 404;
		return error_path();
	}
	s_join_path(location->root, relative, fullpath);

	i32	rawFd = open(fullpath.c_str(), O_RDONLY);
	if (rawFd == -1) {
		status = (errno == ENOENT) ? 404 : (errno == EACCES ? 403 : 500);
		return error_path();
	}

	struct stat	st;
	if (fstat(rawFd, &st) == -1) {
		close(rawFd);
		status = 500;
		return error_path();
	}

	if (S_ISDIR(st.st_mode)) {
		close(rawFd);
		if (location->index.empty()) {
			status = 403;
			return error_path();
		}
		std::string indexPath = fullpath;
		if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
			indexPath += '/';
		indexPath += location->index;

		rawFd = open(indexPath.c_str(), O_RDONLY);
		if (rawFd == -1) {
			status = (errno == ENOENT) ? 404 : 500;
			return error_path;
		}
		if (fstat(rawFd, &st) == -1 || S_ISDIR(st.st_mode)) {
			close(rawFd);
			status = 500;
			return error_path();
		}
		fullpath = indexPath;
	}

	if (s_set_noblock(rawFd) == false) {
		close(rawFd);
		status = 500;
		return error_path();
	}

	fd.readEnd = rawFd;
	fd.writeEnd = -1;

	s_append_status_line(clientInput, 200, "OK");
	s_append_cstr(clientInput, "Content-Type: ");
	s_append_cstr(clientInput, s_get_mime_type(fullpath));
	s_append_cstr(clientInput, "\r\n");
	s_append_cstr(clientInput, "Content-Length: ");
	s_append_content_length(clientInput, (usize)st.st_size);
	s_append_cstr(clientInput, "\r\n");

	return 0;
}

template <usize bufferSize> inline
isize Connection<bufferSize>::del_first_run() {
		// Open files
		// Set FDs
}

template <usize bufferSize> inline
isize Connection<bufferSize>::post_first_run() {
		// Open files
		// Set FDs
}

// === DEL ==========================================================================
template <usize bufferSize> inline // Header will already be built in the configure function
isize Connection<bufferSize>::del_method(usize bytes, u32 events) {
	return write_to_client(bytes, events);
}

// === GET ==========================================================================
// Header will already be built in the configure function
template <usize bufferSize>
isize Connection<bufferSize>::get_method(usize bytes, u32 events) {
	isize bytesRead = read_from_server(bytes);
	if (bytesRead < 0)
		return bytesRead;
	return write_to_client(bytes, events);
}

// === POST =========================================================================
template <usize bufferSize> inline
isize Connection<bufferSize>::post_method(usize bytes, u32 events) {
	isize bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return bytesRead;

	isize bytesWritten = write_to_server(bytes);
	if (bytesWritten < 0)
		return bytesWritten;

	// Return path until the operation isnt complete
	if (status == 0 && fd.writeEnd == -1) {
		status = 201;
		// set status
		// build header
	}
	return write_to_client(bytes, events);
}
}
