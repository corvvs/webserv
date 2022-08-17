#ifndef FILEREADER_HPP
#define FILEREADER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
#include <fstream>
#include <map>

class FileReader : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

protected:
    char_string file_path_;
    bool originated_;
    ResponseDataList response_data;

    // ファイルからデータを読み出しておく
    minor_error read_from_file();

public:
    FileReader(const RequestMatchingResult &match_result);
    FileReader(const char_string &path);
    ~FileReader();

    virtual void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    virtual void inject_socketlike(ISocketLike *socket_like);
    virtual bool is_originatable() const;
    virtual bool is_reroutable() const;
    virtual bool is_responsive() const;
    virtual bool is_origination_started() const;
    virtual void start_origination(IObserver &observer);
    virtual void leave();
    virtual ResponseHTTP *respond(const RequestHTTP *request);
};

#endif
