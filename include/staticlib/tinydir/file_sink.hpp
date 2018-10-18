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
 * File:   file_sink.hpp
 * Author: alex
 *
 * Created on February 6, 2017, 2:44 PM
 */

#ifndef STATICLIB_TINYDIR_FILE_SINK_HPP
#define STATICLIB_TINYDIR_FILE_SINK_HPP

#include <string>

#include "staticlib/config.hpp"
#include "staticlib/io/span.hpp"

#include "staticlib/tinydir/tinydir_exception.hpp"

namespace staticlib {
namespace tinydir {

/**
 * Implementation of a file descriptor/handle wrapper with a 
 * unified interface for *nix and windows
 */
class file_sink {
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
     * File open mode
     */
    enum class open_mode {
        create, append, insert
    };

    /**
     * Constructor
     * 
     * @param file_path path to file
     */
    file_sink(const std::string& file_path, open_mode mode = open_mode::create);

    /**
     * Destructor, will close the descriptor
     */
    ~file_sink() STATICLIB_NOEXCEPT;

    /**
     * Deleted copy constructor
     * 
     * @param other instance
     */
    file_sink(const file_sink&) = delete;

    /**
     * Deleted copy assignment operator
     * 
     * @param other instance
     * @return this instance
     */
    file_sink& operator=(const file_sink&) = delete;

    /**
     * Move constructor
     * 
     * @param other other instance
     */
    file_sink(file_sink&& other) STATICLIB_NOEXCEPT;

    /**
     * Move assignment operator
     * 
     * @param other other instance
     * @return this instance
     */
    file_sink& operator=(file_sink&& other) STATICLIB_NOEXCEPT;

    /**
     * Writes specified number of bytes to this file descriptor
     * 
     * @param buf source buffer
     * @param count number of bytes to write
     * @return number of bytes successfully written
     */
    std::streamsize write(sl::io::span<const char> span);

    /**
     * Writes specified number of bytes to this file descriptor
     *
     * @param string source file path
     * @param position to write source data
     * @return number of bytes successfully written
     */
    std::streamsize write_from_file(const std::string& source_file, std::streamsize offset);

    /**
     * Seeks over this file descriptor
     *
     * @param offset offset to seek with
     * @param whence seek direction, supported are 'b' (begin, default),
     *        'e' (end),  and 'c' (current position)
     * @return resulting offset location as measured in bytes from the beginning of the file
     */
    std::streampos seek(std::streamsize offset, char whence);

    /**
     * No-op
     * 
     * @return zero
     */
    std::streamsize flush();

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

#endif /* STATICLIB_TINYDIR_FILE_SINK_HPP */

