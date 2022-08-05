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

private:
    char_string file_path_;
    bool originated_;
    ResponseDataList response_data;
    int fd_;

    // ファイルからデータを読み出しておく
    void read_from_file();
    void close_if_needed();

public:
    FileReader(const RequestMatchingResult &match_result);
    ~FileReader();

    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_reroutable() const;
    bool is_responsive() const;
    bool is_origination_started() const;
    void start_origination(IObserver &observer);
    void leave();
    ResponseHTTP *respond(const RequestHTTP &request);
};

#endif
