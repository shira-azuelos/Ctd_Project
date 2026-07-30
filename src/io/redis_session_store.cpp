#include "io/redis_session_store.h"
#include <iostream>
#include <string>
#include <cstdlib>

#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace io {
void RedisSessionStore::set_session(const std::string& username,
                                     const std::string& room_id,
                                     int ttl_seconds) {
    std::thread([username, room_id, ttl_seconds]() {
        const char* env_host = std::getenv("REDIS_HOST");
        std::string host = env_host ? env_host : "127.0.0.1";
        constexpr int port = 6379;

#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            return;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
        if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
            closesocket(sock);
            WSACleanup();
            return;
        }
#else
        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
        if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(sock);
            return;
        }
#endif

        std::string key     = "session:" + username;
        std::string ttl_str = std::to_string(ttl_seconds);

        std::string resp =
            "*4\r\n"
            "$5\r\nSETEX\r\n"
            "$" + std::to_string(key.size())     + "\r\n" + key     + "\r\n" +
            "$" + std::to_string(ttl_str.size()) + "\r\n" + ttl_str + "\r\n" +
            "$" + std::to_string(room_id.size()) + "\r\n" + room_id + "\r\n";

#ifdef _WIN32
        ::send(sock, resp.c_str(), static_cast<int>(resp.size()), 0);
        closesocket(sock);
        WSACleanup();
#else
        ::send(sock, resp.c_str(), resp.size(), 0);
        ::close(sock);
#endif
    }).detach();
}

} 
