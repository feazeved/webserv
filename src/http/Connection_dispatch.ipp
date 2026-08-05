#pragma once

#include "Connection.hpp"
#include <cerrno>
#include <sys/socket.h>

template <usize bufferSize>
i32 HTTP::Connection<bufferSize>::dispatch() {
    if (isSSE) {
        if (!sse_buffer.empty()) {
            ssize_t n = send(fd.client, sse_buffer.c_str(), sse_buffer.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
            if (n > 0) {
                sse_buffer.erase(0, n);
                return sse_buffer.empty() ? 1 : 2;
            } else if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                return 0;
            }
            return 2;
        }
        return 1;
    }

    if (!headerParsed) {
        isize r = parse_header(0, 0);
        if (r < 0)
            return 0;
        if (!headerParsed)
            return 1;
    }

    i32 ret = handle_game_request();
    if (ret >= 0)
        return ret;

    return 0;
}
