#pragma once
#include "core.hpp"
#include "VirtualServer.hpp"

namespace HTTP {
// Default error header
// <!doctype html><title>404</title><h1>404</h1>

// Considering this is an error, the connection will close. Which means you can override the buffer! hell yeah
// The only memory saving you really need to do is for loading error_pages

VIRTUALSERVER_INL
(bool) cache_error_pages() {

}
}