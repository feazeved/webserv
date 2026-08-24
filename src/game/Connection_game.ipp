#pragma once
#include "Connection.hpp"

#include <cstdlib>
#include <sstream>
#include <sys/socket.h>
#include "State.hpp"

namespace {
    inline bool isPath(const char* path, usize path_len, const char* str, usize len) {
        return (path_len == len && MEMCMP(path, str, len) == 0);
    }
}

CONNECTION_INL
(i32) handle_game_request() {
    const char* path = reinterpret_cast<const char*>(recvBuffer.data + request.path.index);
    usize path_len = request.path.size;

    if ((request.mode & Mode::GET) && isPath(path, path_len, "/events", 7)) {
        std::string headers =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/event-stream\r\n"
            "Cache-Control: no-cache\r\n"
            "Connection: keep-alive\r\n"
            "\r\n";
        // sse_buffer = headers;
        // isSSE = true;
        cfg->gameState->addSSEClient(this);
        return 2;
    }

    if ((request.mode & Mode::POST) && isPath(path, path_len, "/join", 5)) {
        i32 id = cfg->gameState->addPlayer();

        std::ostringstream body;
        body << "{\"id\":" << id << "}";
        std::string body_str = body.str();

        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body_str.size() << "\r\n"
             << "\r\n"
             << body_str;
        std::string response = resp.str();
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
        return 0;
    }

    if ((request.mode & Mode::POST) && isPath(path, path_len, "/move", 5)) {
        i32 playerId = 0;
        f64 x = 0.0, y = 0.0;
        const char* q = reinterpret_cast<const char*>(recvBuffer.data + request.query.index);
        usize q_len = request.query.size;
        if (q_len > 0) {
            std::string query(q, q_len);
            size_t pos = query.find("playerId=");
            if (pos != std::string::npos)
                playerId = atoi(query.c_str() + pos + 9);
            pos = query.find("x=");
            if (pos != std::string::npos)
                x = atof(query.c_str() + pos + 2);
            pos = query.find("y=");
            if (pos != std::string::npos)
                y = atof(query.c_str() + pos + 2);
        }
        if (playerId > 0)
            cfg->gameState->movePlayer(playerId, x, y);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        send(clientFd, response.c_str(), response.size(), MSG_DONTWAIT | MSG_NOSIGNAL);
        return 0;
    }

    return -1;
}
