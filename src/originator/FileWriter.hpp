#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP
#include "IOriginator.hpp"
#include "IRouter.hpp"
#include "SocketUNIX.hpp"
#include "http.hpp"
#include "iobserver.hpp"
#include "isocketlike.hpp"
#include "requesthttp.hpp"
#include <map>

class FileWriter : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

private:
    char_string file_path_;
    byte_string content_to_write_;

    FileWriter(const byte_string &file_path);
    FileWriter(const char_string &file_path);

public:
    FileWriter(const byte_string &file_path, const byte_string &content_to_write_);
};

#endif
