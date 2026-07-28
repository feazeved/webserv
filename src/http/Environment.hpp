#include "core.hpp"

namespace HTTP {

// Needed for cookies and queries
class Environment {
private:
	static const usize envSize = 4ul * 1024;
	static const usize bufferSize = 32ul * 1024;

public:
	u8 buffer[bufferSize];
	char *envp[envSize];
	usize envIndex, bufIndex;	// save path here

	bool append(char *str, usize length) {
		char *end = str + length - 1;	// The last null terminator is not important
		if (length / 2 <= (envSize - envIndex)) {	// FAST PATH, should be guaranteed
			envp[envIndex] = str++;
			while (str < end) {
				if (*str == 0)
					envp[++envIndex] = ++str;	// Might seem like a bug to not have an else here, but its intended to guarantee min length of 2
				str++;							// &&& inputs become \0\0\0, 
			}
			return true;
		}
		else if (envIndex < envSize) {
			envp[envIndex] = str++;
			while (str < end && envIndex < envSize) {
				if (*str == 0)
					envp[++envIndex] = ++str;
				str++;
			}
			return true;
		}
		return false;
	}

	Environment(const char **envpSrc) : 
		buffer(), envp(), envIndex(0), bufIndex(0) {
		usize length;
		const char *str;

		for (; envIndex < envSize / 2; envIndex++) {
			str = envpSrc[envIndex];
			if (str == NULL)
				break;
			length = STRLEN_BUILTIN(str) + 1;
			if (bufIndex + length > sizeof(buffer) / 2)
				break;
			MEMCPY_BUILTIN(buffer + bufIndex, str, length);
			bufIndex += length;
		}
	}

private:
	Environment();
};
}
