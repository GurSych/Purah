// Copyright 2026 GurSych (Daniil Gurchin)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cctype>

#include "Lexer.hpp"
#include "Tokens.hpp"

using namespace purah;

lxr::Lexer::Lexer(purah::fsys::File& file) {
    if (!file.is_open()) file.read();
    content_ = file.content();
    file_    = file.file_id();
}

lxr::Lexer::Lexer(std::string_view content, purah::tkn::__TOKEN_FILE_t__ file)
    : content_{content}, file_{file} {}

bool lxr::Lexer::is_end() const {
    return pos_ >= content_.size();
}

tkn::TokenPosition lxr::Lexer::get_position() const {
    return {file_, line_, row_, pos_};
}

char lxr::Lexer::advance() {
    char chr = content_[pos_++];
    if (chr == '\n') {
        line_++;
        row_ = 0;
    } else {
        row_++;
    };
    return chr;
}

char lxr::Lexer::peek() const {
    if (is_end()) return '\0';
    return content_[pos_];
}
char lxr::Lexer::peek_next() const {
    if (pos_ + 1 >= content_.size()) return '\0';
    return content_[pos_ + 1];
}
bool lxr::Lexer::peek_match(char chr) const {
    return peek() == chr;
}

void lxr::Lexer::skip_whitespace() {
    while (!is_end() && is_space(peek())) advance();
}

tkn::Token lxr::Lexer::get_identifier() {
    tkn::TokenPosition position = get_position();
    std::string value{};
    while (!is_end() && is_alpha(peek())) value += advance();
    return {tkn::TokenType::IDENTIFIER, value, position};
}
tkn::Token lxr::Lexer::get_number() {
    tkn::TokenPosition position = get_position();
    std::string value{};
    while (!is_end() && is_digit(peek())) value += advance();
    if (!peek_match('.'))
        return {tkn::TokenType::INTEGER, value, position};
    value += advance();
    while (!is_end() && is_digit(peek())) value += advance();
    return {tkn::TokenType::FLOAT, value, position};
}
tkn::Token lxr::Lexer::get_string() {
    tkn::TokenPosition position = get_position();
    std::string value{};
    advance();
    while (!is_end() && !peek_match('"')) value += advance();
    advance();
    return {tkn::TokenType::STRING, value, position};
}

bool lxr::Lexer::is_space(char chr) {
    return std::isspace(static_cast<unsigned char>(chr));
}
bool lxr::Lexer::is_digit(char chr) {
    return std::isdigit(static_cast<unsigned char>(chr));
}
bool lxr::Lexer::is_alpha(char chr) {
    return std::isalpha(static_cast<unsigned char>(chr));
}
