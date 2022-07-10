#ifndef FILEREADER_HPP
#define FILEREADER_HPP
#include "../communication/RequestHTTP.hpp"
#include "../interface/IObserver.hpp"
#include "../interface/IOriginator.hpp"
#include "../interface/IRouter.hpp"
#include "../interface/ISocketlike.hpp"
#include "../socket/SocketUNIX.hpp"
#include "../utils/http.hpp"
#include <fstream>
#include <map>

class FileReader : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

private:
    char_string file_path_;
    byte_string data_;
    bool is_ready_to_draw_;

    // ファイルからデータを読み出しておく
    void read_from_file();

public:
    FileReader(const byte_string &file_path);
    FileReader(const char_string &file_path);

    bool is_ready_to_draw() const;
    byte_string draw_data() const;
};

#endif
