// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "overlay_delegate.hpp"

namespace tt {

/** A GUI widget which may exist anywhere on a window overlaid above any other widget.
 *
 * The overlay widget allows a content widget to be shown on top of other
 * widgets in the window. It may be used for pop-up widgets, dialog boxes and
 * sheets. 
 *
 * The size of the overlay widget is based on the `widget::minimum_size()`,
 * `widget::preferred_size()` and `widget::maximum_size()`. Unlike other
 * container widgets the clipping rectangle is made tightly around the container
 * widget so that no drawing will happen outside of the overlay. The overlay
 * itself will draw outside the clipping rectangle, for drawing a border and
 * potentially a shadow.
 *
 * As an overlay widget still confined to a window, like other widgets, when
 * setting its layout parameters, it is recommended to use
 * `widget::make_overlay_rectangle()` to make a rectangle that will fit inside
 * the window.
 * 
 * It is recommended that the content of an overlay widget is a scroll widget
 * so that when the overlay widget is drawn smaller than the requested rectangle
 * the content will behave correctly.
 */
class overlay_widget final : public widget {
public:
    using super = widget;
    using delegate_type = overlay_delegate;

    /** Constructs an empty overlay widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     * @param delegate An optional delegate can be used to populate the overlay widget
     *                 during initialization.
     */
    overlay_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate = {}) noexcept;

    /** Add a content widget directly to this overlay widget.
     *
     * This widget is added as the content widget.
     *
     * @pre No content widgets have been added before.
     * @tparam Widget The type of the widget to be constructed.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget &make_widget(Args &&...args) noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(not _content);

        auto &widget = super::make_widget<Widget>(std::forward<Args>(args)...);
        _content = &widget;
        return widget;
    }

    /// @privatesection
    void init() noexcept override;
    void deinit() noexcept override;
    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;
    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;
    [[nodiscard]] color background_color() const noexcept override;
    [[nodiscard]] color foreground_color() const noexcept override;
    void scroll_to_show(tt::rectangle rectangle) noexcept override;
    /// @endprivatesection
private:
    std::weak_ptr<delegate_type> _delegate;
    widget *_content = nullptr;

    void draw_background(draw_context context) noexcept;
};

} // namespace tt