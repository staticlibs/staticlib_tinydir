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
 * File:   file_source.cpp
 * Author: alex
 * 
 * Created on February 6, 2017, 3:00 PM
 */

#include "staticlib/tinydir/file_source.hpp"

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

file_source::file_source(const std::string& file_path) :
file_path(file_path.data(), file_path.size()) {
    std::wstring wpath = su::widen(this->file_path);
    handle = ::CreateFileW(
            wpath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // lpSecurityAttributes
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (INVALID_HANDLE_VALUE == handle) throw tinydir_exception(TRACEMSG(
            "Error opening file descriptor: [" + su::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + this->file_path + "]"));
}

file_source::file_source(file_source&& other) STATICLIB_NOEXCEPT :
handle(other.handle),
file_path(std::move(other.file_path)) {
    other.handle = nullptr;
}

file_source& file_source::operator=(file_source&& other) STATICLIB_NOEXCEPT {
    handle = other.handle;
    other.handle = nullptr;
    file_path = std::move(other.file_path);
    return *this;
}

std::streamsize file_source::read(staticlib::config::span<char> span) {
    if (nullptr != handle) {
        DWORD res;
        DWORD ulen = span.size() <= std::numeric_limits<uint32_t>::max() ?
                static_cast<uint32_t> (span.size()) :
                std::numeric_limits<uint32_t>::max();
        auto err = ::ReadFile(handle, static_cast<void*> (span.data()), ulen,
                std::addressof(res), nullptr);
        if (0 != err) {
            return res > 0 ? static_cast<std::streamsize> (res) : std::char_traits<char>::eof();
        }
        throw tinydir_exception(TRACEMSG("Read error from file: [" + file_path + "]," +
                " error: [" + su::errcode_to_string(::GetLastError()) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to read closed file: [" + file_path + "]"));
}

std::streampos file_source::seek(std::streamsize offset, char whence) {
    if (nullptr != handle) {
        DWORD dwMoveMethod;
        switch (whence) {
        case 'b': dwMoveMethod = FILE_BEGIN;
            break;
        case 'c': dwMoveMethod = FILE_CURRENT;
            break;
        case 'e': dwMoveMethod = FILE_END;
            break;
        default: throw tinydir_exception(TRACEMSG("Invalid whence value: [" + whence + "]" +
                    " for seeking file: [" + file_path + "]"));
        }
        LONG lDistanceToMove = static_cast<LONG> (offset & 0xffffffff);
        LONG lDistanceToMoveHigh = static_cast<LONG> (offset >> 32);
        DWORD dwResultLow = ::SetFilePointer(
                handle,
                lDistanceToMove,
                std::addressof(lDistanceToMoveHigh),
                dwMoveMethod);
        if (INVALID_SET_FILE_POINTER != dwResultLow || ::GetLastError() == NO_ERROR) {
            return (static_cast<long long int> (lDistanceToMoveHigh) << 32) +dwResultLow;
        }
        throw tinydir_exception(TRACEMSG("Seek error over file: [" + file_path + "]," +
                " error: [" + su::errcode_to_string(::GetLastError()) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to seek over closed file: [" + file_path + "]"));
}

void file_source::close() STATICLIB_NOEXCEPT {
    if (nullptr != handle) {
        ::CloseHandle(handle);
        handle = nullptr;
    }
}

off_t file_source::size() {
    if (nullptr != handle) {
        DWORD res = ::GetFileSize(handle, nullptr);
        if (INVALID_FILE_SIZE != res || ::GetLastError() == NO_ERROR) {
            return static_cast<off_t> (res);
        }
        throw tinydir_exception(TRACEMSG("Error getting size of file: [" + file_path + "]," +
                " error: [" + su::errcode_to_string(::GetLastError()) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to get size of closed file: [" + file_path + "]"));
}

#else // STATICLIB_WINDOWS

file_source::file_source(const std::string& file_path) :
file_path(file_path.data(), file_path.size()) {
    fd = ::open(this->file_path.c_str(), O_RDONLY);
    if (-1 == fd) throw tinydir_exception(TRACEMSG("Error opening file: [" + this->file_path + "]," +
            " error: [" + ::strerror(errno) + "]"));
}

file_source::file_source(file_source&& other) STATICLIB_NOEXCEPT :
fd(other.fd),
file_path(std::move(other.file_path)) {
    other.fd = -1;
}

file_source& file_source::operator=(file_source&& other) STATICLIB_NOEXCEPT {
    fd = other.fd;
    other.fd = -1;
    file_path = std::move(other.file_path);
    return *this;
}

std::streamsize file_source::read(staticlib::config::span<char> span) {
    if (-1 != fd) {
        auto res = ::read(fd, span.data(), span.size());
        if (-1 != res) {
            return res > 0 ? res : std::char_traits<char>::eof();
        }
        throw tinydir_exception(TRACEMSG("Read error from file: [" + file_path + "]," +
                " error: [" + ::strerror(errno) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to read closed file: [" + file_path + "]"));
}

std::streampos file_source::seek(std::streamsize offset, char whence) {
    if (-1 != fd) {
        int whence_int;
        switch (whence) {
        case 'b': whence_int = SEEK_SET;
            break;
        case 'c': whence_int = SEEK_CUR;
            break;
        case 'e': whence_int = SEEK_END;
            break;
        default: throw tinydir_exception(TRACEMSG("Invalid whence value: [" + whence + "]" +
                    " for seeking file: [" + file_path + "]"));
        }
        auto res = lseek(fd, offset, whence_int);
        if (static_cast<off_t> (-1) != res) return res;
        throw tinydir_exception(TRACEMSG("Seek error over file: [" + file_path + "]," +
                " error: [" + ::strerror(errno) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to seek over closed file: [" + file_path + "]"));
}

void file_source::close() STATICLIB_NOEXCEPT {
    if (-1 != fd) {
        ::close(fd);
        fd = -1;
    }
}

off_t file_source::size() {
    if (-1 != fd) {
#if defined(STATICLIB_MAC) || defined(STATICLIB_IOS)
        struct stat stat_buf;
        int rc = ::fstat(fd, &stat_buf);
#else
        struct stat64 stat_buf;
        int rc = ::fstat64(fd, &stat_buf);
#endif // STATICLIB_MAC || STATICLIB_IOS
        if (0 == rc) {
            return stat_buf.st_size;
        }
        throw tinydir_exception(TRACEMSG("Error getting size of file: [" + file_path + "]," +
                " error: [" + ::strerror(errno) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to get size of closed file: [" + file_path + "]"));
}

#endif // STATICLIB_WINDOWS

file_source::~file_source() STATICLIB_NOEXCEPT {
    close();
}

const std::string& file_source::path() const {
    return file_path;
}

} // namespace
}
