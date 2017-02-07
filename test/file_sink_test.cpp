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
 * File:   file_sink_test.cpp
 * Author: alex
 *
 * Created on February 6, 2017, 3:05 PM
 */

#include "staticlib/tinydir/file_sink.hpp"

#include <cstring>
#include <iostream>

#include "staticlib/config.hpp"

#include "staticlib/config/assert.hpp"

#include "staticlib/tinydir/operations.hpp"
#include "staticlib/tinydir/tinydir_path.hpp"

namespace sc = staticlib::config;
namespace st = staticlib::tinydir;

void test_write() {
    auto dir = std::string("file_sink_test");
    st::create_directory(dir);
    auto tf = st::tinydir_path(dir);
    auto deferred = sc::defer([tf]() STATICLIB_NOEXCEPT {
        tf.remove_quietly();
    });
    auto filename = dir + "/tmp.file";
    auto file = st::tinydir_path(filename);
    {
        auto fd = file.open_write();
        fd.write({"foo", 3});
    }
    auto tfile = st::tinydir_path(filename);
    auto deferred2 = sc::defer([tfile]() STATICLIB_NOEXCEPT {
        tfile.remove_quietly();
    });
    slassert(3 == file.open_read().size());
    slassert(filename == file.open_read().path());
}

int main() {
    try {
        test_write();
        slassert(!st::tinydir_path("file_sink_test").exists());
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

