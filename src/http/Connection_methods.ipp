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
#include <string>

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

	// There is no location. Error 404
	if (!s_resolve_location(cfg, reqPath, vars.path.size, &location, relative)) {
		status = 404;
		return error_path();
	}
	s_join_path(location->root, relative, fullpath);

	struct stats st;
	if (stat(fullpath.c_str(), &st) == -1) {
		status = (errno == ENOENT) ? 404 : (errno == EACCES ? 403 : 500);
		return error_path();
	}

	if (S_ISDIR(st.st_mode)) {
		if (location->index.empty()) {
			status = 403;
			return error_path();
		}
		std::string	indexPath = fullpath;
		if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
			indexPath += '/';
		indexPath += location->index;

		if (stat(indexPath.c_str(), &st) == -1) {
			status = (errno == ENOENT) ? 404 : 500;
			return error_path();
		}
		if (S_ISDIR(st.st_mode)) {	// Index is directory (maybe should be parsing error?)
			status = 500;
			return error_path();
			fullpath = indexPath;
		}
	}

	i32	rawFd = open(fullpath.c_str(), O_RDONLY);
	if (rawFd == -1) {
		status = (errno == ENOENT) ? 404 : (errno == EACCES ? 403 : 500);
		return error_path();
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
	s_append_cstr(clientInput, "\r\n");

	return 0;
}

template <usize bufferSize> inline
isize Connection<bufferSize>::del_first_run() {
	const char*	reqPath = (const char*)clientOutput.data + vars.path.index;
	Location*	location = NULL;
	std::string	relative;
	std::string	fullPath;

	// There is no location. Error 404
	if (!s_resolve_location(cfg, reqPath, vars.path.size, &location, relative)) {
		status = 404;
		return error_path();
	}
	s_join_path(location->root, relative, fullPath);

	struct stat	st;
	if (stat(fullPath.c_str(), &st) == -1) {
		status = (errno == ENOENT) ? 404 : 500;
		return error_path();
	}
	if (S_ISDIR(st.st_mode)) {	// I think we shouldnt delete directories
		status = 403;
		return error_path();
	}
	if (unlink(fullPath.c_str()) == -1) {	// Maybe use std::remove bcs I dont see unlink in subject
		status = (errno == EACCES || errno == EPERM) ? 403 : 500;
		return error_path();
	}

	fd.readEnd = -1;	// probably unnecessary
	fd.writeEnd = -1;

	status = 204;
	s_append_status_line(clientInput, 204, "No Content");
	s_append_cstr(clientInput, "\r\n");

	return 0;
}

template <usize bufferSize> inline
isize Connection<bufferSize>::post_first_run() {
	const char*	reqPath = (const char*)clientOutput.data + vars.path.index;
	Location*	location = NULL;
	std::string	relative;

	if (s_resolve_location(cfg, reqPath, vars.path.size, &location, relative)) {
		status = 404;
		return error_path();
	}

	if (relative.empty() || relative[relative.size() - 1] == '/') {
		status = 400;
		return error_path();
	}

	const std::string&	uploadDir = location->upload_store.empty() ? location->root : location->upload_store;
	if (uploadDir.empty()) {
		status = 500;
		return error_path();
	}

	std::string	destPath;
	s_join_path(uploadDir, relative, destPath);

	int	rawFd = open(destPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (rawFd == -1) {
		if (errno == EEXIST)
			status = 409;
		else if (errno == EACCES)
			status = 403;
		else
			status = 500;
		return error_path();
	}

	if (s_set_noblock(rawFd) == false) {
		close(rawFd);
		status = 500;
		return error_path();
	}

	fd.writeEnd = rawFd;
	fd.readEnd = -1;

	return 0;
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
	if (!status.is_set() && fd.writeEnd == -1) {
		status = Status::i201;
		s_append_status_line(clientInput, 201, "Created");	// TODO: append for statuses, order matters
		s_append_cstr(clientInput, "\r\n");
		// set status
		// build header
	}
	return write_to_client(bytes, events);
}
}
