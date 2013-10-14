/*
Copyright (c) 2013 Mathias Kaerlev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef VOXIE_PATH_H
#define VOXIE_PATH_H

#include <sys/stat.h>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#define PATH_SEP "\\/"
#else
#define PATH_SEP "/"
#endif

inline bool file_exists(const std::string & filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
        return true;
    return false;
}

inline void read_file(const char * filename, char ** data, size_t * ret_size,
                      bool binary = true)
{
    std::ifstream fp;
    fp.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!fp) {
        std::cout << "Could not load file " << filename << std::endl;
        return;
    }

    size_t size = fp.tellg();
    fp.seekg(0, std::ios::beg);
    if (binary)
        *data = new char[size];
    else
        *data = new char[size + 1];
    fp.read(*data, size);
    fp.close();
    if (!binary)
        (*data)[size] = 0;
    *ret_size = size;
}

inline size_t get_file_size(const char * filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

inline std::string get_path_filename(const std::string & path)
{
    size_t pos = path.find_last_of(PATH_SEP);
    if (pos == std::string::npos)
        return path;
    return path.substr(pos + 1);
}

inline std::string get_path_dirname(const std::string & path)
{
    size_t pos = path.find_last_of(PATH_SEP);
    if (pos == std::string::npos)
        return "";
    return path.substr(0, pos + 1);
}

inline std::string get_path_basename(const std::string & path)
{
    std::string path2 = get_path_filename(path);
    return path2.substr(0, path2.find_last_of("."));
}

#endif // VOXIE_PATH_H
