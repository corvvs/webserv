#ifndef FILEPOSTER_HPP
#define FILEPOSTER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
#include <deque>
#include <map>

class FilePoster : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;
    typedef HeaderHolderSubpart header_holder_type;

    class FileEntry : public ISocketLike {
    public:
        struct Attribute {
            IObserver *observer;
            ISocketLike *master;
            t_fd fd_;

            void close_fd() throw();

            Attribute();
        };

        struct Status {
            bool leaving;
            light_string::size_type written_size;
            bool originated;
            bool is_over;

            Status();
        };

    private:
        Attribute attr;
        Status status;

        void perform_sending(IObserver &observer);
        void retransmit(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
        void prepare_writing();

    public:
        byte_string name;
        light_string content;

        MultiPart::CH::ContentType content_type;
        MultiPart::CH::ContentDisposition content_disposition;

        FileEntry(const byte_string &name_, const light_string &content_, IObserver *observer, ISocketLike *master);
        FileEntry(const byte_string &name_,
                  const light_string &content_,
                  const MultiPart::CH::ContentType &content_type_,
                  const MultiPart::CH::ContentDisposition &content_disposition_,
                  IObserver *observer,
                  ISocketLike *master);
        ~FileEntry();

        virtual t_fd get_fd() const;
        virtual t_port get_port() const;
        virtual void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);
        void leave();
        void start_origination(IObserver &observer);
        bool is_over() const throw();
    };

    typedef std::deque<FileEntry *> entry_list_type;

    struct Attribute {
        IObserver *observer;
        ISocketLike *master;
        t_fd fd_;

        void close_fd() throw();

        Attribute();
    };

    struct Status {
        bool originated;
        bool leaving;
        bool is_responsive;
        FileEntry *running_entry;

        Status();
    };

private:
    char_string directory_path_;
    light_string request_path;
    ResponseDataList response_data;
    const IContentProvider &content_provider;
    light_string boundary;
    byte_string body;
    entry_list_type entries;
    Attribute attr;
    Status status;

    // ファイルアップロード処理
    void prepare_posting();
    // リクエストターゲットがディレクトリであることを確認
    void check_target_directory();
    // リクエストを解析してFileEntryを取り出す
    void extract_file_entries();
    // リクエストがマルチパートだった場合, それを分解する
    void decompose_multipart(const light_string &body, const light_string &boundary);
    // マルチパートを分解した後のサブパートを解析する
    void analyze_subpart(const light_string &subpart);
    void shift_entry();
    ResponseHTTP::header_list_type determine_response_headers(const IResponseDataConsumer::t_sending_mode sm) const;

public:
    FilePoster(const RequestMatchingResult &match_result, const IContentProvider &request);
    ~FilePoster();

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
