#ifndef ERRORPAGEGENERATOR_HPP
#define ERRORPAGEGENERATOR_HPP
#include "../utils/FileCacher.hpp"
#include "FileReader.hpp"

class ErrorPageGenerator : public FileReader {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;

private:
    minor_error error;
    bool should_close_;

    void generate_html();
    ResponseHTTP::header_list_type determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const;

public:
    ErrorPageGenerator(const minor_error &error,
                       const RequestMatchingResult::status_dict_type &match_result,
                       FileCacher &cacher,
                       bool should_close = false);
    ~ErrorPageGenerator();

    void start_origination(IObserver &observer);
    void leave();
    ResponseHTTP *respond(const RequestHTTP *request, bool should_close);
};

#endif
