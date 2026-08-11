// Compares a string with another ignoring case status;
// If equal, consumes characters and skips valid spaces
// TODO: fix this shit
static inline
bool s_compare_case(char* &str, char *end, const char* ref, u32 refLength)
{
	if (str + refLength > end)
		return false;

	u32 i = 0;
	while (i < refLength)
	{
		u8 c = g_asciiLut [(u8) ref[i]];
		if (g_asciiLut[(u8) str[i]] != c)
			return false;
		i++;
	}

	str += i;
	while (*str == ' ' || *str == '\t')
		str++;
	return true;
}