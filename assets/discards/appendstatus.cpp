// template <usize bufferSize> static inline
// void	s_append_status_line(Buffer<bufferSize>& buf, u16 code, const char* reason) {
// 	char	line[16];
// 	i32		len = snprintf(line, sizeof(line), "HTTP/1.1 %u ", (unsigned)code);
// 	buf.append((const u8*)line, (usize)len);
// 	s_append_cstr(buf, reason);
// 	s_append_cstr(buf, "\r\n");
// }

// template <usize bufferSize> static inline
// void	s_append_content_length(Buffer<bufferSize>& buf, usize value) {
// 	char digits[24];
// 	int len = snprintf(digits, sizeof(digits), "%zu", value);
// 	buf.append((const u8*)digits, (usize)len);
// }

template <usize bufferSize> static inline
void	s_append_cstr(Buffer<bufferSize>& buf, const char* str) {
	buf.append((const u8*)str, (usize)strlen(str));
}