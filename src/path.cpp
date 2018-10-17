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
 * File:   path.cpp
 * Author: alex
 * 
 * Created on September 6, 2016, 12:39 PM
 */

#include <algorithm>

#include "staticlib/io.hpp"

#include "staticlib/tinydir/path.hpp"

#ifdef STATICLIB_WINDOWS
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "staticlib/utils/windows.hpp"
#else // !STATICLIB_WINDOWS
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#ifndef STATICLIB_MAC
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/types.h> 
#else // STATICLIB_MAC
#include <copyfile.h>
#endif // !STATICLIB_MAC
#endif // STATICLIB_WINDOWS

#include "tinydir.h"

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"
#include "staticlib/utils.hpp"

#include "staticlib/tinydir/operations.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

std::string file_type(const path& tf) {
    if (tf.is_directory()) return "directory";
    if (tf.is_regular_file()) return "regular_file";
    if (tf.exists()) return "unexistent";
    return "unknown";
}

std::string delete_file_or_dir(const std::string& path) {
    std::string error;
#ifdef STATICLIB_WINDOWS
    auto wpath = sl::utils::widen(path);
    auto res1 = ::DeleteFileW(wpath.c_str());
    if (0 == res1) {
        error = "DeleteFileW: " + sl::utils::errcode_to_string(::GetLastError());
        auto res2 = ::RemoveDirectoryW(wpath.c_str());
        if (0 == res2) {
            error.append(", RemoveDirectoryW: " + sl::utils::errcode_to_string(::GetLastError()));
        } else {
            error = "";
        }
    }
#else // !STATICLIB_WINDOWS
    auto res = std::remove(path.c_str());
    if (0 != res) {
        error = TRACEMSG(::strerror(errno));
    }
#endif // STATICLIB_WINDOWS
    return error;
}

std::string move_file_or_dir(const std::string& from, const std::string& to) {
    std::string error;
#ifdef STATICLIB_WINDOWS
    auto wfrom = sl::utils::widen(from);
    auto wto = sl::utils::widen(to);
    auto err = ::MoveFileExW(wfrom.c_str(), wto.c_str(),
            MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (0 == err) {
        error = "MoveFileExW: " + sl::utils::errcode_to_string(::GetLastError());
    }
#else // !STATICLIB_WINDOWS
    auto err = std::rename(from.c_str(), to.c_str());
    if (0 != err) {
        error = TRACEMSG(::strerror(errno));
    }
#endif // STATICLIB_WINDOWS
    return error;
}

// https://stackoverflow.com/q/10195343/314015
void copy_single_file(const path& from_path, const std::string& to) {
    const std::string& from = from_path.filepath();
#ifdef STATICLIB_WINDOWS
    auto wfrom = sl::utils::widen(from);
    auto wto = sl::utils::widen(to);
    auto err = ::CopyFileW(wfrom.c_str(), wto.c_str(), false);
    if (0 == err) {
        throw tinydir_exception(TRACEMSG("Error copying file: [" + from + "]," +
                " target: [" + to + "]" +
                " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
    }
#else // !STATICLIB_WINDOWS
#ifndef STATICLIB_MAC
    int source = ::open(from.c_str(), O_RDONLY, 0);
    if (-1 == source) throw tinydir_exception(TRACEMSG("Error opening src file: [" + from + "]," +
            " error: [" + ::strerror(errno) + "]"));
    auto deferred_src = sl::support::defer([source]() STATICLIB_NOEXCEPT {
        ::close(source);
    });
    struct stat stat_source;
    auto err_stat = ::fstat(source, std::addressof(stat_source));
    if (-1 == err_stat) throw tinydir_exception(TRACEMSG("Error obtaining file status: [" + from + "]," +
            " error: [" + ::strerror(errno) + "]"));
    
    int dest = ::open(to.c_str(), O_WRONLY | O_CREAT | O_TRUNC, stat_source.st_mode);
    if (-1 == dest) throw tinydir_exception(TRACEMSG("Error opening dest file: [" + to + "]," +
            " error: [" + ::strerror(errno) + "]"));
    auto deferred_dest = sl::support::defer([dest]() STATICLIB_NOEXCEPT {
        ::close(dest);
    });

    auto err_sf = ::sendfile(dest, source, 0, stat_source.st_size);
    if (-1 == err_sf) throw tinydir_exception(TRACEMSG("Error copying file: [" + from + "]," +
            " target: [" + to + "]" +
            " error: [" + ::strerror(errno) + "]"));
#else // STATICLIB_MAC
    auto err_cf = ::copyfile(from.c_str(), to.c_str(), nullptr, COPYFILE_ALL);
    if (0 != err_cf) throw tinydir_exception(TRACEMSG("Error copying file: [" + from + "]," +
            " target: [" + to + "]" +
            " error code: [" + sl::support::to_string(err_cf) + "]"));
#endif // !STATICLIB_MAC
#endif // STATICLIB_WINDOWS
}

std::string delete_dir_recursively(const std::string& path) STATICLIB_NOEXCEPT {
    for(auto& ch : list_directory(path)) {
        if (ch.is_directory()) {
            auto err = delete_dir_recursively(ch.filepath());
            if (!err.empty()) return err;
        } else {
            auto err = delete_file_or_dir(ch.filepath());
            if (!err.empty()) return err;
        }
    }
    return delete_file_or_dir(path);
}

} // namespace

path::path(const std::string& path) :
fpath(normalize_path(path)),
fname(sl::utils::strip_parent_dir(this->fpath)) {
    if (this->fname.empty()) throw tinydir_exception(TRACEMSG("Error opening file, path: [" + this->fpath + "]"));
    std::string dirpath = this->fname.length() < this->fpath.length() ? sl::utils::strip_filename(this->fpath) : "./";
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

path::path(std::nullptr_t, void* /* tinydir_file* */ pfile) {
    auto file = static_cast<tinydir_file*> (pfile);
#ifdef STATICLIB_WINDOWS        
    this->fpath = sl::utils::narrow(file->path);
    this->fname = sl::utils::narrow(file->name);
#else
    this->fpath = std::string(file->path);
    this->fname = std::string(file->name);
#endif
    this->is_dir = 0 != file->is_dir;
    this->is_reg = 0 != file->is_reg;
    this->is_exist = true;
}

path::path(const path& other) :
fpath(other.fpath.data(), other.fpath.length()),
fname(other.fname.data(), other.fname.length()),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { }

path& path::operator=(const path& other) {
    fpath = std::string(other.fpath.data(), other.fpath.length());
    fname = std::string(other.fname.data(), other.fname.length());
    is_dir = other.is_dir;
    is_reg = other.is_reg;
    is_exist = other.is_exist;
    return *this;
}

path::path(path&& other) STATICLIB_NOEXCEPT :
fpath(std::move(other.fpath)),
fname(std::move(other.fname)),
is_dir(other.is_dir),
is_reg(other.is_reg),
is_exist(other.is_exist) { 
    other.is_dir = false;
    other.is_reg = false;
    other.is_exist = false;
}

path& path::operator=(path&& other) STATICLIB_NOEXCEPT {
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

const std::string& path::filepath() const {
    return fpath;
}

const std::string& path::filename() const {
    return fname;
}

bool path::exists() const {
    return is_exist;
}

bool path::is_directory() const {
    return is_dir;
}

bool path::is_regular_file() const {
    return is_reg;
}

file_source path::open_read() const {
    return file_source(fpath);
}

file_sink path::open_write() const {
    return file_sink(fpath);
}

file_sink path::open_insert() const {
    return file_sink(fpath, file_sink::open_mode::insert);
}

file_sink path::open_append() const {
    return file_sink(fpath, file_sink::open_mode::append);
}

void path::remove() const {
    auto err = is_dir ? delete_dir_recursively(fpath) : delete_file_or_dir(fpath);
    if (!err.empty()) {
        throw tinydir_exception(TRACEMSG("Cannot remove file: [" + fpath + "]," +
                " type: [" + file_type(*this) + "], error: [" + err + "]"));
    }
}

bool path::remove_quietly() const STATICLIB_NOEXCEPT {
    auto err = is_dir ? delete_dir_recursively(fpath) : delete_file_or_dir(fpath);
    return err.empty();
}

path path::rename(const std::string& target) const {
    auto err = move_file_or_dir(fpath, target);
    if (!err.empty()) {
        throw tinydir_exception(TRACEMSG("Cannot rename file: [" + fpath + "]," +
                " type: [" + file_type(*this) + "]," +
                " to: [" + target + "]," +
                " error: [" + err + "]"));
    }
    return path(target);
}

path path::copy_file(const std::string& target) const {
    if (!is_regular_file()) {
        throw tinydir_exception(TRACEMSG("Cannot copy invalid file," +
                " path: [" + fpath + "]," +
                " target path: [" + target + "]"));
    }
    copy_single_file(fpath, target);
    return path(target);
}

void path::resize(size_t size){
#ifdef STATICLIB_WINDOWS
    std::wstring wpath = sl::utils::widen(this->fname);
    auto handle = ::CreateFileW(
            wpath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // lpSecurityAttributes
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (INVALID_HANDLE_VALUE == handle) throw tinydir_exception(TRACEMSG(
            "Error opening file descriptor: [" + sl::utils::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + this->fname + "]"));
    auto deferred_src = sl::support::defer([handle]() STATICLIB_NOEXCEPT {
                                                   ::CloseHandle(handle);
                                               });
    auto set_pointer_res = ::SetFilePointer(handle, static_cast<DWORD>(size), nullptr, FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == set_pointer_res) throw tinydir_exception(TRACEMSG(
            "Error SetFilePointer: [" + sl::utils::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + this->fname + "]"));
    auto res = ::SetEndOfFile(handle);
    if (0 == res) throw tinydir_exception(TRACEMSG(
            "Error SetEndOfFile: [" + sl::utils::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + this->fname + "]"));
#else
    int dest = ::open(fpath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (-1 == dest) throw support::exception(TRACEMSG("Cannot open file to resize: [" + fpath + "]," +
                                                      " error: [" + ::strerror(errno) + "]"));
    auto deferred_src = sl::support::defer([dest]() STATICLIB_NOEXCEPT {
                                               ::close(dest);
                                           });
    auto result = ftruncate(dest, size);
    if (-1 == result) throw support::exception(TRACEMSG("Cannot resize file: [" + fpath + "]," +
                                                        " error: [" + ::strerror(errno) + "]"));
#endif
}

} // namespace
}
