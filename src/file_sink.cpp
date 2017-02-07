/*
 * Copyright 2017, alex at staticlibs.net
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
 * File:   file_sink.cpp
 * Author: alex
 *
 * Created on February 6, 2017, 2:52 PM
 */

#include "staticlib/tinydir/file_sink.hpp"

#ifdef STATICLIB_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "staticlib/utils/windows.hpp"
#else // STATICLIB_WINDOWS
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#endif // STATICLIB_WINDOWS

#include "staticlib/utils.hpp"

namespace staticlib {
namespace tinydir {

namespace { // anonymous

namespace su = staticlib::utils;

} // namespace

#ifdef STATICLIB_WINDOWS

file_sink::file_sink(const std::string& file_path) :
file_path(file_path.data(), file_path.size()) {
    std::wstring wpath = su::widen(this->file_path);
    handle = ::CreateFileW(
            wpath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // lpSecurityAttributes
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (INVALID_HANDLE_VALUE == handle) throw tinydir_exception(TRACEMSG(
            "Error opening file descriptor: [" + su::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + this->file_path + "]"));
}

file_sink::file_sink(file_sink&& other) :
handle(other.handle),
file_path(std::move(other.file_path)) {
    other.handle = nullptr;
}

file_sink& file_sink::operator=(file_sink&& other) {
    handle = other.handle;
    other.handle = nullptr;
    file_path = std::move(other.file_path);
    return *this;
}

std::streamsize file_sink::write(staticlib::config::span<const char> span) {
    if (nullptr != handle) {
        DWORD res;
        DWORD ulen = span.size() <= std::numeric_limits<uint32_t>::max() ?
                static_cast<uint32_t> (span.size()) :
                std::numeric_limits<uint32_t>::max();
        auto err = ::WriteFile(handle, static_cast<const void*> (span.data()), ulen,
                std::addressof(res), nullptr);
        if (0 != err) return static_cast<std::streamsize> (res);
        throw tinydir_exception(TRACEMSG("Write error to file: [" + file_path + "]," +
                " error: [" + su::errcode_to_string(::GetLastError()) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to write into closed file: [" + file_path + "]"));
}

void file_sink::close() STATICLIB_NOEXCEPT {
    if (nullptr != handle) {
        ::CloseHandle(handle);
        handle = nullptr;
    }
}

#else // STATICLIB_WINDOWS

file_sink::file_sink(const std::string& file_path) :
file_path(file_path.data(), file_path.size()) {
    this->fd = ::open(this->file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (-1 == this->fd) throw tinydir_exception(TRACEMSG("Error opening file: [" + this->file_path + "]," +
            " error: [" + ::strerror(errno) + "]"));
}

file_sink::file_sink(file_sink&& other) :
fd(other.fd),
file_path(std::move(other.file_path)) {
    other.fd = -1;
}

file_sink& file_sink::operator=(file_sink&& other) {
    fd = other.fd;
    other.fd = -1;
    file_path = std::move(other.file_path);
    return *this;
}

std::streamsize file_sink::write(staticlib::config::span<const char> span) {
    if (-1 != fd) {
        auto res = ::write(fd, span.data(), span.size());
        if (-1 != res) return res;
        throw tinydir_exception(TRACEMSG("Write error to file: [" + file_path + "]," +
                " error: [" + ::strerror(errno) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to write into closed file: [" + file_path + "]"));
}

void file_sink::close() STATICLIB_NOEXCEPT {
    if (-1 != fd) {
        ::close(fd);
        fd = -1;
    }
}

#endif // STATICLIB_WINDOWS

file_sink::~file_sink() STATICLIB_NOEXCEPT {
    close();
}

std::streamsize file_sink::flush() {
    return 0;
}

const std::string& file_sink::path() const {
    return file_path;
}

} // namespace
}
