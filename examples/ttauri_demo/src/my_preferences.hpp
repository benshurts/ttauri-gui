// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/preferences.hpp"
#include "ttauri/observable.hpp"

class my_preferences : public tt::preferences {
public:
    using super = tt::preferences;

    tt::observable<std::string> audio_output_device_id;

    my_preferences(tt::URL location) noexcept : super(location)
    {
        audio_output_device_id.subscribe(_set_modified_ptr);
    }
    
    void reset() noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        super::reset();

        audio_output_device_id = "";
    }

    [[nodiscard]] tt::datum serialize() const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto r = super::serialize();

        r["audio_output_device_id"] = *audio_output_device_id;
        return r;
    }

    void deserialize(tt::datum const &data) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ++_deserializing;
        super::deserialize(data);

        deserialize_value<std::string>(audio_output_device_id, data, "audio_output_device_id");
        --_deserializing;
    }

};

inline std::unique_ptr<my_preferences> g_my_preferences;
