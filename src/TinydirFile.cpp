/*
 * Copyright 2016, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   TinydirFile.cpp
 * Author: alex
 * 
 * Created on September 6, 2016, 12:39 PM
 */

#include "staticlib/tinydir/TinydirFile.hpp"

#define UNICODE
#define _UNICODE
#include "tinydir.h"

#include "staticlib/config.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

namespace sc = staticlib::config;

} // namespace

class TinydirFile::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::string path;
    std::string name;
    bool is_dir;
    bool is_reg;
    
public:
    Impl(const std::string& path) {
        tinydir_file file;
#ifdef STATICLIB_WINDOWS
        auto err = tinydir_file_open(std::addressof(file), sc::widen(path).c_str());
#else
        auto err = tinydir_file_open(std::addressof(file), path.c_str());
#endif
        if (err) throw TinydirException(TRACEMSG("Error opening file, path: [" + path + "]"));
        read_fields(std::addressof(file));
    }
    
    Impl(void* /* tinydir_file* */ pfile) {
        auto file = static_cast<tinydir_file*> (pfile);
        read_fields(file);
    }

    const std::string& get_path(const TinydirFile&) const {
        return path;
    }

    const std::string& get_name(const TinydirFile&) const {
        return name;
    }

    bool is_directory(const TinydirFile&) const {
        return is_dir;
    }

    bool is_regular_file(const TinydirFile&) const {
        return is_reg;
    }
    
private:
    void read_fields(tinydir_file* file) {
#ifdef STATICLIB_WINDOWS        
        this->path = sc::narrow(file->path);
        this->name = sc::narrow(file->name);
#else
        this->path = std::string(file->path);
        this->name = std::string(file->name);
#endif
        this->is_dir = 0 != file->is_dir;
        this->is_reg = 0 != file->is_reg;
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(TinydirFile, (const std::string&), (), TinydirException)
PIMPL_FORWARD_CONSTRUCTOR(TinydirFile, (void*), (), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, const std::string&, get_path, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, const std::string&, get_name, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, bool, is_directory, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, bool, is_regular_file, (), (const), TinydirException)

} // namespace
}
