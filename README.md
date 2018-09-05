tinydir library wrapper and file IO for Staticlibs
==================================================

[![travis](https://travis-ci.org/staticlibs/staticlib_tinydir.svg?branch=master)](https://travis-ci.org/staticlibs/staticlib_tinydir)
[![appveyor](https://ci.appveyor.com/api/projects/status/github/staticlibs/staticlib_tinydir?svg=true)](https://ci.appveyor.com/project/staticlibs/staticlib-tinydir)

This project is a part of [Staticlibs](http://staticlibs.net/).

This library implements filesystem directory reading, path manipulation and file IO operations,
it is built on top of [tinydir](https://github.com/cxong/tinydir) library. `file_source` and `file_sink`
classes can be used as `Source` and `Sink` with [staticlib_io](https://github.com/staticlibs/staticlib_io) API.

Link to the [API documentation](http://staticlibs.github.io/staticlib_tinydir/docs/html/namespacestaticlib_1_1tinydir.html).

Usage example
-------------

List directory:

    auto vec = sl::tinydir::list_directory("path/to/dir/");
    for (auto& el : vec) {
        std::cout << el.filename() << std::endl;
        std::cout << el.filepath() << std::endl;
        std::cout << el.is_directory() << std::endl;
        std::cout << el.is_regular_file() << std::endl;
    }

How to build
------------

[CMake](http://cmake.org/) is required for building.

[pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/) utility is used for dependency management.
For Windows users ready-to-use binary version of `pkg-config` can be obtained from [tools_windows_pkgconfig](https://github.com/staticlibs/tools_windows_pkgconfig) repository.
See [StaticlibsPkgConfig](https://github.com/staticlibs/wiki/wiki/StaticlibsPkgConfig) for Staticlibs-specific details about `pkg-config` usage.

To build the library on Windows using Visual Studio 2013 Express run the following commands using
Visual Studio development command prompt 
(`C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts\VS2013 x86 Native Tools Command Prompt`):

    git clone https://github.com/staticlibs/staticlib_config.git
    git clone https://github.com/staticlibs/staticlib_support.git
    git clone https://github.com/staticlibs/staticlib_io.git
    git clone https://github.com/staticlibs/staticlib_utils.git
    git clone https://github.com/staticlibs/staticlib_tinydir.git
    git clone https://github.com/staticlibs/tinydir.git
    cd staticlib_tinydir
    mkdir build
    cd build
    cmake ..
    msbuild staticlib_tinydir.sln

See [StaticlibsToolchains](https://github.com/staticlibs/wiki/wiki/StaticlibsToolchains) for 
more information about the toolchain setup and cross-compilation.

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2018-09-05**

 * version 1.2.2
 * use `644` permission for `file_sink`
 * symlinks support

**2018-02-28**

 * version 1.2.1
 * submodule dropped

**2017-12-24**

 * version 1.2.0
 * file IO operations
 * path manipulation operations
 * vs2017 support

**2017-04-08**

 * version 1.1.0
 * classes renamed

**2016-09-06**

 * version 1.0
 * initial public version
