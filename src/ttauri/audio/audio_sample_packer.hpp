// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_sample_format.hpp"
#include "../required.hpp"
#include "../architecture.hpp"
#include "../rapid/numeric_array.hpp"
#include "../random/dither.hpp"
#include <cstddef>
#include <bit>

namespace tt {

class audio_sample_packer {
public:
    /** Audio sample packer
     * One instance of this class can be used to pack multiple buffers either
     * from one audio-proc to the next, or for each channel in a group of
     * interleaved channels.
     *
     * @param format The sample format.
     */
    audio_sample_packer(audio_sample_format format) noexcept;

    /** Unpack samples.
     *
     * @param src A pointer to an array of floating point samples of a single channel.
     * @param dst A pointer to a byte array to store the packed samples into.
     * @param num_samples Number of samples.
     */
    void operator()(float const *tt_restrict src, std::byte *tt_restrict dst, size_t num_samples) const noexcept;

private:
    i8x16 _store_shuffle_indices;
    i8x16 _concat_shuffle_indices;
    f32x4 _multiplier;
    mutable dither _dither;
    audio_sample_format _format;
    int _num_chunks_per_quad;
    int _chunk_stride;
    int _direction;
    int _start_byte;
    int _align_shift;
};

} // namespace tt
