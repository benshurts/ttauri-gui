// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "URL.hpp"
#include <format>
#include <memory>
#include <iostream>
#include <string_view>

namespace tt {

/*! Location inside a configuration file.
 */
class parse_location {
    /** The URL to the file that was parsed.
     * This is a shared_ptr, since a lot of Location objects will point to the same file.
     */
    std::shared_ptr<URL> _file;

    /** Line where the token was found.
     * Starts at 0.
     */
    int _line;

    /** Column where the token was found.
     * Starts at 0.
     */
    int _column;

public:
    /** Construct an empty location object.
     */
    parse_location() noexcept : _file({}), _line(0), _column(0) {}

    /** Construct a location.
     * @param file An URL to the file where the token was found.
     */
    parse_location(std::shared_ptr<URL> const &file) noexcept : _file(file), _line(0), _column(0) {}

    /** Construct a location.
     * @param file An URL to the file where the token was found.
     */
    parse_location(URL const &file) noexcept : _file(std::make_shared<URL>(std::move(file))), _line(0), _column(0) {}

    /** Construct a location.
     * @param file An URL to the file where the token was found.
     * @param line Line number where the token was found.
     * @param column Column where the token was found.
     */
    parse_location(std::shared_ptr<URL> const &file, int line, int column) noexcept :
        _file(file), _line(line - 1), _column(column - 1)
    {
    }

    /** Construct a location.
     * @param line Line number where the token was found.
     * @param column Column where the token was found.
     */
    parse_location(int line, int column) noexcept : _file(), _line(line - 1), _column(column - 1) {}

    [[nodiscard]] bool has_file() const noexcept
    {
        return static_cast<bool>(_file);
    }

    [[nodiscard]] URL const &file() const noexcept
    {
        return *_file;
    }

    [[nodiscard]] int line() const noexcept
    {
        return _line + 1;
    }

    [[nodiscard]] int column() const noexcept
    {
        return _column + 1;
    }

    [[nodiscard]] std::pair<int, int> line_and_column() const noexcept
    {
        return {_line + 1, _column + 1};
    }

    void set_file(std::shared_ptr<URL> file)
    {
        _file = std::move(file);
    }

    void set_file(URL file)
    {
        _file = std::make_shared<URL>(std::move(file));
    }

    void set_line(int line) noexcept
    {
        _line = line - 1;
    }

    void set_column(int column) noexcept
    {
        _column = column - 1;
    }

    void set_line_and_column(std::pair<int, int> line_and_column) noexcept
    {
        _line = line_and_column.first - 1;
        _column = line_and_column.second - 1;
    }

    void increment_column() noexcept
    {
        ++_column;
    }

    void tab_column() noexcept
    {
        _column /= 8;
        _column += 1;
        _column *= 8;
    }

    void increment_line() noexcept
    {
        _column = 0;
        ++_line;
    }

    parse_location &operator+=(char c) noexcept
    {
        switch (c) {
        case '\t': _column = ((_column / 8) + 1) * 8; break;
        case '\f': [[fallthrough]];
        case '\n': ++_line; [[fallthrough]];
        case '\r': _column = 0; break;
        default: ++_column;
        }
        return *this;
    }

    parse_location &operator+=(std::string const &s) noexcept
    {
        for (ttlet c : s) {
            *this += c;
        }
        return *this;
    }

    parse_location &operator+=(char const *s) noexcept
    {
        while (ttlet c = *s++) {
            *this += c;
        }
        return *this;
    }

    parse_location &operator+=(parse_location const &location) noexcept
    {
        if (location._line == 0) {
            _column += location._column;
        } else {
            _line += location._line;
            _column = location._column;
        }
        return *this;
    }

    friend std::string to_string(parse_location const &l) noexcept
    {
        return std::format("{0}:{1}:{2}", l.file(), l.line(), l.column());
    }

    friend std::ostream &operator<<(std::ostream &os, parse_location const &l)
    {
        os << to_string(l);
        return os;
    }
};

} // namespace tt

namespace std {

template<typename CharT>
struct formatter<tt::parse_location, CharT> : formatter<string_view, CharT> {
    auto format(tt::parse_location t, auto &fc)
    {
        return formatter<string_view, CharT>::format(to_string(t), fc);
    }
};

} // namespace std
