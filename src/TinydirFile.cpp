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
#include "staticlib/utils.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

#include "staticlib/tinydir/operations.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

namespace su = staticlib::utils;

} // namespace

class TinydirFile::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::string path;
    std::string name;
    bool is_dir;
    bool is_reg;
    
public:
    Impl(const std::string& path) {
        std::string filename = su::strip_parent_dir(path);
        if (filename.empty()) throw TinydirException(TRACEMSG("Error opening file, path: [" + path + "]"));
        std::string dirpath = filename.length() < path.length() ? su::strip_filename(path) : "./";
        // tinydir_file_open doing the same under the hood, but looks to be broken on windows
        auto vec = list_directory(dirpath);
        bool success = false;
        for (auto& tf : vec) {
            if (tf.get_name() == filename) {
                this->path = std::string(tf.get_path().data(), tf.get_path().length());
                this->name = std::string(tf.get_name().data(), tf.get_name().length());
                this->is_dir = tf.is_directory();
                this->is_reg = tf.is_regular_file();
                success = true;
                break;
            }
        }
        if (!success) throw TinydirException(TRACEMSG("Error opening file, path: [" + path + "]," + 
                " directory: [" + dirpath + "], filename: [" + filename + "]"));
    }
    
    Impl(std::nullptr_t, void* /* tinydir_file* */ pfile) {
        auto file = static_cast<tinydir_file*> (pfile);
#ifdef STATICLIB_WINDOWS        
        this->path = su::narrow(file->path);
        this->name = su::narrow(file->name);
#else
        this->path = std::string(file->path);
        this->name = std::string(file->name);
#endif
        this->is_dir = 0 != file->is_dir;
        this->is_reg = 0 != file->is_reg;
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

    su::FileDescriptor open_read(const TinydirFile&) {
        if (!(!is_dir && is_reg)) throw TinydirException(TRACEMSG(
                "Cannot open descriptor to non-regular file: [" + path + "]"));
        return su::FileDescriptor(path, 'r');
    }

    su::FileDescriptor open_write(const TinydirFile&) {
        if (!(!is_dir && is_reg)) throw TinydirException(TRACEMSG(
                "Cannot open descriptor to non-regular file: [" + path + "]"));
        return su::FileDescriptor(path, 'w');
    }       
};
PIMPL_FORWARD_CONSTRUCTOR(TinydirFile, (const std::string&), (), TinydirException)
PIMPL_FORWARD_CONSTRUCTOR(TinydirFile, (std::nullptr_t)(void*), (), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, const std::string&, get_path, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, const std::string&, get_name, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, bool, is_directory, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, bool, is_regular_file, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, su::FileDescriptor, open_read, (), (const), TinydirException)
PIMPL_FORWARD_METHOD(TinydirFile, su::FileDescriptor, open_write, (), (const), TinydirException)

} // namespace
}
