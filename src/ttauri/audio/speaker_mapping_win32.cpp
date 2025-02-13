// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "speaker_mapping_win32.hpp"
#include <windows.h>
#include <ks.h>
#include <ksmedia.h>

namespace tt {

[[nodiscard]] speaker_mapping speaker_mapping_from_win32(uint32_t from) noexcept
{
    auto r = speaker_mapping{0};

    if (from & SPEAKER_FRONT_LEFT) {
        r |= speaker_mapping::front_left;
    }
    if (from & SPEAKER_FRONT_RIGHT) {
        r |= speaker_mapping::front_right;
    }
    if (from & SPEAKER_FRONT_CENTER) {
        r |= speaker_mapping::front_center;
    }
    if (from & SPEAKER_LOW_FREQUENCY) {
        r |= speaker_mapping::low_frequency;
    }
    if (from & SPEAKER_BACK_LEFT) {
        r |= speaker_mapping::back_left;
    }
    if (from & SPEAKER_BACK_RIGHT) {
        r |= speaker_mapping::back_right;
    }
    if (from & SPEAKER_FRONT_LEFT_OF_CENTER) {
        r |= speaker_mapping::front_left_of_center;
    }
    if (from & SPEAKER_FRONT_RIGHT_OF_CENTER) {
        r |= speaker_mapping::front_right_of_center;
    }
    if (from & SPEAKER_BACK_CENTER) {
        r |= speaker_mapping::back_center;
    }
    if (from & SPEAKER_SIDE_LEFT) {
        r |= speaker_mapping::side_left;
    }
    if (from & SPEAKER_SIDE_RIGHT) {
        r |= speaker_mapping::side_right;
    }
    if (from & SPEAKER_TOP_CENTER) {
        r |= speaker_mapping::top_center;
    }
    if (from & SPEAKER_TOP_FRONT_LEFT) {
        r |= speaker_mapping::top_front_left;
    }
    if (from & SPEAKER_TOP_FRONT_CENTER) {
        r |= speaker_mapping::top_front_center;
    }
    if (from & SPEAKER_TOP_FRONT_RIGHT) {
        r |= speaker_mapping::top_front_right;
    }
    if (from & SPEAKER_TOP_BACK_LEFT) {
        r |= speaker_mapping::top_back_left;
    }
    if (from & SPEAKER_TOP_BACK_CENTER) {
        r |= speaker_mapping::top_back_center;
    }
    if (from & SPEAKER_TOP_BACK_RIGHT) {
        r |= speaker_mapping::top_back_right;
    }

    return r;
}


[[nodiscard]] uint32_t speaker_mapping_to_win32(speaker_mapping from) noexcept
{
    auto r = uint32_t{0};

    if (to_bool(from & speaker_mapping::front_left)) {
        r |= SPEAKER_FRONT_LEFT;
    }
    if (to_bool(from & speaker_mapping::front_right)) {
        r |= SPEAKER_FRONT_RIGHT;
    }
    if (to_bool(from & speaker_mapping::front_center)) {
        r |= SPEAKER_FRONT_CENTER;
    }
    if (to_bool(from & speaker_mapping::low_frequency)) {
        r |= SPEAKER_LOW_FREQUENCY;
    }
    if (to_bool(from & speaker_mapping::back_left)) {
        r |= SPEAKER_BACK_LEFT;
    }
    if (to_bool(from & speaker_mapping::back_right)) {
        r |= SPEAKER_BACK_RIGHT;
    }
    if (to_bool(from & speaker_mapping::front_left_of_center)) {
        r |= SPEAKER_FRONT_LEFT_OF_CENTER;
    }
    if (to_bool(from & speaker_mapping::front_right_of_center)) {
        r |= SPEAKER_FRONT_RIGHT_OF_CENTER;
    }
    if (to_bool(from & speaker_mapping::back_center)) {
        r |= SPEAKER_BACK_CENTER;
    }
    if (to_bool(from & speaker_mapping::side_left)) {
        r |= SPEAKER_SIDE_LEFT;
    }
    if (to_bool(from & speaker_mapping::side_right)) {
        r |= SPEAKER_SIDE_RIGHT;
    }
    if (to_bool(from & speaker_mapping::top_center)) {
        r |= SPEAKER_TOP_CENTER;
    }
    if (to_bool(from & speaker_mapping::top_front_left)) {
        r |= SPEAKER_TOP_FRONT_LEFT;
    }
    if (to_bool(from & speaker_mapping::top_front_center)) {
        r |= SPEAKER_TOP_FRONT_CENTER;
    }
    if (to_bool(from & speaker_mapping::top_front_right)) {
        r |= SPEAKER_TOP_FRONT_RIGHT;
    }
    if (to_bool(from & speaker_mapping::top_back_left)) {
        r |= SPEAKER_TOP_BACK_LEFT;
    }
    if (to_bool(from & speaker_mapping::top_back_center)) {
        r |= SPEAKER_TOP_BACK_CENTER;
    }
    if (to_bool(from & speaker_mapping::top_back_right)) {
        r |= SPEAKER_TOP_BACK_RIGHT;
    }

    return r;
}


}
