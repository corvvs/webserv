#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
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
    ResponseDataList response_data;
    bool originated_;
    int fd_;

    void write_to_file();
    void close_if_needed();

public:
    FileWriter(const RequestMatchingResult &match_result, const byte_string &content_to_write_);
    ~FileWriter();

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
