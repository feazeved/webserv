/* (IMPORTANT) This function presumes 32 byte padding
This function performs a 32 byte load of a field delimited by : then compares
against a table of reference strings to find a match. Because MEMCMP length is
fixed, the compiler automatically vectorizes the comparison

Returns: 0 on no matches, -1 on errors or
		index associated with the string compared

TODO:	Finding can be two operations, Setting or can be one operation
		Move table to init, Automate the creation of the enums from the table
*/
static inline
isize s_match_field(char* &ptr, char* end) {
	static const char fieldTable[][32] = 
	{"status", "location", "transfer-encoding", "content-length"};	
	static const usize fieldCount = ARRAY_SIZE(fieldTable);
	char *optr = ptr;

	while (ptr < end && *ptr != ':')
		ptr++;
	usize length = (usize)(ptr - optr);
	if (length >= 32 || *ptr != ':')
		return (*ptr != ':') ? -1 : 0;
	ptr++;
	char buffer[64];
	MEMCPY_INLINE(buffer, optr, 32);
	for (usize i = 0; i < 32; i++)
		buffer[i] |= 32;
	MEMSET_INLINE(buffer + length, 0, 32);

	for (usize i = 0; i < fieldCount; i++) {
		if (MEMCMP(fieldTable[i], buffer, 32) == 0)
			return (isize)i + 1;
	}
	return 0;
}
