// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/axis.hpp"
#include <memory>
#include <functional>

namespace tt {
template<axis,bool>
class scroll_widget;

template<axis Axis, bool ControlsWindow>
class scroll_delegate {
public:
    virtual void init(scroll_widget<Axis,ControlsWindow> &sender) noexcept {}
    virtual void deinit(scroll_widget<Axis, ControlsWindow> &sender) noexcept {}
};

} // namespace tt
