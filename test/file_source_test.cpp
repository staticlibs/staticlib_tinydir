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
 * File:   file_source_test.cpp
 * Author: alex
 *
 * Created on February 6, 2017, 3:11 PM
 */

#include "staticlib/tinydir/file_source.hpp"

#include "staticlib/tinydir/file_sink.hpp"

#include <cstring>
#include <iostream>

#include "staticlib/config/assert.hpp"

namespace sc = staticlib::config;
namespace st = staticlib::tinydir;

void test_desc() {
    st::file_source desc{"CMakeCache.txt"};
}

void test_desc_fail() {
    bool catched = false;
    try {
        st::file_source desc{"aaa"};
    } catch (const st::tinydir_exception&) {
        catched = true;
    }
    slassert(catched);
}

void test_read() {
    st::file_source desc{"CMakeCache.txt"};
    desc.seek(16);
    std::array<char, 12> buf;
    desc.read(buf);
    std::string res{buf.data(), buf.size()};
    slassert(res == "akeCache fil");
}

void test_accessors() {
    st::file_source file{"CMakeCache.txt"};
    (void) file;
    slassert("CMakeCache.txt" == file.path());
}

int main() {
    try {
        test_desc();
        test_desc_fail();
        test_read();
        test_accessors();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

