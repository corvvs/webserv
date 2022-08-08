#ifndef ECHOER_HPP
#define ECHOER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../socket/SocketUNIX.hpp"
#include "../utils/http.hpp"
#include <fstream>
#include <map>

// [責務]
// - リクエストをそのまま返すこと
class Echoer : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

private:
    byte_string data_;
    bool originated_;
    ResponseDataList response_data;

public:
    Echoer(const RequestMatchingResult &match_result);

    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_origination_started() const;
    bool is_reroutable() const;
    bool is_responsive() const;
    void start_origination(IObserver &observer);
    ResponseHTTP *respond(const RequestHTTP &request);
    void leave();
};

#endif
