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

#ifndef PURAH_FILESYSTEM_FILESYSTEM_HPP
#define PURAH_FILESYSTEM_FILESYSTEM_HPP

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "../Lexer/Tokens.hpp"
#include "../Exceptions/Exceptions.hpp"

namespace purah::fsys {

    enum class FileType {
        PURAH, AST, SOURCE
    };

    class File {
    public:
        File(const std::string& path) : path_{path} {}
        File(const std::string& path, tkn::__TOKEN_FILE_t__ file_id)
            : path_{path}, file_id_{file_id} {}
        File(const std::string& path, tkn::__TOKEN_FILE_t__ file_id, tkn::__TOKEN_FILE_t__ imported_from)
            : path_{path}, file_id_{file_id}, imported_from_{imported_from} {}


        FileType type() const {
            if (path_.extension() == ".purah") return FileType::PURAH;
            if (path_.extension() == ".pu")    return FileType::AST;
            return FileType::SOURCE;
        }

        auto  path()    const { return path_; }
        auto& stream()  const { return stream_; }

        auto  file_id()       const { return file_id_; }
        auto  imported_from() const { return imported_from_; }

        void open() {
            stream_.open(path_);
        }
        void close() {
            stream_.close();
        }
        bool is_open() const {
            return stream_.is_open();
        }


    private:
        std::filesystem::path path_{};
        std::ifstream stream_{};
        const tkn::__TOKEN_FILE_t__ file_id_{-1};
        const tkn::__TOKEN_FILE_t__ imported_from_{-1};
    };

    class FileSystem {
    public:
        FileSystem() = default;

        tkn::__TOKEN_FILE_t__ add(const std::string& path) {
            files_.emplace_back(path, file_counter_);
            return file_counter_++;
        }

        File& get_file(tkn::__TOKEN_FILE_t__ file_id) {
            if(file_id < files_.size()) return files_[file_id];
            throw exptn::InternalInterpreterError(
                "Invalid file id",
                "FileSystem is asked for file id: " + std::to_string(file_id)
                    + ", but only " + std::to_string(files_.size()) + " files are available"
            );
        }

    private:
        static inline tkn::__TOKEN_FILE_t__ file_counter_{0};
        std::vector<File> files_{};
    };

}

#endif // PURAH_FILESYSTEM_FILESYSTEM_HPP
