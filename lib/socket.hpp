/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#pragma once

#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <string>

#include "types.hpp"


namespace cemu {

class SocketError : public Error {
    using Error::Error;
};


class BrokenPipeError : public Error {
    using Error::Error;
};


class SocketTimeout : public Error {
    using Error::Error;
};


class SocketAddress {
public:
    SocketAddress() {
    }

    /**
     * Instantiate a socket address.
     * @param addr Socket address as a human readable string (unix socket or "host[:port]" address).
     * @exception SocketError
     * @see resolve()
     */
    SocketAddress(const std::string &addr) {
        resolve(addr);
    }

    virtual ~SocketAddress() {
    }

    /**
     * Resolve a socket address and assign it to this instance.
     * @param addr Socket address as a human readable string (unix socket or IPv4 address).
     * @exception SocketError
     */
    void resolve(const std::string &addr);

    /**
     * @return This address as a human readable string.
     */
    std::string to_string() const {
        return _addr;
    }

    /**
     * Separate hostname and port from an address string.
     * @return host and port from an address string.
     */
    static std::pair<std::string, std::string> host_port(const std::string &addr);

private:
    //FIXME XXX use variant
    using sockaddr_t = union {
        ::sockaddr    sa;
        ::sockaddr_un un;
        ::sockaddr_in in;
    };

    /**
     * Instantiate from socket address union.
     * @param saddr Socket address union;
     */
    SocketAddress(const sockaddr_t &saddr);

    std::string _addr{};
    sockaddr_t  _saddr{};
    ::socklen_t _saddrlen{};

    friend class Socket;
};


class Socket {
public:
    Socket() {
    }

    Socket(Socket &&other) {
        *this = std::move(other);
    }

    /**
     * Close this socket.
     */
    virtual ~Socket() {
        close();
    }

    /**
     * @return true if this socket is open (valid); false otherwise.
     */
    bool is_open() const noexcept {
        return (_fd >= 0);
    }

    /**
     * @see is_open()
     */
    explicit operator bool() const noexcept {
        return is_open();
    }

    /**
     * Move operator.
     * This instance is closed then moved.
     * @param other Instance to move.
     * @return A reference to this instance.
     */
    Socket &operator=(Socket &&s);

    /**
     * Wait for connections on this listen socket.
     * @return The accepted socket.
     * @exception SocketError
     */
    Socket accept();

    /**
     * Connect to an address.
     * @param sa Address to connect to.
     * @return The connected socket.
     * @exception SocketError
     */
    static Socket connect(const SocketAddress &sa);

    /**
     * Create a listen socket at a specified address.
     * @param sa Address to listen from.
     * @return The listen socket.
     * @exception SocketError
     */
    static Socket listen(const SocketAddress &sa);

    /**
     * Close this socket.
     */
    void close();

    /**
     * Read a value from this socket.
     */
    template<typename T>
    void read(T &value) {
        read(reinterpret_cast<uint8_t *>(&value), sizeof(T));
    }

    /**
     * Write a value to this socket.
     */
    template<typename T>
    void write(const T &value) {
        write(reinterpret_cast<uint8_t *>(&value), sizeof(T));
    }

    /**
     * Write a string to this socket.
     */
    void write(const std::string &str) {
        write(reinterpret_cast<const uint8_t *>(str.c_str()), str.size());
    }

    /**
     * @return The socket address associated to this socket.
     */
    const SocketAddress &address() const {
        return _sa;
    }

    /**
     * Read a string ended with a new line from this socket.
     * @return The read string with the newline character removed.
     */
    std::string readline();

    /**
     * @return This socket as a human readable string.
     */
    std::string to_string() const;

protected:
    /**
     * Create a socket given its address and its native file descriptor.
     * If the native file descriptor is invalid (-1) a new socket is created.
     * @param sa Socket address;
     * @param fd Native file descriptor.
     * @exception SocketError
     */
    Socket(const SocketAddress &sa, int fd = -1);

    /**
     * Wait for an event on this socket.
     * @param events  poll(3) events to wait for;
     * @param timeout Timeout in ms (-1 is no timeout).
     * @return The received events.
     * @exception BrokenPipeError
     * @exception SocketTimeout
     * @exception SocketError
     */
    int poll(int events, int timeout = -1);

    /**
     * Wait until this socket has incoming data.
     * @param timeout Timeout in ms (-1 is no timeout).
     * @exception BrokenPipeError
     * @exception SocketTimeout
     * @exception SocketError
     */
    void poll_read(int timeout = -1);

    /**
     * Wait until this socket is ready to send data.
     * @param timeout Timeout in ms (-1 is no timeout).
     * @exception BrokenPipeError
     * @exception SocketTimeout
     * @exception SocketError
     */
    void poll_write(int timeout = -1);

    /**
     * Read data from this socket.
     * @param buf  Destination buffer;
     * @param size Lenght of data to read (in bytes).
     * @exception BrokenPipeError
     * @exception SocketError
     */
    void read(uint8_t *buf, size_t size);

    /**
     * Write data to this socket.
     * @param buf  Data buffer to write;
     * @param size Size of the buffer.
     * @exception BrokenPipeError
     * @exception SocketError
     */
    void write(const uint8_t *buf, size_t size);

    SocketAddress _sa{};
    int           _fd{-1};
    bool          _listen{};
};

}
