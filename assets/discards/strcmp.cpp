
// Compares and advances pointer if valid
// BUFFER_INL_T
// (usize N, bool) strcmp(const char (&string)[N]) {
// 	const usize strLength = N - 1;
// 	bool isMatch = MEMCMP(data + readPos, string, strLength) == 0;
// 	readPos += isMatch ? strLength : 0;
// 	return isMatch;
// }