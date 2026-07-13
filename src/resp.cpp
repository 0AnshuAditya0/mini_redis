#include "resp.hpp"

// Helper: find "\r\n" starting at pos. Returns npos if not found.
static size_t find_crlf(const std::string& buf, size_t pos) {
    return buf.find("\r\n", pos);
}

std::optional<std::vector<std::string>> parse_resp_command(std::string& buffer) {
    // Need at least the array header line, e.g. "*3\r\n"
    if (buffer.empty() || buffer[0] != '*') return std::nullopt;

    size_t pos = 0;
    size_t line_end = find_crlf(buffer, pos);
    if (line_end == std::string::npos) return std::nullopt; // incomplete

    int num_elements;
    try {
        num_elements = std::stoi(buffer.substr(1, line_end - 1));
    } catch (...) {
        return std::nullopt; // malformed, but let's not crash — treat as incomplete for now
    }

    pos = line_end + 2;
    std::vector<std::string> tokens;

    for (int i = 0; i < num_elements; ++i) {
        if (pos >= buffer.size() || buffer[pos] != '$') return std::nullopt; // incomplete
        line_end = find_crlf(buffer, pos);
        if (line_end == std::string::npos) return std::nullopt;

        int len = std::stoi(buffer.substr(pos + 1, line_end - pos - 1));
        pos = line_end + 2;

        if (pos + len + 2 > buffer.size()) return std::nullopt; // incomplete (bulk string + trailing \r\n)

        tokens.push_back(buffer.substr(pos, len));
        pos += len + 2; // skip value + \r\n
    }

    // Full command parsed — remove consumed bytes from buffer
    buffer.erase(0, pos);
    return tokens;
}

std::string resp_simple_string(const std::string& s) {
    return "+" + s + "\r\n";
}

std::string resp_bulk_string(const std::string& s) {
    return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n";
}

std::string resp_nil() {
    return "$-1\r\n";
}

std::string resp_integer(long long n) {
    return ":" + std::to_string(n) + "\r\n";
}

std::string resp_error(const std::string& msg) {
    return "-" + msg + "\r\n";
}