#ifndef FILEPOSTER_HPP
#define FILEPOSTER_HPP
#include "../Interfaces.hpp"
#include "../communication/RequestHTTP.hpp"
#include "../utils/http.hpp"
#include <map>

class FilePoster : public IOriginator {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::char_string char_string;
    typedef HTTP::light_string light_string;
    typedef byte_string::size_type size_type;
    typedef HeaderHolderSubpart header_holder_type;

    struct FileEntry {
        byte_string name;
        light_string content;

        MultiPart::CH::ContentType content_type;
        MultiPart::CH::ContentDisposition content_disposition;

        FileEntry(const byte_string &name_, const light_string &content_);
        FileEntry(const byte_string &name_,
                  const light_string &content_,
                  const MultiPart::CH::ContentType &content_type_,
                  const MultiPart::CH::ContentDisposition &content_disposition_);
    };

    typedef std::vector<FileEntry> entry_list_type;

private:
    char_string directory_path_;
    ResponseDataList response_data;
    bool originated_;
    const IContentProvider &content_provider;
    light_string boundary;
    byte_string body;
    entry_list_type entries;

    // ファイルアップロード処理
    void post_files();
    // リクエストターゲットがディレクトリであることを確認
    void check_target_directory();
    // リクエストを解析してFileEntryを取り出す
    void extract_file_entries();
    // リクエストがマルチパートだった場合, それを分解する
    void decompose_multipart(const light_string &body, const light_string &boundary);
    // マルチパートを分解した後のサブパートを解析する
    void analyze_subpart(const light_string &subpart);
    // FileEntryの内容をディスクに書き込む
    void write_file(const FileEntry &file) const;

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
    ResponseHTTP *respond(const RequestHTTP *request);
};

#endif
