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
 * File:   TinydirException.hpp
 * Author: alex
 *
 * Created on September 6, 2016, 12:34 PM
 */

#ifndef STATICLIB_TINYDIR_TINYDIREXCEPTION_HPP
#define	STATICLIB_TINYDIR_TINYDIREXCEPTION_HPP

#include "staticlib/config/BaseException.hpp"

namespace staticlib {
namespace tinydir {

/**
 * Module specific exception
 */
class TinydirException : public staticlib::config::BaseException {
public:
    /**
     * Default constructor
     */
    TinydirException() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    TinydirException(const std::string& msg) :
    staticlib::config::BaseException(msg) { }

};

} //namespace
}

#endif	/* STATICLIB_TINYDIR_TINYDIREXCEPTION_HPP */

