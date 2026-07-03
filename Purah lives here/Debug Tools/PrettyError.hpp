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

#ifndef PURAH_DEBUG_TOOLS_PRETTY_ERROR_HPP
#define PURAH_DEBUG_TOOLS_PRETTY_ERROR_HPP

#pragma once

#include <exception>
#include <string>

#include "../Exceptions/Exceptions.hpp"

using namespace purah;

namespace purah::debug {

    class PrettyError {
    public:
        PrettyError(std::exception_ptr e) : e_{e} {}
        PrettyError(error::Exception& e) : e_{std::make_exception_ptr(e)} {}
        PrettyError(std::exception& e) : e_{std::make_exception_ptr(e)} {}

        std::string message(bool colorized = true, bool ascii_only = false) {
            return PrettyError::str(e_, false, colorized, ascii_only);
        }

        static std::string str(std::exception_ptr e,
                               bool is_cause_exception = false,
                               bool colorized = true,
                               bool ascii_only = false) {
            std::string result{};
            if (!e) return result;

            auto prefix = [&]() -> std::string {
                if (!is_cause_exception) return colorized ? "\033[31m||" : "||";
                if (!ascii_only) return colorized ? "\033[31m└–" : "└–";
                return colorized ? "\033[31m|–" : "|–";
            };
            auto postfix = [&]() -> std::string {
                return colorized ? "\033[0m" : "";
            };

            try {
                std::rethrow_exception(e);
            } catch (const error::Exception& e) {
                result += prefix();
                result += e.name() + ": ";
                result += postfix();
                result += e.title() + "\n" + e.message();
                if (e.cause_exception()) {
                    result += "\n";
                    result += str(e.cause_exception(), true, colorized, ascii_only);
                }
            } catch (const std::exception& e) {
                result += prefix();
                result += "STD Exception: ";
                result += postfix();
                result += e.what();
            } catch (...) {
                result += prefix();
                result += "Unknown Error";
                result += postfix();
            }

            return result;
        }

    private:
        std::exception_ptr e_;
    };

}

#endif // PURAH_DEBUG_TOOLS_PRETTY_ERROR_HPP
