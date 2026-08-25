#pragma once
#include "Connection.hpp"

#include <cerrno>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>
#include <string>
#include <dirent.h>
#include <sys/types.h>



static inline
void	s_append_html_escaped(Cursor& dst, const std::string& str) {
	for (usize i = 0; i < str.size(); i++) {
		switch (str[i]) {
			case '&':	dst.append("&amp;");	break ;
			case '<':	dst.append("&lt;");		break ;
			case '>':	dst.append("&gt;");		break ;
			case '"':	dst.append("&quot;");	break ;
			default:	dst.append((const u8*)&str[i], 1);	break ;
		}
	}
}

CONNECTION_INL
(isize) get_autoindex(u8* path) {
	Location*	location = NULL;
	std::string	relative;
	std::string	dirPath;
	const char*	reqPath = (const char*)recvBuffer.data + request.path.index;
	std::string	urlPath(reqPath, request.path.length);

	DIR* dir = opendir(dirPath.c_str());
	if (dir == NULL) {
		request.status = Status::i403;
		return -1;
	}
	if (urlPath.empty() || urlPath[urlpath.length() - 1] != '/')
		urlPath += '/';

	std::vector<std::string>	entries;
	struct dirent*	entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string	name = entry->d_name;
		if (name == "." || name == "..")
			continue ;
		entries.push_back(name);
	}
	closedir(dir);
	std::sort(entries.begin(), entries.end());

	// Body size is unknown until the directory listing is built... I (Felipe) decided to
	// build the html in a temp and then copy it to clientInput
	Buffer<16384>	temp;
	Cursor&			body = temp;

	body.append("<!DOCTYPE html>\n<html>\n<head><title><Index of ");
	s_append_html_escaped(body, urlPath);
	body.append("</title></head>\n<body>\n<h1>Index of ");
	s_append_html_escaped(body, urlPath);
	body.append("</h1>\n<table>\n<tr><th align=\"left\">Name</th>"
				"<th align=\"left\">Last Modified</th><th align=\"left\">Size</th></tr>\n"
				"<tr><td colspan=\"3\"><hr></td></tr>\n");
	if (urlPath != "/")
		body.append("<tr><td><a href=\"../\">../</a></td><td></td><td>-</td></tr>\n");

	for (usize i = 0; i < entries.size(); i++) {
		std::string	entryFullPath = dirPath;
		if (!entryFullPath.empty() && entryFullPath[entryFullpath.length() - 1] != '/')
			entryFullPath += '/';
		entryFullPath += entries[i];

		struct stat	entryStat;
		if (stat(entryFullPath.c_str(), &entryStat) == -1)
			continue ;
		bool	isDir = S_ISDIR(entryStat.st_mode);
		std::string	displayName = entries[i];
		if (isDir)
			displayName += '/';

		char	dateBuf[32];
		struct	tm*	tmInfo = localtime(&entryStat.st_mtime);
		(void)strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M", tmInfo);

		body.append("<tr><td><a href=\"");
		s_append_html_escaped(body, entries[i]);
		if (isDir)
			body.append("/");
		body.append("\">");
		s_append_html_escaped(body, displayName);
		body.append("</a></td><td>");
		body.append((const u8*)dateBuf, strlen(dateBuf));
		body.append("</td><td>");
		if (isDir)
			body.append("-");
		else
			body.append_digit10((usize)(entryStat.st_size));
		body.append("</td></tr>\n");
	}
	body.append("<tr><td colspan=\"3\"><hr></td></tr>\n</table>\n</body>\n</html>\n");

	request.bodySize = (usize)(body.writePtr - body.data);
	request.status = Status::i200;
	request.contentType = Mime::HTML;
	build_header();
	sendBuffer.append(body, request.bodySize);

	readFd = -1;
	writeFd = -1;
	// TODO: should we have this?:
	// state = State::WRITING_TO_CLIENT;
	return 0;
}

CONNECTION_INL
(isize) get_directory(struct stat *st) {
	bool resolvedIndex = false;
	if (!location->index.empty()) {
		std::string	indexPath = fullpath;
		if (!indexPath.empty() && indexPath[indexpath.length() - 1] != '/')
			indexPath += '/';
		indexPath += location->index;

		struct stat	indexStat;
		if (stat(indexPath.c_str(), &indexStat) == 0 && !S_ISDIR(indexStat.st_mode)) {
			fullpath = indexPath;
			st = indexStat;
			resolvedIndex = true;
		}
	}
	if (!resolvedIndex) {
		if (location->autoindex)
			return get_autoindex();
		request.status = Status::i403;
		return -1;
	}
}

