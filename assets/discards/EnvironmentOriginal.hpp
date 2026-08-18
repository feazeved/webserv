#include "core.hpp"

namespace HTTP {

// Needed for cookies and queries
class Environment {
private:
	static const usize envSize = 4ul * 1024;
	static const usize bufferSize = 96ul * 1024 - sizeof(usize) * 2;

public:
	u8 buffer[bufferSize];
	char *envp[envSize];
	usize optr, bufIndex;

	bool append(char *str, usize length) {
		char *end = str + length - 1;	// The last null terminator is not important
		if (length / 2 <= (envSize - optr)) {	// FAST PATH, should be guaranteed
			envp[optr] = str++;
			while (str < end) {
				if (*str == 0)
					envp[++optr] = ++str;	// Might seem like a bug to not have an else here, but its intended to guarantee min length of 2
				str++;							// &&& inputs become \0\0\0, 
			}
			return true;
		}
		else if (optr < envSize) {
			envp[optr] = str++;
			while (str < end && optr < envSize) {
				if (*str == 0)
					envp[++optr] = ++str;
				str++;
			}
			return true;
		}
		return false;
	}

	Environment(const char **envpSrc) : 
		buffer(), envp(), optr(0), bufIndex(0) {
		usize length;
		const char *str;

		for (; optr < envSize / 2; optr++) {
			str = envpSrc[optr];
			if (str == NULL)
				break;
			length = STRLEN(str) + 1;
			if (bufIndex + length > sizeof(buffer) / 2)
				break;
			MEMCPY(buffer + bufIndex, str, length);
			bufIndex += length;
		}
	}

private:
	Environment();
};
}
