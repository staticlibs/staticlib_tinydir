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
 * File:   tinydir_path.cpp
 * Author: alex
 * 
 * Created on September 6, 2016, 12:39 PM
 */

#include "staticlib/tinydir/tinydir_path.hpp"

#ifdef STATICLIB_WINDOWS
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else // STATICLIB_WINDOWS
#include <cstdio>
#endif

#include "tinydir.h"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "staticlib/tinydir/operations.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

namespace su = staticlib::utils;

std::string file_type(const tinydir_path& tf) {
    if (tf.is_directory()) return "directory";
    if (tf.is_regular_file()) return "regular_file";
    if (tf.exists()) return "unexistent";
    return "unknown";
}

std::string detele_file_or_dir(const tinydir_path& tf) {
    std::string error;
#ifdef STATICLIB_WINDOWS
    auto wpath = su::widen(tf.path());
    auto res1 = ::DeleteFileW(wpath.c_str());
    if (0 == res1) {
        error = "DeleteFileW: " + su::errcode_to_string(::GetLastError());
        auto res2 = ::RemoveDirectoryW(wpath.c_str());
        if (0 == res2) {
            error.append(", RemoveDirectoryW: " + su::errcode_to_string(::GetLastError()));
        } else {
            error = "";
        }
    }
#else // !STATICLIB_WINDOWS
    auto res = std::remove(tf.path().c_str());
    if (0 != res) {
        error = TRACEMSG(::strerror(errno));
    }
#endif // STATICLIB_WINDOWS
    return error;
}

} // namespace

tinydir_path::tinydir_path(const std::string& tinydir_path) :
fpath(tinydir_path.data(), tinydir_path.length()),
fname(su::strip_parent_dir(this->fpath)) {
    if (this->fname.empty()) throw tinydir_exception(TRACEMSG("Error opening file, tinydir_path: [" + this->fpath + "]"));
    std::string dirpath = this->fname.length() < this->fpath.length() ? su::strip_filename(this->fpath) : "./";
    // tinydir_file_open is doing the same under the hood, but looks to be broken on windows
    auto vec = list_directory(dirpath);
    this->is_exist = false;
    for (auto& tf : vec) {
        if (tf.filename() == this->fname) {
            this->is_dir = tf.is_directory();
            this->is_reg = tf.is_regular_file();
            this->is_exist = true;
            break;
        }
    }
}

tinydir_path::tinydir_path(std::nullptr_t, void* /* tinydir_file* */ pfile) {
    auto file = static_cast<tinydir_file*> (pfile);
#ifdef STATICLIB_WINDOWS        
    this->fpath = su::narrow(file->path);
    this->fname = su::narrow(file->filename);
#else
    this->fpath = std::string(file->path);
    this->fname = std::string(file->name);
#endif
    this->is_dir = 0 != file->is_dir;
    this->is_reg = 0 != file->is_reg;
    this->is_exist = true;
}

tinydir_path::tinydir_path(const tinydir_path& other) :
fpath(other.fpath.data(), other.fpath.length()),
fname(other.fname.data(), other.fname.length()),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { }

tinydir_path& tinydir_path::operator=(const tinydir_path& other) {
    fpath = std::string(other.fpath.data(), other.fpath.length());
    fname = std::string(other.fname.data(), other.fname.length());
    is_dir = other.is_dir;
    is_reg = other.is_reg;
    is_exist = other.is_exist;
    return *this;
}

tinydir_path::tinydir_path(tinydir_path&& other) :
fpath(std::move(other.fpath)),
fname(std::move(other.fname)),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { 
    other.is_dir = false;
    other.is_reg = false;
    other.is_exist = false;
}

tinydir_path& tinydir_path::operator=(tinydir_path&& other) {
    fpath = std::move(other.fpath);
    fname = std::move(other.fname);
    is_dir = other.is_dir;
    other.is_dir = false;
    is_reg = other.is_reg;
    other.is_reg = false;
    is_exist = other.is_exist;
    other.is_exist = false;
    return *this;
}

const std::string& tinydir_path::path() const {
    return fpath;
}

const std::string& tinydir_path::filename() const {
    return fname;
}

bool tinydir_path::exists() const {
    return is_exist;
}

bool tinydir_path::is_directory() const {
    return is_dir;
}

bool tinydir_path::is_regular_file() const {
    return is_reg;
}

file_source tinydir_path::open_read() const {
    return file_source(fpath);
}

file_sink tinydir_path::open_write() const {
    return file_sink(fpath);
}

void tinydir_path::remove() const {
    auto err = detele_file_or_dir(*this);
    if (!err.empty()) {
        throw tinydir_exception(TRACEMSG("Cannot remove file: [" + fpath + "]," +
                " type: [" + file_type(*this) + "], error: [" + err + "]"));
    }
}

bool tinydir_path::remove_quietly() const STATICLIB_NOEXCEPT {
    auto err = detele_file_or_dir(*this);
    return err.empty();
}

} // namespace
}
