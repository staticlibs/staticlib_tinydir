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

std::string file_type(const TinydirFile& tf) {
    if (tf.is_directory()) return "directory";
    if (tf.is_regular_file()) return "regular_file";
    if (tf.exists()) return "unexistent";
    return "unknown";
}

std::string detele_file_or_dir(const TinydirFile& tf) {
    std::string error;
#ifdef STATICLIB_WINDOWS
    auto wpath = su::widen(tf.path());
    auto res1 = ::DeleteFileW(wpath.c_str());
    if (0 == res1) {
        error = "DeleteFileW: " + su::errcode_to_string(::GetLastError());
        auto res2 = ::RemoveDirectoryW(wpath.c_str());
        if (0 == res) {
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

TinydirFile::TinydirFile(const std::string& path) :
fpath(path.data(), path.length()),
fname(su::strip_parent_dir(this->fpath)) {
    if (this->fname.empty()) throw TinydirException(TRACEMSG("Error opening file, path: [" + this->fpath + "]"));
    std::string dirpath = this->fname.length() < this->fpath.length() ? su::strip_filename(this->fpath) : "./";
    // tinydir_file_open is doing the same under the hood, but looks to be broken on windows
    auto vec = list_directory(dirpath);
    this->is_exist = false;
    for (auto& tf : vec) {
        if (tf.name() == this->fname) {
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
    this->fpath = su::narrow(file->path);
    this->fname = su::narrow(file->name);
#else
    this->fpath = std::string(file->path);
    this->fname = std::string(file->name);
#endif
    this->is_dir = 0 != file->is_dir;
    this->is_reg = 0 != file->is_reg;
    this->is_exist = true;
}

TinydirFile::TinydirFile(const TinydirFile& other) :
fpath(other.fpath.data(), other.fpath.length()),
fname(other.fname.data(), other.fname.length()),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { }

TinydirFile& TinydirFile::operator=(const TinydirFile& other) {
    fpath = std::string(other.fpath.data(), other.fpath.length());
    fname = std::string(other.fname.data(), other.fname.length());
    is_dir = other.is_dir;
    is_reg = other.is_reg;
    is_exist = other.is_exist;
    return *this;
}

TinydirFile::TinydirFile(TinydirFile&& other) :
fpath(std::move(other.fpath)),
fname(std::move(other.fname)),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { 
    other.is_dir = false;
    other.is_reg = false;
    other.is_exist = false;
}

TinydirFile& TinydirFile::operator=(TinydirFile&& other) {
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

const std::string& TinydirFile::path() const {
    return fpath;
}

const std::string& TinydirFile::name() const {
    return fname;
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
            "Cannot open descriptor to non-regular file: [" + fpath + "]," +
            " type: [" + file_type(*this) + "]"));
    return su::FileDescriptor(fpath, 'r');
}

su::FileDescriptor TinydirFile::open_write() const {
    if (!(!is_exist || is_reg)) throw TinydirException(TRACEMSG(
            "Cannot open descriptor to non-regular file: [" + fpath + "]," +
            " type: [" + file_type(*this) + "]"));
    return su::FileDescriptor(fpath, 'w');
}

void TinydirFile::remove() const {
    auto err = detele_file_or_dir(*this);
    if (!err.empty()) {
        throw TinydirException(TRACEMSG("Cannot remove file: [" + fpath + "]," +
                " type: [" + file_type(*this) + "], error: [" + err + "]"));
    }
}

bool TinydirFile::remove_quietly() const STATICLIB_NOEXCEPT {
    auto err = detele_file_or_dir(*this);
    return err.empty();
}

} // namespace
}
