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

#ifndef PURAH_LEXER_TOKENS_HPP
#define PURAH_LEXER_TOKENS_HPP

#pragma once

#include <string>
#include <cstdint>

#if defined(EOF)
    #undef EOF
#endif

namespace purah::tkn {

    using __TOKEN_FILE_t__   =  int32_t;
    using __TOKEN_LINE_t__   = uint64_t;
    using __TOKEN_COLUMN_t__ = uint64_t;
    using __TOKEN_LENGTH_t__ = uint32_t;

    enum class TokenType {
        EOF = -1, EMPTY,                           /*   EOF EMPTY                   */
        IDENTIFIER,                                /*   identifier                  */
        INTEGER, STRING, FLOAT, BOOL,              /*   integer string float bool   */
        SLASH, DBL_SLASH,                          /*   / //                        */
        BACKSLASH, DBL_BACKSLASH,                  /*   \ \\                        */
        PLUS, MINUS,                               /*   + -                         */
        EQUALS, L_BRACKET, R_BRACKET,              /*   = ( )                       */
        INCREMENT, DECREMENT,                      /*   ++ --                       */
        STAR, PERCENT,                             /*   * %                         */
        IF, ELSE, FUNCTION, RETURN,                /*   if else func return         */
        L_CRL_BRACKET, R_CRL_BRACKET, ARROW,       /*   { } ->                      */
        L_SQR_BRACKET, R_SQR_BRACKET,              /*   [ ]                         */
        NOT, AND, OR, TRUE, FALSE,                 /*   ! && || true false          */
        EQUALITY, NO_EQUALITY,                     /*   == !=                       */
        MORE, MORE_OR_EQUALITY,                    /*   > >=                        */
        LESS, LESS_OR_EQUALITY,                    /*   < <=                        */
        DOT, COMMA, COLON, SEMICOLON,              /*   . , : ;                     */
        VAR, CONST,                                /*   var const                   */
        WHILE, FOR,                                /*   while for                   */
        BREAK, CONTINUE,                           /*   break continue              */
        COUT,                                      /*   COUT                        */
        CLASS                                      /*   class                       */
    };

    class Token {
    public:
        Token(TokenType          type,
              std::string        value,
              __TOKEN_FILE_t__   file,
              __TOKEN_LINE_t__   line,
              __TOKEN_COLUMN_t__ column,
              __TOKEN_LENGTH_t__ length
        ) : type_{type}, value_{value}, file_{file}, line_{line}, column_{column}, length_{length} {}

        TokenType          type()   const { return type_;   }
        const std::string& value()  const { return value_;  }
        __TOKEN_FILE_t__   file()   const { return file_;   }
        __TOKEN_LINE_t__   line()   const { return line_;   }
        __TOKEN_COLUMN_t__ column() const { return column_; }
        __TOKEN_LENGTH_t__ length() const { return length_; }

    private:
        TokenType   type_{};
        std::string value_{};
        __TOKEN_FILE_t__   file_{};
        __TOKEN_LINE_t__   line_{};
        __TOKEN_COLUMN_t__ column_{};
        __TOKEN_LENGTH_t__ length_{};
    };

}

#endif // PURAH_LEXER_TOKENS_HPP
