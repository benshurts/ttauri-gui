// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "unfair_mutex.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <thread>

namespace tt {
class time_stamp_count;

/** Timestamp
 */
struct hires_utc_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_utc_clock>;
    static const bool is_steady = false;

    /** Get the current time.
     */
    [[nodiscard]] static time_point now() noexcept;

    /** Get the current time and TSC value.
     * @pre Use `set_thread_affinity()` to set the CPU affinity to a single CPU.
     */
    [[nodiscard]] static time_point now(time_stamp_count &tsc) noexcept;

    /** Make a time point from a time stamp count.
     * This function will work in two modes:
     *  - subsystem off: Uses now() and the time_stamp_count frequency to
     *    estimate a timepoint from the given tsc.
     *  - subsystem on: Uses the calibrated TSC offset and more accurate
     *    frequency to estimate a timepoint from the given tsc.
     *    this is much faster and a lot more accurate.
     */
    [[nodiscard]] static time_point make(time_stamp_count const &tsc) noexcept;

    /** This will start the calibration subsystem.
     */
    static bool start_subsystem() noexcept;

    /** This will stop the calibration subsystem.
     */
    static void stop_subsystem() noexcept;

    /** A calibration step which will drift the per-cpu tsc-offset.
     * This is a fast wait-free function that may be called from any
     * thread. It is useful to call this from the render thread
     * which means small adjustments to the calibrations are made at
     * 60 fps.
     */
    static void adjust_for_drift() noexcept;

private:
    static inline std::atomic<bool> subsystem_is_running = false;
    static inline std::jthread subsystem_thread;
    static inline unfair_mutex mutex;
    static inline std::array<std::atomic<hires_utc_clock::time_point>, maximum_num_cpus> tsc_epochs = {};

    static void subsystem_proc_frequency_calibration(std::stop_token stop_token) noexcept;
    static void subsystem_proc(std::stop_token stop_token) noexcept;

    /** Subsystem initializer.
     */
    static bool init_subsystem() noexcept;

    /** Subsystem de_initializer.
     */
    static void deinit_subsystem() noexcept;

    [[nodiscard]] static size_t find_cpu_id(uint32_t cpu_id) noexcept;
};

std::string format_engineering(hires_utc_clock::duration duration);

/** Return a ISO-8601 formated date-time.
 * @param utc_timestamp The time_point to format.
 */
std::string format_iso8601_utc(hires_utc_clock::time_point utc_timestamp) noexcept;

/** Return a ISO-8601 formated date-time.
 * @param utc_timestamp The time_point to format.
 * @param time_zone If time_zone is a nullptr then the current timezone is used.
 */
std::string format_iso8601(hires_utc_clock::time_point utc_timestamp, std::chrono::time_zone const *time_zone = nullptr) noexcept;

} // namespace tt
