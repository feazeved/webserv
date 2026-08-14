
isize match_mime_type(char* &ptr, char* end) {
	static const char ltable[][4] = {
		{'h','t','m','l'},
		{'h','t','m'},
		{'c','s','s'},
		{'j','s','o','n'},
		{'j','s'},
		{'p','n','g'},
		{'j','p','g'},
		{'j','p','e','g'},
		{'g','i','f'},
		{'t','x','t'}
	};

	static const usize matchSize = sizeof(*ltable);
	static const usize matchCount = ARRAY_SIZE(ltable);

	const usize minLength = (usize) MAX(0, end - ptr - 3);
	const usize maxLength = MIN(minLength, 252);
	char buffer[64];

	usize length = 0;
	while (length < maxLength && ptr[length] != '.')
		length++;
	if (length >= maxLength)
		return -1;
	length++;

	MEMCPY_INLINE(buffer, ptr, matchSize);
	for (usize i = 0; i < matchSize; i++)
		buffer[i] |= 32;
	MEMSET_INLINE(buffer + length, 0, matchSize);

	for (usize i = 0; i < matchCount; i++) {
		if (MEMCMP(ltable[i], buffer, matchSize) == 0)
			return (isize)i + 1;
	}
	return 0;
}

static inline
isize s_get_mime_type(const char *dotPos, const char *end) {
	static const char *mimeStrings[] = {"\x09" "text/html", "\x09" "text/html", 
	"\x08" "text/css", "\x10" "application/json", "\x16" "application/javascript",
	"\x09" "image/png", "\x0A" "image/jpeg", "\x0A" "image/jpeg", 
	"\x09" "image/gif", "\x0A" "text/plain", "\x18" "application/octet-stream"};

	isize matchId = match_mime_type(ptr, end);
	cache.append("Content-Type: ");
	cache.appendInline(mimeStrings[matchId] + 1, mimeStrings[matchId][0]);

}

// static inline
// const char* s_get_mime_type(const std::string& path) {
// 	usize	dot_pos = path.find_last_of('.');	// Alex: Ideally this function should be inside parse_target
// 												// Save an enum to the file type inside the class, might be relevant for other stuff
// 	if (dot_pos == std::string::npos)
// 		return "application/octet-stream";
// 	std::string extension = path.substr(dot_pos + 1);
// 	if (extension == "html" || extension == "htm")	return "text/html";
// 	if (extension == "css")                 		return "text/css";
// 	if (extension == "js")                  		return "application/javascript";
// 	if (extension == "json")                 		return "application/json";
// 	if (extension == "png")                  		return "image/png";
// 	if (extension == "jpg" || extension == "jpeg")	return "image/jpeg";
// 	if (extension == "gif")                  		return "image/gif";
// 	if (extension == "txt")                  		return "text/plain";
// 	return "application/octet-stream";
// }

// TODO: parsing should process the data not just store it
// Alex: Lets have this save an enum to extension type. Example:
// enum ContentType : u8 {
// 	Html,
// 	Css,
// 	JavaScript,
// 	Json,
// 	Png,
// 	Jpeg,
// 	Gif,
// 	Text,
// };
static inline
const char* s_get_mime_type(const std::string& path) {
	usize	dot_pos = path.find_last_of('.');	// Alex: Ideally this function should be inside parse_target
												// Save an enum to the file type inside the class, might be relevant for other stuff
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