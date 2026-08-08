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

template <usize bufferSize> inline
isize Connection<bufferSize>::get_first_run() {
	const char*	reqPath = (const char*)clientOutput.data + vars.path.index;
	Location*	location = NULL;
	std::string	relative;
	std::string	fullpath;

	// There is no location. Error 404
	if (!s_resolve_location(cfg, reqPath, vars.path.size, &location, relative)) {
		status = Status::i404;
		return error_path();
	}
	s_join_path(location->root, relative, fullpath);

	struct stats st;
	if (stat(fullpath.c_str(), &st) == -1) {
		status = (errno == ENOENT) ? Status::i404 : (errno == EACCES ? Status::i403 : Status::i500);
		return error_path();
	}

	if (S_ISDIR(st.st_mode)) {
		if (location->index.empty()) {
			status = Status::i403;
			return error_path();
		}
		std::string	indexPath = fullpath;
		if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
			indexPath += '/';
		indexPath += location->index;

		if (stat(indexPath.c_str(), &st) == -1) {
			status = (errno == ENOENT) ? Status::i404 : Status::i500;
			return error_path();
		}
		if (S_ISDIR(st.st_mode)) {	// Index is directory (maybe should be parsing error?)
			status = Status::i500;
			return error_path();
			fullpath = indexPath;
		}
	}

	i32	rawFd = open(fullpath.c_str(), O_RDONLY);
	if (rawFd == -1) {
		status = (errno == ENOENT) ? Status::i404 : (errno == EACCES ? Status::i403 : Status::i500);
		return error_path();
	}

	if (s_set_noblock(rawFd) == false) {
		close(rawFd);
		status = Status::i500;
		return error_path();
	}

	fd.readEnd = rawFd;
	fd.writeEnd = -1;
	status = Status::i200;

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
		status = Status::i404;
		return error_path();
	}
	s_join_path(location->root, relative, fullPath);

	struct stat	st;
	if (stat(fullPath.c_str(), &st) == -1) {
		status = (errno == ENOENT) ? Status::i404 : Status::i500;
		return error_path();
	}
	if (S_ISDIR(st.st_mode)) {	// I think we shouldnt delete directories
		status = Status::i403;
		return error_path();
	}
	if (unlink(fullPath.c_str()) == -1) {	// Maybe use std::remove bcs I dont see unlink in subject
		status = (errno == EACCES || errno == EPERM) ? Status::i403 : Status::i500;
		return error_path();
	}

	fd.readEnd = -1;	// probably unnecessary
	fd.writeEnd = -1;

	status = Status::i204;
	build_header();
	return 0;
}

template <usize bufferSize> inline
isize Connection<bufferSize>::post_first_run() {
	const char*	reqPath = (const char*)clientOutput.data + vars.path.index;
	Location*	location = NULL;
	std::string	relative;

	if (s_resolve_location(cfg, reqPath, vars.path.size, &location, relative)) {
		status = Status::i404;
		return error_path();
	}

	if (relative.empty() || relative[relative.size() - 1] == '/') {
		status = Status::i400;
		return error_path();
	}

	const std::string&	uploadDir = location->upload_store.empty() ? location->root : location->upload_store;
	if (uploadDir.empty()) {
		status = Status::i500;
		return error_path();
	}

	std::string	destPath;
	s_join_path(uploadDir, relative, destPath);

	int	rawFd = open(destPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (rawFd == -1) {
		if (errno == EEXIST)
			status = Status::i409;
		else if (errno == EACCES)
			status = Status::i403;
		else
			status = Status::i500;
		return error_path();
	}

	if (s_set_noblock(rawFd) == false) {
		close(rawFd);
		status = Status::i500;
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
		build_header();
	}
	return write_to_client(bytes, events);
}
}
