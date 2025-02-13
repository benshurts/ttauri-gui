// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "radio_button_widget.hpp"

namespace tt {

[[nodiscard]] bool radio_button_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        // Make room for button and margin.
        _button_size = {theme().size, theme().size};
        ttlet extra_size = extent2{theme().margin + _button_size.width(), 0.0f};
        _minimum_size += extra_size;
        _preferred_size += extra_size;
        _maximum_size += extra_size;

        _minimum_size = max(_minimum_size, _button_size);
        _preferred_size = max(_minimum_size, _button_size);
        _maximum_size = max(_minimum_size, _button_size);

        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void radio_button_widget::layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        _button_rectangle = align(rectangle(), _button_size, alignment::top_left);

        _label_rectangle = aarectangle{_button_rectangle.right() + theme().margin, 0.0f, width(), height()};

        _pip_rectangle =
            align(_button_rectangle, extent2{theme().icon_size, theme().icon_size}, alignment::middle_center);
    }
    super::layout(displayTimePoint, need_layout);
}

void radio_button_widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        draw_radio_button(context);
        draw_radio_pip(context, display_time_point);
    }

    super::draw(std::move(context), display_time_point);
}

void radio_button_widget::draw_radio_button(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    context.draw_box_with_border_inside(
        _button_rectangle, background_color(), focus_color(), corner_shapes{_button_rectangle.height() * 0.5f});
}

void radio_button_widget::draw_radio_pip(draw_context const &context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    _animated_value.update(state() == button_state::on ? 1.0f : 0.0f, display_time_point);
    if (_animated_value.is_animating()) {
        request_redraw();
    }

    // draw pip
    auto float_value = _animated_value.current_value();
    if (float_value > 0.0) {
        ttlet scaled_pip_rectangle = scale(_pip_rectangle, float_value);
        context.draw_box(scaled_pip_rectangle, accent_color(), corner_shapes{scaled_pip_rectangle.height() * 0.5f});
    }
}

} // namespace tt
