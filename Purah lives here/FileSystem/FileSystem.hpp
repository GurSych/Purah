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

namespace purah { namespace fsys {

    enum class FileType {
        PURAH, AST, SOURCE
    };

    class File {
    public:
        File(const std::string& path) : path_{path} {}

        FileType type() const {
            if (path_.extension() == ".purah") return FileType::PURAH;
            if (path_.extension() == ".pu")    return FileType::AST;
            return FileType::SOURCE;
        }

        auto  path()   const { return path_;   }
        auto& stream() const { return stream_; }

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
    };

} }

#endif // PURAH_FILESYSTEM_FILESYSTEM_HPP
