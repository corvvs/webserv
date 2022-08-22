#ifndef REDIRECTOR_HPP
#define REDIRECTOR_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
#include <fstream>
#include <map>

class Redirector : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

private:
    bool originated_;
    byte_string redirect_to;
    HTTP::t_status status_code;
    ResponseDataList response_data;

public:
    Redirector(const RequestMatchingResult &match_result);

    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
    void inject_socketlike(ISocketLike *socket_like);
    bool is_originatable() const;
    bool is_reroutable() const;
    HTTP::byte_string reroute_path() const;
    bool is_responsive() const;
    bool is_origination_started() const;
    void start_origination(IObserver &observer);
    void leave();
    ResponseHTTP *respond(const RequestHTTP *request, bool should_close);
};

#endif
