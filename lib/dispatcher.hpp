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

#include <functional>


namespace cemu {

/**
 * Dispatchable instance.
 * Encapsulator for an instance of type T that can be
 * moved around and then returned back where it came from.
 */
template<typename T>
class DispatcherT : public T {
public:
    DispatcherT() = delete;

    DispatcherT(const DispatcherT &) = delete;

    DispatcherT(DispatcherT &) = delete;

    /**
     * Create a dispatcher.
     * @param dispatch Dispatcher callback;
     * @param instance Instance to move into this dispatcher.
     */
    DispatcherT(const std::function<void(T &&)> &dispatch, T &&instance)
        : T{std::move(instance)},
          _dispatch{dispatch} {
    }

    DispatcherT(DispatcherT &&other) {
        operator=(std::move(other));
    }

    /**
     * Destructor.
     * Dispatch this instance if it was not previouly dispatched.
     * @see dispatch()
     */
    virtual ~DispatcherT() {
        dispatch();
    }

    /**
     * Move operator.
     * @param other Dispatcher to move into this dispatcher.
     */
    DispatcherT &operator=(DispatcherT &&other) {
        static_cast<T &>(*this) = static_cast<T &&>(other);
        _dispatch = std::move(other._dispatch);
        return *this;
    }

    /**
     * Dispatch the instance of this dispatcher.
     * The dispatch callback is executed and the instance in this dispatcher is moved out.
     * This method must be called only once.
     */
    void dispatch() {
        if (_dispatch) {
            _dispatch(std::move(*this));
            static_cast<T &>(*this) = {};
            _dispatch = {};
        }
    }

    /**
     * @return true if this dispatcher is valid; false otherwise.
     */
    operator bool() const {
        return (_dispatch ? true : false);
    }

    /**
     * @return true if this dispatcher is invalid; false otherwise.
     */
    bool operator!() const {
        return (!_dispatch);
    }

private:
    std::function<void(T &&)> _dispatch{};
};

}
