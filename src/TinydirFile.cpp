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

#include <cstdio>

#define UNICODE
#define _UNICODE
#include "tinydir.h"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "staticlib/tinydir/operations.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

namespace su = staticlib::utils;

std::string file_type(const TinydirFile& tf) {
    if (tf.is_directory()) return "directory";
    if (tf.is_regular_file()) return "regular_file";
    if (tf.exists()) return "unexistent";
    return "unknown";
}

} // namespace

TinydirFile::TinydirFile(const std::string& path) :
path(path.data(), path.length()),
name(su::strip_parent_dir(this->path)) {
    if (this->name.empty()) throw TinydirException(TRACEMSG("Error opening file, path: [" + this->path + "]"));
    std::string dirpath = this->name.length() < this->path.length() ? su::strip_filename(this->path) : "./";
    // tinydir_file_open is doing the same under the hood, but looks to be broken on windows
    auto vec = list_directory(dirpath);
    this->is_exist = false;
    for (auto& tf : vec) {
        if (tf.get_name() == this->name) {
            this->is_dir = tf.is_directory();
            this->is_reg = tf.is_regular_file();
            this->is_exist = true;
            break;
        }
    }
}

TinydirFile::TinydirFile(std::nullptr_t, void* /* tinydir_file* */ pfile) {
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
    this->is_exist = true;
}

TinydirFile::TinydirFile(const TinydirFile& other) :
path(other.path.data(), other.path.length()),
name(other.name.data(), other.name.length()),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { }

TinydirFile& TinydirFile::operator=(const TinydirFile& other) {
    path = std::string(other.path.data(), other.path.length());
    name = std::string(other.name.data(), other.name.length());
    is_dir = other.is_dir;
    is_reg = other.is_reg;
    is_exist = other.is_exist;
    return *this;
}

TinydirFile::TinydirFile(TinydirFile&& other) :
path(std::move(other.path)),
name(std::move(other.name)),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { 
    other.is_dir = false;
    other.is_reg = false;
    other.is_exist = false;
}

TinydirFile& TinydirFile::operator=(TinydirFile&& other) {
    path = std::move(other.path);
    name = std::move(other.name);
    is_dir = other.is_dir;
    other.is_dir = false;
    is_reg = other.is_reg;
    other.is_reg = false;
    is_exist = other.is_exist;
    other.is_exist = false;
    return *this;
}

const std::string& TinydirFile::get_path() const {
    return path;
}

const std::string& TinydirFile::get_name() const {
    return name;
}

bool TinydirFile::exists() const {
    return is_exist;
}

bool TinydirFile::is_directory() const {
    return is_dir;
}

bool TinydirFile::is_regular_file() const {
    return is_reg;
}

su::FileDescriptor TinydirFile::open_read() const {
    if (!(is_exist && is_reg)) throw TinydirException(TRACEMSG(
            "Cannot open descriptor to non-regular file: [" + path + "]," +
            " type: [" + file_type(*this) + "]"));
    return su::FileDescriptor(path, 'r');
}

su::FileDescriptor TinydirFile::open_write() const {
    if (!(!is_exist || is_reg)) throw TinydirException(TRACEMSG(
            "Cannot open descriptor to non-regular file: [" + path + "]," +
            " type: [" + file_type(*this) + "]"));
    return su::FileDescriptor(path, 'w');
}

void TinydirFile::remove() const {
    bool success = remove_quietly();
    if (!success) {
        throw TinydirException(TRACEMSG(
                "Cannot remove file: [" + path + "]," +
                " type: [" + file_type(*this) + "]"));
    }
}

bool TinydirFile::remove_quietly() const STATICLIB_NOEXCEPT {
    return 0 == std::remove(path.c_str());
}

} // namespace
}
