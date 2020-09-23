// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../observable.hpp"
#include "../text/format10.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class ToggleWidget : public Widget {
protected:
    static constexpr hires_utc_clock::duration animationDuration = 150ms;

    aarect toggleRectangle;

    aarect sliderRectangle;
    float sliderMoveRange;

    aarect labelRectangle;

    std::unique_ptr<TextCell> onLabelCell;
    std::unique_ptr<TextCell> offLabelCell;
    std::unique_ptr<TextCell> otherLabelCell;

public:
    observable<bool> value;
    observable<std::u8string> onLabel;
    observable<std::u8string> offLabel;

    template<
        typename V = observable<bool>,
        typename L1 = observable<std::u8string>,
        typename L2 = observable<std::u8string>,
        typename L3 = observable<std::u8string>>
    ToggleWidget(
        Window &window,
        Widget *parent,
        V &&value = observable<bool>{},
        L1 &&onLabel = observable<std::u8string>{},
        L2 &&offLabel = observable<std::u8string>{}) noexcept :
        Widget(window, parent),
        value(std::forward<V>(value)),
        onLabel(std::forward<L1>(onLabel)),
        offLabel(std::forward<L2>(offLabel))
    {
        [[maybe_unused]] ttlet value_cbid = this->value.add_callback([this](auto...) {
            this->window.requestRedraw = true;
        });
        [[maybe_unused]] ttlet on_label_cbid = this->onLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
        [[maybe_unused]] ttlet off_label_cbid = this->offLabel.add_callback([this](auto...) {
            requestConstraint = true;
        });
    }

    ~ToggleWidget() {}

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateConstraints(); result < WidgetUpdateResult::Self) {
            return result;
        }

        onLabelCell = std::make_unique<TextCell>(*onLabel, theme->labelStyle);
        offLabelCell = std::make_unique<TextCell>(*offLabel, theme->labelStyle);

        ttlet minimumHeight =
            std::max({onLabelCell->preferredExtent().height(), offLabelCell->preferredExtent().height(), Theme::smallSize});

        ttlet minimumWidth = std::max({onLabelCell->preferredExtent().width(), offLabelCell->preferredExtent().width()}) +
            Theme::smallSize * 2.0f + Theme::margin;

        _preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
        _preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};

        return WidgetUpdateResult::Self;
    }

    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (ttlet result = Widget::updateLayout(displayTimePoint, forceLayout); result < WidgetUpdateResult::Self) {
            return result;
        }

        toggleRectangle = aarect{
            -0.5f, // Expand horizontally due to rounded shape
            base_line() - Theme::smallSize * 0.5f,
            Theme::smallSize * 2.0f + 1.0f, // Expand horizontally due to rounded shape
            Theme::smallSize};

        ttlet labelX = Theme::smallSize * 2.0f + Theme::margin;
        labelRectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

        sliderRectangle = shrink(aarect{0.0f, toggleRectangle.y(), toggleRectangle.height(), toggleRectangle.height()}, 1.5f);

        ttlet sliderMoveWidth = Theme::smallSize * 2.0f - (sliderRectangle.x() * 2.0f);
        sliderMoveRange = sliderMoveWidth - sliderRectangle.width();

        return WidgetUpdateResult::Self;
    }

    void drawToggle(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.cornerShapes = vec{toggleRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(toggleRectangle);
    }

    void drawSlider(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        // Prepare animation values.
        ttlet animationProgress = value.animation_progress(animationDuration);
        if (animationProgress < 1.0f) {
            window.requestRedraw = true;
        }

        ttlet animatedValue = to_float(value, animationDuration);

        ttlet positionedSliderRectangle = mat::T2(sliderMoveRange * animatedValue, 0.0f) * sliderRectangle;

        if (*value) {
            if (*enabled && window.active) {
                drawContext.color = theme->accentColor;
            }
        } else {
            if (*enabled && window.active) {
                drawContext.color = hover ? theme->borderColor(nestingLevel() + 1) : theme->borderColor(nestingLevel());
            }
        }
        std::swap(drawContext.color, drawContext.fillColor);
        drawContext.cornerShapes = vec{positionedSliderRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(positionedSliderRectangle);
    }

    void drawLabel(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        ttlet &labelCell = *value ? onLabelCell : offLabelCell;

        labelCell->draw(drawContext, labelRectangle, Alignment::TopLeft, base_line(), true);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        drawToggle(drawContext);
        drawSlider(drawContext);
        drawLabel(drawContext);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton && rectangle().contains(event.position)) {
                handleCommand(command::gui_activate);
            }
        }
    }

    void handleCommand(command command) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (!*enabled) {
            return;
        }

        if (command == command::gui_activate) {
            if (compare_then_assign(value, !*value)) {
                window.requestRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    HitBox hitBoxTest(vec position) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (rectangle().contains(position)) {
            return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        return *enabled;
    }
};

} // namespace tt
