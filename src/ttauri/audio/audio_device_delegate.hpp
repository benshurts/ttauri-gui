// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_block.hpp"

namespace tt {

class audio_device_delegate {
public:
    audio_device_delegate();
    virtual ~audio_device_delegate() = 0;

    /** Process a block of samples.
     *
     * @param inputBlock Samples captured from the audio device.
     * @param outputBlock Samples rendered to the audio device.
     * @param current_time The current time according to the audio clock.
     */
    virtual void process_audio(
    ) noexcept = 0;
};

}
