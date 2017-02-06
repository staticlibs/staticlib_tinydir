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
 * File:   TinydirFile_test.cpp
 * Author: alex
 *
 * Created on September 6, 2016, 6:12 PM
 */

#include "staticlib/tinydir/operations.hpp"

#include <cstring>
#include <iostream>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "staticlib/config/assert.hpp"

namespace sc = staticlib::config;
namespace st = staticlib::tinydir;
namespace su = staticlib::utils;

void test_file() {
    // create dir
    auto dir = std::string("TinydirFile_test");
    st::create_directory(dir);
    auto deferred = sc::defer([dir]() STATICLIB_NOEXCEPT{
        st::TinydirFile(dir).remove_quietly();
    });
    
    auto filename = dir + "/tmp.file";
    auto file = st::TinydirFile(filename);
    slassert(!file.is_directory());
    slassert(!file.is_regular_file());
    slassert(!file.exists());
    {
        auto fd = file.open_write();
        fd.write({"foo", 3});
    }
    auto deferred2 = sc::defer([filename]() STATICLIB_NOEXCEPT{
        st::TinydirFile(filename).remove_quietly();
    });
    auto nfile = st::TinydirFile(filename);
    slassert(!nfile.is_directory());
    slassert(nfile.is_regular_file());
    slassert(nfile.exists());
}

int main() {
    try {
        test_file();
        slassert(!st::TinydirFile("TinydirFile_test").exists());
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
