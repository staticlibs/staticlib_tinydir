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
#include <vector>
#else // STATICLIB_WINDOWS
#ifndef STATICLIB_MAC
#include <sys/sendfile.h>
#endif // STATICLIB_MAC
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

#ifdef STATICLIB_WINDOWS

file_sink::file_sink(const std::string& file_path, open_mode mode) :
file_path(file_path.data(), file_path.size()) {
    std::wstring wpath = sl::utils::widen(this->file_path);
    auto access = mode == open_mode::append ? FILE_APPEND_DATA : GENERIC_WRITE;
    auto flags = mode == open_mode::create ? CREATE_ALWAYS : OPEN_ALWAYS;
    auto attributes = mode == open_mode::insert ? FILE_FLAG_OVERLAPPED : FILE_ATTRIBUTE_NORMAL;
    handle = ::CreateFileW(
            wpath.c_str(),
            access,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // lpSecurityAttributes
            flags,
            attributes,
            NULL);
    if (INVALID_HANDLE_VALUE == handle) throw tinydir_exception(TRACEMSG(
            "Error opening file descriptor: [" + sl::utils::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + this->file_path + "]"));
}

file_sink::file_sink(file_sink&& other) STATICLIB_NOEXCEPT :
handle(other.handle),
file_path(std::move(other.file_path)) {
    other.handle = nullptr;
}

file_sink& file_sink::operator=(file_sink&& other) STATICLIB_NOEXCEPT {
    handle = other.handle;
    other.handle = nullptr;
    file_path = std::move(other.file_path);
    return *this;
}

std::streamsize file_sink::write(sl::io::span<const char> span) {
    if (nullptr != handle) {
        DWORD res;
        DWORD ulen = span.size() <= std::numeric_limits<uint32_t>::max() ?
                static_cast<uint32_t> (span.size()) :
                std::numeric_limits<uint32_t>::max();
        auto err = ::WriteFile(handle, static_cast<const void*> (span.data()), ulen,
                std::addressof(res), nullptr);
        if (0 != err) return static_cast<std::streamsize> (res);
        throw tinydir_exception(TRACEMSG("Write error to file: [" + file_path + "]," +
                " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to write into closed file: [" + file_path + "]"));
}

std::streamsize file_sink::write_from_file(const std::string source_file, std::streamsize offset) {
    if (nullptr == handle) throw tinydir_exception(TRACEMSG("Attempt to write into closed file: [" + file_path + "]"));
    std::wstring wpath = sl::utils::widen(source_file);
    auto handle_read = ::CreateFileW(
            wpath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // lpSecurityAttributes
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    if (INVALID_HANDLE_VALUE == handle_read) throw tinydir_exception(TRACEMSG(
            "Error opening file descriptor: [" + sl::utils::errcode_to_string(::GetLastError()) + "]" +
            ", specified path: [" + source_file + "]"));

    auto deferred_src = sl::support::defer([handle_read]() STATICLIB_NOEXCEPT {
                                                   ::CloseHandle(handle_read);
                                               });
    const size_t buff_size = 1024*8;
    std::vector<char> buffer(buff_size);
    auto tmp_span = sl::io::make_span(buffer);

    DWORD bytes_readed = 0;
    DWORD res = 0;
    DWORD overall_writed = 0;
    OVERLAPPED overlapped;
	overlapped.Offset = static_cast<DWORD> (offset);
    overlapped.OffsetHigh = 0; // greater part of offset not used
    overlapped.hEvent = nullptr;
	BOOL err = false;
    while (ReadFile(handle_read, buffer.data(), buff_size, std::addressof(bytes_readed), NULL) && bytes_readed > 0) {
		overlapped.Offset = static_cast<DWORD> (offset) + overall_writed;
        ::WriteFile(handle, static_cast<const void*> (buffer.data()), bytes_readed,
                std::addressof(res), std::addressof(overlapped));
        // write result for overlapped files recieved throught GetOverlappedResult
        err = ::GetOverlappedResult(handle, std::addressof(overlapped), std::addressof(res), true);
        overall_writed += res;
    }

	if (err) return static_cast<std::streamsize> (res);
    throw tinydir_exception(TRACEMSG("Write error from file [" + source_file + "] to file: [" + file_path + "]," +
            " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
}

std::streampos file_sink::seek(std::streamsize offset, char whence) {
	if (nullptr == handle) throw tinydir_exception(TRACEMSG("Attempt to seek over closed file: [" + file_path + "]"));
    DWORD whence_int;
    switch (whence) {
        case 'b': whence_int = FILE_BEGIN;
            break;
        case 'c': whence_int = FILE_CURRENT;
            break;
        case 'e': whence_int = FILE_END;
            break;
        default: throw tinydir_exception(TRACEMSG("Invalid whence value: [" + whence + "]" +
                                                  " for seeking file: [" + file_path + "]"));
    }

    auto res = SetFilePointer(handle, static_cast<LONG>(offset), nullptr, whence_int);
    if (INVALID_SET_FILE_POINTER != res && ERROR_NEGATIVE_SEEK != res ) return  static_cast<std::streampos> (res);
    throw tinydir_exception(TRACEMSG("Seek error over file: [" + file_path + "]," +
                                     " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
}

void file_sink::close() STATICLIB_NOEXCEPT {
    if (nullptr != handle) {
        ::CloseHandle(handle);
        handle = nullptr;
    }
}

#else // STATICLIB_WINDOWS

file_sink::file_sink(const std::string& file_path, open_mode mode) :
file_path(file_path.data(), file_path.size()) {
    auto flags = O_WRONLY;
    switch (mode) {
    case open_mode::append:
        flags = O_WRONLY | O_APPEND;
        break;
    case open_mode::insert:
        flags = O_RDWR | O_CREAT;
        break;
    case open_mode::create:
    default:
        flags = O_WRONLY | O_CREAT | O_TRUNC;
        break;
    }

    this->fd = ::open(this->file_path.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (-1 == this->fd) throw tinydir_exception(TRACEMSG("Error opening file: [" + this->file_path + "]," +
            " error: [" + ::strerror(errno) + "]"));
}

file_sink::file_sink(file_sink&& other) STATICLIB_NOEXCEPT :
fd(other.fd),
file_path(std::move(other.file_path)) {
    other.fd = -1;
}

file_sink& file_sink::operator=(file_sink&& other) STATICLIB_NOEXCEPT {
    fd = other.fd;
    other.fd = -1;
    file_path = std::move(other.file_path);
    return *this;
}

std::streamsize file_sink::write(sl::io::span<const char> span) {
    if (-1 != fd) {
        auto res = ::write(fd, span.data(), span.size());
        if (-1 != res) return res;
        throw tinydir_exception(TRACEMSG("Write error to file: [" + file_path + "]," +
                " error: [" + ::strerror(errno) + "]"));
    } else throw tinydir_exception(TRACEMSG("Attempt to write into closed file: [" + file_path + "]"));
}

std::streamsize file_sink::write_from_file(const std::string source_file, std::streamsize offset) {
    const int error_value = -1;
    if (error_value == fd) tinydir_exception(TRACEMSG("Attempt to write into closed file: [" + file_path + "]"));

    int source = ::open(source_file.c_str(), O_RDONLY, 0);
    if (error_value == source) throw sl::tinydir::tinydir_exception(TRACEMSG("Error opening src file: [" + source_file + "]," +
                                                                    " error: [" + ::strerror(errno) + "]"));
    auto deferred_src = sl::support::defer([source]() STATICLIB_NOEXCEPT {
                                               ::close(source);
                                           });
    struct stat stat_source;
    auto err_stat = ::fstat(source, std::addressof(stat_source));
    if (error_value == err_stat) throw sl::tinydir::tinydir_exception(TRACEMSG("Error obtaining file status: [" + source_file + "]," +
                                                                      " error: [" + ::strerror(errno) + "]"));

    this->seek(offset, 'b');
#ifdef STATICLIB_MAC
    const size_t buff_size = 1024*10;
    std::vector<char> buffer(buff_size);
    ssize_t readed = 0;
    ssize_t writed_bytes = 0;
    const ssize_t read_eof = 0;
    while (true) {
        readed = ::read(source, buffer.data(), buff_size);
        if (read_eof == readed) break;
        if (error_value == readed) throw support::exception(TRACEMSG("Error copying file. Can't read source file: [" + source_file + "]," +
                                                                     " to write to: [" + file_path + "]" +
                                                                     " error: [" + ::strerror(errno) + "]"));
        writed_bytes = ::write(fd, buffer.data(), readed);
        if (error_value == writed_bytes) throw support::exception(TRACEMSG("Error copying file. Can't write to file, from: [" + source_file + "]," +
                                                                                " to: [" + file_path + "]" +
                                                                                " error: [" + ::strerror(errno) + "]"));
    }
#else
    auto writed_bytes = ::sendfile(fd, source, NULL, stat_source.st_size);
    if (error_value == writed_bytes) throw support::exception(TRACEMSG("Error copying file: [" + source_file + "]," +
                                                                            " target: [" + file_path + "]" +
                                                                            " error: [" + ::strerror(errno) + "]"));
#endif

    return writed_bytes;
}

void file_sink::close() STATICLIB_NOEXCEPT {
    if (-1 != fd) {
        ::close(fd);
        fd = -1;
    }
}

std::streampos file_sink::seek(std::streamsize offset, char whence) {
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
