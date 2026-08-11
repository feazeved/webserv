#pragma once

#include "Connection.hpp"

namespace HTTP {

static inline
bool	s_resolve_location(ServerConfig* cfg, const char* reqPath, usize reqPathLen, Location** outLocation, std::string& outRelative) {
	std::vector<Location>::iterator	it = cfg->locations.begin();
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
void	s_join_path(const std::string& base, const std::string& relative, std::string& out) {
	out = base;
	if (!out.empty() && out[out.size() - 1] != '/' && (relative.empty() || relative[0] != '/'))
		out += '/';
	out += relative;
}

static inline
void	s_append_number(Cursor& dst, usize number) {
	char	buffer[24];
	usize	length = dst.itoa10(number, buffer + sizeof(buffer) - 1);
	dst.append((const u8*)(buffer + sizeof(buffer) - 1 - length), length);
}

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

}
