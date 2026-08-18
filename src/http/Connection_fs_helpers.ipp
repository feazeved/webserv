#pragma once

#include "Connection.hpp"

namespace HTTP {

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
