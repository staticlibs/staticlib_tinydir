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
 * File:   operations.hpp
 * Author: alex
 *
 * Created on September 6, 2016, 12:37 PM
 */

#ifndef STATICLIB_TINYDIR_OPERATIONS_HPP
#define	STATICLIB_TINYDIR_OPERATIONS_HPP

#include <string>
#include <vector>

#include "staticlib/tinydir/TinydirFile.hpp"

namespace staticlib {
namespace tinydir {

/**
 * Lists the entries of the specified FS directory non-recursively.
 * Entries that cannot be read will be ignored.
 * 
 * @param dirpath path to directory to read
 * @return list of enries
 */
std::vector<TinydirFile> list_directory(const std::string& dirpath);

} // namespace
}

#endif	/* STATICLIB_TINYDIR_OPERATIONS_HPP */

