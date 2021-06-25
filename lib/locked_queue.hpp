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

#include <mutex>
#include <queue>


namespace cemu {

template<typename T>
class LockedQueue {
public:
    LockedQueue() {
    }

    LockedQueue(LockedQueue &&lq)
        : _queue{static_cast<const std::queue<T> &&>(lq)} {
    }

    virtual ~LockedQueue() {
    }

    void clear() {
        std::scoped_lock<std::mutex> _{_qlock};
        _queue = {};
    }

    void push(const T &&value) {
        std::scoped_lock<std::mutex> _{_qlock};
        _queue.push(std::move(value));
    }

    T pop() {
        std::scoped_lock<std::mutex> _{_qlock};
        auto val = std::move(_queue.front());
        _queue.pop();
        return val;
    }

    T & back() {
        std::scoped_lock<std::mutex> _{_qlock};
        return _queue.back();
    }

    size_t size() const {
        std::scoped_lock<std::mutex> _{_qlock};
        return _queue.size();
    }

private:
    mutable std::mutex _qlock{};
    std::queue<T>      _queue{};
};

}
