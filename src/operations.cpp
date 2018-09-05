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
typedef BOOLEAN(*CreateSymbolicLinkW_type)(LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags);
#else // !STATICLIB_WINDOWS
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#endif // STATICLIB_WINDOWS

#include "tinydir.h"

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"
#include "staticlib/utils.hpp"

namespace staticlib {
namespace tinydir {

std::vector<path> list_directory(const std::string& dirpath) {
    tinydir_dir dir;
    std::string errstr;
#ifdef STATICLIB_WINDOWS
    auto err_open = tinydir_open(std::addressof(dir), sl::utils::widen(dirpath).c_str());
    if (err_open) {
        errstr = sl::utils::errcode_to_string(::GetLastError());
    }
#else
    auto err_open = tinydir_open(std::addressof(dir), dirpath.c_str());
    if (err_open) {
        errstr = ::strerror(errno);
    }
#endif    
    if (err_open) throw tinydir_exception(TRACEMSG("Error opening directory," +
            " path: [" + dirpath + "], error: [" + errstr + "]"));
    auto deferred = sl::support::defer([&dir]() STATICLIB_NOEXCEPT {
        tinydir_close(std::addressof(dir));
    });
    std::vector<path> res;
    while (dir.has_next) {
        tinydir_file file;
        auto err_read = tinydir_readfile(std::addressof(dir), std::addressof(file));
        if (!err_read) { // skip files that we cannot read
            auto tf = path(nullptr, std::addressof(file));
            if ("." != tf.filename() && ".." != tf.filename()) {
                res.emplace_back(std::move(tf));
            }
        }
        auto err_next = tinydir_next(std::addressof(dir));
        if (err_next) throw tinydir_exception(TRACEMSG("Error iterating directory, path: [" + dirpath + "]"));
    }
    std::sort(res.begin(), res.end(), [](const path& a, const path& b) {
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
    auto wpath = sl::utils::widen(dirpath);
    auto res = ::CreateDirectoryW(wpath.c_str(), nullptr);
    success = 0 != res;
    if (!success) {
        error = sl::utils::errcode_to_string(::GetLastError());
    }
#else // !STATICLIB_WINDOWS
    auto res = ::mkdir(dirpath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    success = 0 == res;
    if (!success) {
        error = ::strerror(errno);
    }
#endif // STATICLIB_WINDOWS    
    if (!success) throw tinydir_exception(TRACEMSG(
            "Error creating directory, path: [" + dirpath + "],"
            " error: [" + error + "]"));
}

std::string normalize_path(const std::string& path) {
    auto res = std::string(path.data(), path.length());
    sl::utils::replace_all(res, "/./", "/");
    sl::utils::replace_all(res, "\\", "/");
    while (std::string::npos != res.find("//")) {
        sl::utils::replace_all(res, "//", "/");
    }
    if (res.length() > 1 && '/' == res.back()) {
        res.resize(res.length() - 1);
    }
    return res;
}

std::string full_path(const std::string& fpath) {
#ifdef STATICLIB_WINDOWS
    auto wpath = sl::utils::widen(fpath);
    auto wabs = ::_wfullpath(nullptr, wpath.c_str(), _MAX_PATH);
    if (nullptr == wabs) throw tinydir_exception(TRACEMSG(
            "Error determining full path, path: [" + fpath + "],"
            " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
    auto deferred = sl::support::defer([wabs] () STATICLIB_NOEXCEPT {
        std::free(wabs);
    });
    auto wabs_str = std::wstring(wabs);
    auto res = sl::utils::narrow(wabs_str);
    sl::utils::replace_all(res, "\\", "/");
    return res;
#else // !STATICLIB_WINDOWS
    auto abs = ::realpath(fpath.c_str(), nullptr);
    if (nullptr == abs) throw tinydir_exception(TRACEMSG(
            "Error determining full path, path: [" + fpath + "],"
            " error: [" + ::strerror(errno) + "]"));
    auto deferred = sl::support::defer([abs] () STATICLIB_NOEXCEPT {
        std::free(abs);
    });
    return std::string(abs);
#endif // STATICLIB_WINDOWS
}

void create_symlink(const std::string& dest, const std::string& spath) {
#ifdef STATICLIB_WINDOWS
#ifdef _WIN64 // 64-bit
    auto wdest = sl::utils::widen(dest);
    auto wspath = sl::utils::widen(spath);
    auto flags = 0; //0x2; // SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    if (path(dest).is_directory()) {
        flags |= 0x1; // SYMBOLIC_LINK_FLAG_DIRECTORY
    }
    auto res = ::CreateSymbolicLinkW(wspath.c_str(), wdest.c_str(), flags);
    if (0 == res) throw tinydir_exception(TRACEMSG(
        "Error creating symbolic link, dest: [" + dest + "], link: [" + spath + "]" +
        " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
#else // 32-bit
    (void)dest;
    (void)spath;
    throw tinydir_exception(TRACEMSG("Symbolic links are not supported in 32-bit mode"));
#endif
#else // !STATICLIB_WINDOWS
    auto res = ::symlink(dest.c_str(), spath.c_str());
    if (0 != res) throw tinydir_exception(TRACEMSG(
        "Error creating symbolic link, dest: [" + dest + "], link: [" + spath + "]" +
        " error: [" + ::strerror(errno) + "]"));
#endif // STATICLIB_WINDOWS
}

} // namespace
}
