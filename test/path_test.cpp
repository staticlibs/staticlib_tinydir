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
 * File:   path_test.cpp
 * Author: alex
 *
 * Created on September 6, 2016, 6:12 PM
 */

#include "staticlib/tinydir/operations.hpp"

#include <cstring>
#include <iostream>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "staticlib/config/assert.hpp"

void test_file() {
    auto dir = std::string("path_test");
    auto dir_moved = std::string("path_test_moved");
    {
        // create dir        
        sl::tinydir::create_directory(dir);
        auto file = sl::tinydir::path(dir + "/tmp.file");
        slassert(!file.is_directory());
        slassert(!file.is_regular_file());
        slassert(!file.exists());
        {
            auto fd = file.open_write();
            fd.write({"foo", 3});
        }
        {
            auto fd = file.open_write(sl::tinydir::file_sink::open_mode::append);
            fd.write({"bar", 3});
        }
        {
            auto fd = file.open_read();
            auto sink = sl::io::string_sink();
            sl::io::copy_all(fd, sink);
            slassert("foobar" == sink.get_string());
        }
        {
            auto orig = sl::tinydir::path(dir + "/tmp.file");
            slassert(orig.exists());
            auto copied = orig.copy_file("tmp_copy.file");
            slassert("tmp_copy.file" == copied.filepath());
            slassert(copied.exists());
            slassert(copied.is_regular_file());
            auto fd = copied.open_read();
            auto sink = sl::io::string_sink();
            sl::io::copy_all(fd, sink);
            slassert("foobar" == sink.get_string());
            //copied.remove();
        }
        
        auto tdir = sl::tinydir::path(dir);
        auto moved = tdir.rename(dir_moved);
        slassert(moved.is_directory());
        slassert(!moved.is_regular_file());
        slassert(moved.exists());
        
        auto deferred = sl::support::defer([&moved]() STATICLIB_NOEXCEPT {
            moved.remove_quietly();
        });
        
        auto nfile = sl::tinydir::path(dir_moved + "/tmp.file");
        auto deferred2 = sl::support::defer([&nfile]() STATICLIB_NOEXCEPT{
            nfile.remove_quietly();
        });
        slassert(!nfile.is_directory());
        slassert(nfile.is_regular_file());
        slassert(nfile.exists());
    }
    slassert(!sl::tinydir::path(dir).exists());
    slassert(!sl::tinydir::path(dir_moved).exists());
}

void test_remove_dir() {
    auto dir = std::string("path_dir_test");
    sl::tinydir::create_directory(dir);
    sl::tinydir::create_directory(dir + "/foo");
    sl::tinydir::create_directory(dir + "/bar");
    sl::tinydir::create_directory(dir + "/foo/baz");
    auto fi = sl::tinydir::path(dir + "/foo/42.txt");
    {
        auto sink = fi.open_write();
        sink.write({"foo"});
    }
    auto dirpath = sl::tinydir::path(dir);
    slassert(dirpath.exists());
    slassert(dirpath.is_directory());
    slassert(dirpath.remove_quietly());
    auto dirpath_refreshed = sl::tinydir::path(dir);
    slassert(!dirpath_refreshed.exists());
}

int main() {
    try {
        test_file();
        test_remove_dir();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
