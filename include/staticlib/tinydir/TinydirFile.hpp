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
 * File:   TinydirFile.hpp
 * Author: alex
 *
 * Created on September 6, 2016, 12:36 PM
 */

#ifndef STATICLIB_TINYDIR_TINYDIRFILE_HPP
#define	STATICLIB_TINYDIR_TINYDIRFILE_HPP

#include <string>

#include "staticlib/utils.hpp"

#include "staticlib/tinydir/TinydirException.hpp"

namespace staticlib {
namespace tinydir {

/**
 * Contains the details of FS file or directory, instances of this class
 * are completely disconnected from FS - don't hold any system handles.
 */
class TinydirFile {
    std::string path;
    std::string name;
    bool is_dir = false;
    bool is_reg = false;
    bool is_exist = false;

public:
    /**
     * Constructor
     * 
     * @param path file path
     */
    TinydirFile(const std::string& path);

    /**
     * Copy constructor
     * 
     * @param other other instance
     */
    TinydirFile(const TinydirFile& other);
    
    /**
     * Copy assignment operator
     * 
     * @param other other instance
     * @return this instance
     */
    TinydirFile& operator=(const TinydirFile& other);
    
    /**
     * Move constructor
     * 
     * @param other other instance
     */
    TinydirFile(TinydirFile&& other);
    
    /**
     * Move assignment operator
     * 
     * @param other other instance
     * @return this instance
     */
    TinydirFile& operator=(TinydirFile&& other);
    
    /**
     * Returns FS path to this file
     * 
     * @return FS path to this file
     */
    const std::string& get_path() const;
    
    /**
     * Returns name of this file
     * 
     * @return name of this file
     */
    const std::string& get_name() const;
    
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
    staticlib::utils::FileDescriptor open_read() const;

    /**
     * Open current file for writing
     * 
     * @return file descriptor
     */
    staticlib::utils::FileDescriptor open_write() const;
    
    /**
     * Deletes this file
     * 
     * @throws TinydirException on IO error
     */
    void remove() const;
    
    /**
     * Deletes this file
     * 
     * @return true if file was successfully deleted, false otherwise
     */
    bool remove_quietly() const STATICLIB_NOEXCEPT;
    
    // private api
    
    TinydirFile(std::nullptr_t, void* /* tinydir_file* */ file);

};

} // namespace
}


#endif	/* STATICLIB_TINYDIR_TINYDIRFILE_HPP */
