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

#ifndef PURAH_EXCEPTIONS_EXCEPTIONS_HPP
#define PURAH_EXCEPTIONS_EXCEPTIONS_HPP

#pragma once

#include <string>
#include <exception>

namespace purah::error {

    class Exception : public std::exception {
    public:
        Exception(const std::string& message) : message_{message} {
            make_out();
        }
        Exception(const std::string& title, const std::string& message)
            : title_{title}, message_{message} {
            make_out();
        }
        Exception(const std::string& name, const std::string& title, const std::string& message)
            : name_{name}, title_{title}, message_{message} {
            make_out();
        }

        const char* what() const noexcept override {
            return out.c_str();
        }

        const std::string name()    const noexcept { return title_; }
        const std::string title()   const noexcept { return title_; }
        const std::string message() const noexcept { return message_; }

        void make_out() {
            out.clear();
            if (!name_.empty()) out += "||" + name_ + ": ";
            out += title_ + "\n" + message_;
        }

    private:
        std::string title_{};
        std::string message_{};
        const std::string name_{};
        std::string out{};
    };

    class InternalInterpreterError : public Exception {
    public:
        InternalInterpreterError(const std::string& message)
            : Exception("Internal Interpreter Error", "", message) {}
        InternalInterpreterError(const std::string& title, const std::string& message)
            : Exception("Internal Interpreter Error", title, message) {}

    };

    class LexerError : public Exception {
    public:
        LexerError(const std::string& message)
            : Exception("Lexer Error", "", message) {}
        LexerError(const std::string& title, const std::string& message)
            : Exception("Lexer Error", title, message) {}
    };

}

#endif // PURAH_EXCEPTIONS_EXCEPTIONS_HPP
