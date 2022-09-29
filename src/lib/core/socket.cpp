/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "socket.hpp"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <sstream>


namespace caio {

SocketAddress::SocketAddress(const sockaddr_t &saddr)
{
    char buf[INET_ADDRSTRLEN + 1];

    switch (saddr.sa.sa_family) {
    case AF_UNIX:
        _saddrlen = SUN_LEN(&saddr.un);
        std::copy_n(reinterpret_cast<const uint8_t *>(&saddr), _saddrlen, reinterpret_cast<uint8_t *>(&_saddr));
        _addr = _saddr.un.sun_path;
        break;

    case AF_INET:
        _saddrlen = sizeof(saddr.in);
        _saddr.in = saddr.in;
        if (::inet_ntop(AF_INET, &_saddr.in.sin_addr, buf, sizeof(buf)) != nullptr) {
            _addr = std::string{buf} + ":" + std::to_string(ntohs(_saddr.in.sin_port));
        }
        break;

    default:;
    }
}

std::pair<std::string, std::string> SocketAddress::host_port(const std::string &addr)
{
    auto pos = addr.find(":");
    if (pos == std::string::npos) {
        return {addr, ""};
    }

    auto host = addr.substr(0, pos);
    auto port = addr.substr(pos + 1);

    return {host, port};
}

void SocketAddress::resolve(const std::string &addr)
{
    static const ::addrinfo hints_unix = {
        .ai_flags    = AI_CANONNAME,
        .ai_family   = AF_UNIX,
        .ai_socktype = SOCK_STREAM
    };

    static const ::addrinfo hints_ipv4 = {
        .ai_flags    = AI_CANONNAME,
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    ::addrinfo *res{};
    auto [host, port] = host_port(addr);

    int err = ::getaddrinfo(host.c_str(), port.c_str(), &hints_unix, &res);
    if (err != 0) {
        err = ::getaddrinfo(host.c_str(), port.c_str(), &hints_ipv4, &res);
        if (err != 0) {
            throw SocketError{"Can't resolve address: " + addr + ": " + ::gai_strerror(err)};
        }
    }

    _addr = std::string{res->ai_canonname} + ":" + port;
    _saddrlen = res->ai_addrlen;
    std::copy_n(reinterpret_cast<uint8_t *>(res->ai_addr), _saddrlen, reinterpret_cast<uint8_t *>(&_saddr));

    ::freeaddrinfo(res);
}


Socket::Socket(const SocketAddress &sa, int fd)
{
    if (fd < 0) {
        _fd = ::socket(sa._saddr.sa.sa_family, SOCK_STREAM, 0);
        if (_fd < 0) {
            throw SocketError{"Can't create socket for address: " + sa.to_string() + ": " + Error::to_string()};
        }
    } else {
        _fd = fd;
    }

    _sa = sa;
}

Socket Socket::connect(const SocketAddress &sa)
{
    Socket socket{sa};

    if (::connect(socket._fd, &sa._saddr.sa, sa._saddrlen) < 0) {
        throw SocketError{"Can't connect: " + sa.to_string() + ": " + Error::to_string()};
    }

    return socket;
}

Socket Socket::listen(const SocketAddress &sa)
{
    Socket socket{sa};

    int op = 1;
    ::setsockopt(socket._fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));

    if (::bind(socket._fd, &sa._saddr.sa, sa._saddrlen) != 0) {
        throw SocketError{"Can't bind: " + sa.to_string() + ": " + Error::to_string()};
    }

    if (::listen(socket._fd, 5) != 0) {
        throw SocketError{"Can't listen: " + sa.to_string() + ": " + Error::to_string()};
    }

    socket._listen = true;

    return socket;
}

Socket Socket::accept()
{
    SocketAddress::sockaddr_t csaddr{};
    ::socklen_t csaddrlen = sizeof(csaddr);

    int cfd = ::accept(_fd, &csaddr.sa, &csaddrlen);
    if (cfd < 0) {
        throw SocketError{"Can't accept: " + _sa.to_string() + ": " + Error::to_string()};
    }

    return {SocketAddress{csaddr}, cfd};
}

void Socket::close()
{
    if (is_open()) {
        ::shutdown(_fd, SHUT_RDWR);
        ::close(_fd);
        _sa = {};
        _fd = -1;
        _listen = false;
    }
}

Socket &Socket::operator=(Socket &&other)
{
    close();

    _sa     = other._sa;
    _fd     = other._fd;
    _listen = other._listen;

    other._sa     = {};
    other._fd     = -1;
    other._listen = false;

    return *this;
}

std::string Socket::readline()
{
    std::stringstream line{};
    uint8_t c;

    while (true) {
        read(c);
        if (c == '\n') {
            break;
        }

        line << c;
    }

    return line.str();
}

std::string Socket::to_string() const
{
    if (_fd >= 0) {
        std::stringstream ss{};

        ss << (_listen ? "listen " : "") << "socket at " << _sa.to_string();

        return ss.str();
    }

    return {};
}

int Socket::poll(int events, int timeout)
{
    int err = 0;
    ::pollfd pfd = {
        .fd      = _fd,
        .events  = static_cast<short>(events & 0xFFFF),
        .revents = 0
    };

    while (true) {
        err = ::poll(&pfd, 1, timeout);
        if (err < 0) {
            if (errno == EINTR) {
                continue;
            }

            throw SocketError{"Can't poll: " + Error::to_string()};
        }

        break;
    }

    if (err == 0) {
        throw SocketTimeout{"poll timeout: " + std::to_string(timeout) + "ms"};
    }

    if (pfd.revents & POLLHUP) {
        throw BrokenPipeError{"Connection closed by Peer during poll"};
    }

    if (pfd.revents & POLLERR) {
        throw SocketError{"POLLERR"};
    }

    return (pfd.revents & (POLLIN | POLLOUT));
}

void Socket::poll_read(int timeout)
{
    poll(POLLIN, timeout);
}

void Socket::poll_write(int timeout)
{
    poll(POLLOUT, timeout);
}

void Socket::read(uint8_t *buf, size_t bufsiz)
{
    while (bufsiz) {
        poll_read();
        ssize_t r = ::recv(_fd, buf, bufsiz, 0);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno != EAGAIN) {
                throw SocketError{"Can't read: " + Error::to_string()};
            }
        }

        if (r == 0) {
            throw BrokenPipeError{"Can't read: " + Error::to_string(EPIPE)};
        }

        bufsiz -= r;
        buf += r;
    }
}

void Socket::write(const uint8_t *buf, size_t bufsiz)
{
    while (bufsiz) {
        poll_write();
        ssize_t w = ::send(_fd, buf, bufsiz, 0);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno != EAGAIN) {
                throw SocketError{"Can't write: " + Error::to_string()};
            }
        }

        bufsiz -= w;
        buf += w;
    }
}

}
