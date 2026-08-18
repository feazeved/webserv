#include "core.hpp"

namespace HTTP {

// Needed for cookies and queries
class Environment {
private:
	static const usize envSize = 4096 - 2;		// To align
	static const usize minElements = 64;
	Environment();

public:
	char* envp[envSize];
	char** optr;
	char** writePtr;

	void append(char *ptr) {
		*writePtr = ptr;
		writePtr++;
	}

	void append(u8 *ptr) {
		*writePtr = (char*) ptr;
		writePtr++;
	}

	void reset() {
		writePtr = optr;
		*optr = NULL;
	}

	Environment(char *const *envpSrc) : envp(), optr(envp), writePtr(0) {
		char** endPtr = envp + envSize - minElements;

		while (optr < endPtr && *envpSrc != NULL)
			*optr++ = *envpSrc++;
		*optr = NULL;
		writePtr = optr;
	}
};
}
