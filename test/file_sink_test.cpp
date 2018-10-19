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
#include "staticlib/io.hpp"

#include "staticlib/tinydir/operations.hpp"
#include "staticlib/tinydir/path.hpp"

const std::string dir = "file_sink_test";

void test_write() {
    sl::tinydir::create_directory(dir);
    auto tf = sl::tinydir::path(dir);
    auto deferred = sl::support::defer([tf]() STATICLIB_NOEXCEPT {
        tf.remove_quietly();
    });

    auto filename = dir + "/tmp_write.file";
    auto file = sl::tinydir::path(filename);
    {
        auto fd = file.open_write();
        fd.write("foo");
    }

    slassert(filename == file.open_read().path());
    auto src = file.open_read();
    slassert(3 == src.size());
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    slassert("foo" == sink.get_string());
}

void test_append() {
    sl::tinydir::create_directory(dir);
    auto tf = sl::tinydir::path(dir);
    auto deferred = sl::support::defer([tf]() STATICLIB_NOEXCEPT {
        tf.remove_quietly();
    });

    auto file = sl::tinydir::path(dir + "/tmp_append.file");

    auto not_exist_thrown = false;
    try {
        file.open_write(sl::tinydir::file_sink::open_mode::append);
    } catch (const sl::tinydir::tinydir_exception&) {
        not_exist_thrown = true;
    }
    slassert(not_exist_thrown);
    
    {
        auto fd = file.open_write();
        fd.write("foo");
    }
    {
        auto fd = file.open_write(sl::tinydir::file_sink::open_mode::append);
        fd.write("bar");
    }
    
    auto src = file.open_read();
    slassert(6 == src.size());
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    slassert("foobar" == sink.get_string());

}

void test_seek() {
    sl::tinydir::create_directory(dir);
    auto tf = sl::tinydir::path(dir);
    auto deferred = sl::support::defer([tf]() STATICLIB_NOEXCEPT {
        tf.remove_quietly();
    });

    auto file = sl::tinydir::path(dir + "/tmp_seek.file");
    {
        auto fd = file.open_write();
        fd.write("foo");
    }
    {
        auto fd = file.open_write(sl::tinydir::file_sink::open_mode::from_file);
        fd.seek(1);
        fd.write("bar");
    }
    
    auto src = file.open_read();
    slassert(4 == src.size());
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    slassert("fbar" == sink.get_string());

}

void test_write_from_file() {
    sl::tinydir::create_directory(dir);
    auto tf = sl::tinydir::path(dir);
    auto deferred = sl::support::defer([tf]() STATICLIB_NOEXCEPT {
        tf.remove_quietly();
    });

    auto from = sl::tinydir::path(dir + "/tmp_write_from.file");
    auto file = sl::tinydir::path(dir + "/tmp_write_to.file");
    {
        auto fd = file.open_write();
        fd.write("foo");
    }
    {
        auto fd = from.open_write();
        fd.write("bar");
    }

    {
        auto fd = file.open_write(sl::tinydir::file_sink::open_mode::from_file);
        fd.seek(1);
        fd.write_from_file(from.filepath());
    }

    auto src = file.open_read();
    slassert(4 == src.size());
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    slassert("fbar" == sink.get_string());
}

int main() {
    try {
        test_write();
        test_append();
        test_seek();
        test_write_from_file();
        slassert(!sl::tinydir::path(dir).exists());
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

