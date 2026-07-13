#pragma once
#include <string>
#include <vector>
#include <optional>

// Parses one RESP array-of-bulk-strings command from a buffer.
// Returns the parsed command (as tokens) and consumes the bytes used from `buffer`.
// Returns std::nullopt if the buffer doesn't yet contain a complete command.
std::optional<std::vector<std::string>> parse_resp_command(std::string& buffer);

// Response formatters
std::string resp_simple_string(const std::string& s);   // +OK\r\n
std::string resp_bulk_string(const std::string& s);      // $5\r\nvalue\r\n
std::string resp_nil();                                   // $-1\r\n
std::string resp_integer(long long n);                    // :1\r\n
std::string resp_error(const std::string& msg);            // -ERR ...\r\n