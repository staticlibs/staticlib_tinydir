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
 * File:   path.hpp
 * Author: alex
 *
 * Created on September 6, 2016, 12:36 PM
 */

#ifndef STATICLIB_TINYDIR_PATH_HPP
#define	STATICLIB_TINYDIR_PATH_HPP

#include <string>

#include "staticlib/config/noexcept.hpp"

#include "staticlib/tinydir/tinydir_exception.hpp"
#include "staticlib/tinydir/file_sink.hpp"
#include "staticlib/tinydir/file_source.hpp"

namespace staticlib {
namespace tinydir {

/**
 * Contains the details of FS file or directory, instances of this class
 * are completely disconnected from FS - don't hold any system handles.
 */
class path {
    std::string fpath;
    std::string fname;
    bool is_dir = false;
    bool is_reg = false;
    bool is_exist = false;

public:
    /**
     * Constructor
     * 
     * @param path file path
     */
    path(const std::string& path);

    /**
     * Copy constructor
     * 
     * @param other other instance
     */
    path(const path& other);
    
    /**
     * Copy assignment operator
     * 
     * @param other other instance
     * @return this instance
     */
    path& operator=(const path& other);
    
    /**
     * Move constructor
     * 
     * @param other other instance
     */
    path(path&& other) STATICLIB_NOEXCEPT;
    
    /**
     * Move assignment operator
     * 
     * @param other other instance
     * @return this instance
     */
    path& operator=(path&& other) STATICLIB_NOEXCEPT;
    
    /**
     * Returns FS path to this file
     * 
     * @return FS path to this file
     */
    const std::string& filepath() const;
    
    /**
     * Returns name of this file
     * 
     * @return name of this file
     */
    const std::string& filename() const;
    
    /**
     * Returns whether this file existed in FS
     * at the time of this instance creation
     * 
     * @return true is file exists, false otherwise
     */
    bool exists() const;
    
    /**
     * Returns whether this instance represents a directory
     * 
     * @return whether this instance represents a directory
     */
    bool is_directory() const;
    
    /**
     * Returns whether this instance represents a regular file
     * 
     * @return whether this instance represents a regular file
     */
    bool is_regular_file() const;
    
    /**
     * Open current file for reading
     * 
     * @return file descriptor
     */
    file_source open_read() const;

    /**
     * Open current file for writing
     * 
     * @return file descriptor
     */
    file_sink open_write() const;
    
    /**
     * Deletes this file or directory
     * 
     * @throws tinydir_exception on IO error
     */
    void remove() const;
    
    /**
     * Deletes this file or directory
     * 
     * @return true if file was successfully deleted, false otherwise
     */
    bool remove_quietly() const STATICLIB_NOEXCEPT;
    
    /**
     * Renames this file or directory to the target path.
     * 
     * @param target target path
     * @returns target path instance
     */
    path rename(const std::string& target) const;
    
    // private api
    
    path(std::nullptr_t, void* /* tinydir_file* */ file);

};

} // namespace
}


#endif	/* STATICLIB_TINYDIR_PATH_HPP */
