#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "StringView.hpp"

namespace HTTP {

struct Location {
	StringView url;
	StringView root;
	StringView index;			// If this is specified, its a file
	StringView uploadStore;		// Validation: needs to check access
	StringView cgiBlock;		// Non-empty block range, including braces
	StringView redirectTarget;
	Status redirectStatus;
	u8 methods;
	bool autoindex;

	Location() : methods(0), autoindex(false) {}
};
}
