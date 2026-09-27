#pragma once

#include <span>
#include <string_view>
namespace strobe::fs::details {

bool is_path_separator(char value) noexcept;

bool has_trailing_separator(std::span<const char> path) noexcept;

std::string_view path_name(std::span<const char> null_terminated_path) noexcept;

std::string_view path_extension(
    std::span<const char> null_terminated_path) noexcept;

std::size_t normalize_path_inplace(std::span<char> nonNullTerminatedPath);

}
