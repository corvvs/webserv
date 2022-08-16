#ifndef FILEDELETER_HPP
#define FILEDELETER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
#include <map>

class FileDeleter : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

private:
    char_string file_path_;
    ResponseDataList response_data;
    bool originated_;

    // ターゲットの削除を試みる
    void delete_file();

public:
    FileDeleter(const RequestMatchingResult &match_result);
    ~FileDeleter();

    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_reroutable() const;
    HTTP::byte_string reroute_path() const;
    bool is_responsive() const;
    bool is_origination_started() const;
    void start_origination(IObserver *observer);
    void leave();
    ResponseHTTP *respond(const RequestHTTP *request);
};

#endif
