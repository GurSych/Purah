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

#ifndef PURAH_LEXER_LEXER_HPP
#define PURAH_LEXER_LEXER_HPP

#include <cstddef>
#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <cctype>

#include "../FileSystem/FileSystem.hpp"
#include "Tokens.hpp"

namespace purah::lxr {

    class Lexer {
    public:
        Lexer(purah::fsys::File& file);
        Lexer(std::string_view content, purah::tkn::__TOKEN_FILE_t__ file = -1);

    private:
        bool is_end() const;

        tkn::TokenPosition get_position() const;

        char advance();

        char peek() const;
        char peek_next() const;
        bool peek_match(char chr) const;

        void skip_whitespace();

        tkn::Token get_identifier();
        tkn::Token get_number();
        tkn::Token get_string();

        static bool is_space(char chr);
        static bool is_digit(char chr);
        static bool is_alpha(char chr);

        std::string_view content_{};
        purah::tkn::__TOKEN_FILE_t__ file_{-1};

        std::size_t pos_{};
        std::size_t line_{};
        std::size_t row_{};
    };

}

#endif // PURAH_LEXER_LEXER_HPP
