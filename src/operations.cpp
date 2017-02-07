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
 * File:   tinydir_operations.cpp
 * Author: alex
 * 
 * Created on September 6, 2016, 12:39 PM
 */

#include "staticlib/tinydir/operations.hpp"

#include <cstdlib>
#include <algorithm>
#include <memory>

#ifdef STATICLIB_WINDOWS
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else // !STATICLIB_WINDOWS
#include <cerrno>
#include <sys/stat.h>
#endif // STATICLIB_WINDOWS

#include "tinydir.h"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

namespace su = staticlib::utils;

class DirDeleter {
public:
    void operator()(tinydir_dir* dir) {
        tinydir_close(dir);
    }
};

} // namespace

std::vector<tinydir_path> list_directory(const std::string& dirpath) {
    tinydir_dir dir_obj;
    std::string errstr;
#ifdef STATICLIB_WINDOWS
    auto err_open = tinydir_open(std::addressof(dir_obj), su::widen(dirpath).c_str());
    if (err_open) {
        errstr = su::errcode_to_string(::GetLastError());
    }
#else
    auto err_open = tinydir_open(std::addressof(dir_obj), dirpath.c_str());
    if (err_open) {
        errstr = ::strerror(errno);
    }
#endif    
    if (err_open) throw tinydir_exception(TRACEMSG("Error opening directory," +
            " path: [" + dirpath + "], error: [" + errstr + "]"));
    auto dir = std::unique_ptr<tinydir_dir, DirDeleter>(std::addressof(dir_obj), DirDeleter());
    std::vector<tinydir_path> res;
    while (dir->has_next) {
        tinydir_file file;
        auto err_read = tinydir_readfile(dir.get(), std::addressof(file));
        if (!err_read) { // skip files that we cannot read
            auto tf = tinydir_path(nullptr, std::addressof(file));
            if ("." != tf.filename() && ".." != tf.filename()) {
                res.emplace_back(std::move(tf));
            }
        }
        auto err_next = tinydir_next(dir.get());
        if (err_next) throw tinydir_exception(TRACEMSG("Error iterating directory, path: [" + dirpath + "]"));
    }
    std::sort(res.begin(), res.end(), [](const tinydir_path& a, const tinydir_path& b) {
        if (a.is_directory() && !b.is_directory()) {
            return true;
        } else if (!a.is_directory() && b.is_directory()) {
            return false;
        }
        return a.filename() < b.filename();
    });
    return res;
}

void create_directory(const std::string& dirpath) {
    bool success = false;
    std::string error;
#ifdef STATICLIB_WINDOWS
    auto wpath = su::widen(dirpath);
    auto res = ::CreateDirectoryW(wpath.c_str(), nullptr);
    success = 0 != res;
    if (!success) {
        error = su::errcode_to_string(::GetLastError());
    }
#else // !STATICLIB_WINDOWS
    auto res = ::mkdir(dirpath.c_str(), 0755);
    success = 0 == res;
    if (!success) {
        error = ::strerror(errno);
    }
#endif // STATICLIB_WINDOWS    
    if (!success) throw tinydir_exception(TRACEMSG(
            "Error creating directory, path: [" + dirpath + "],"
            " error: [" + error + "]"));
}

} // namespace
}
