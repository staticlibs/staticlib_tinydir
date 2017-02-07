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
 * File:   file_source.hpp
 * Author: alex
 *
 * Created on February 6, 2017, 2:44 PM
 */

#ifndef STATICLIB_TINYDIR_FILE_SOURCE_HPP
#define	STATICLIB_TINYDIR_FILE_SOURCE_HPP

#include <string>

#include "staticlib/config.hpp"

#include "staticlib/tinydir/tinydir_exception.hpp"

namespace staticlib {
namespace tinydir {

/**
 * Implementation of a file descriptor/handle wrapper with a 
 * unified interface for *nix and windows
 */
class file_source {
#ifdef STATICLIB_WINDOWS
    void* handle = nullptr;
#else // STATICLIB_WINDOWS
    /**
     * Native file descriptor (handle on windows)
     */
    int fd = -1;
#endif // STATICLIB_WINDOWS
    /**
     * Path to file
     */
    std::string file_path;

public:
    /**
     * Constructor
     * 
     * @param file_path path to file
     */
    file_source(const std::string& file_path);

    /**
     * Destructor, will close the descriptor
     */
    ~file_source() STATICLIB_NOEXCEPT;

    /**
     * Deleted copy constructor
     * 
     * @param other instance
     */
    file_source(const file_source&) = delete;

    /**
     * Deleted copy assignment operator
     * 
     * @param other instance
     * @return this instance
     */
    file_source& operator=(const file_source&) = delete;

    /**
     * Move constructor
     * 
     * @param other other instance
     */
    file_source(file_source&& other);

    /**
     * Move assignment operator
     * 
     * @param other other instance
     * @return this instance
     */
    file_source& operator=(file_source&& other);

    /**
     * Reads specified number of bytes from this file descriptor
     * into specified buffer.
     * 
     * @param buf destination buffer
     * @param count number of bytes to read
     * @return number of bytes read, "std::char_traits<char>::eof()" on EOF
     */
    std::streamsize read(staticlib::config::span<char> span);

    /**
     * Seeks over this file descriptor
     * 
     * @param offset offset to seek with
     * @param whence seek direction, supported are 'b' (begin, default),
     *        'e' (end),  and 'c' (current position)
     * @return resulting offset location as measured in bytes from the beginning of the file
     */
    std::streampos seek(std::streamsize offset, char whence = 'b');

    /**
     * Returns the size of the file pointed by this descriptor
     * 
     * @return size of the file in bytes
     */
    off_t size();

    /**
     * Closed the underlying file descriptor, will be called automatically 
     * on destruction
     */
    void close() STATICLIB_NOEXCEPT;

    /**
     * File path accessor
     * 
     * @return path to this file
     */
    const std::string& path() const;
};

} // namespace
}

#endif	/* STATICLIB_TINYDIR_FILE_SOURCE_HPP */

