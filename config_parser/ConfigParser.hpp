#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <map>
#include <string>
#include <vector>

class ConfigParser {
public:
    enum context_type { CTX_HTTP, CTX_SERVER, CTX_LOCATION };

    enum directive_type {
        DIR_HTTP,
        DIR_SERVER,
        DIR_LISTEN,
        DIR_SERVER_NAME,
        DIR_ERROR_PAGE,
        DIR_LOCATION,
        DIR_INDEX,
        DIR_AUTOINDEX,
        DIR_RETURN,
        DIR_ROOT,
        DIR_LIMIT_EXCEPT,
        DIR_UPLOAD_STORE
    };

    // member variables
    std::string data_;

    /**
     * vector<string> v;
     * v[0] = "server1";
     * v[1] = "server2";
     * parsed_data_["server_name"] = v;
     */
    std::map<std::string, std::vector<std::string>> parsed_data_;

    // member functions
    ConfigParser(const std::string &filepath = "./default.conf");
    ConfigParser(const ConfigParser &other);
    ConfigParser &operator=(const ConfigParser &other);
    ~ConfigParser(void);

    void parse(void);

private:
};

#endif
