#ifndef CHUNKED_BODY
#define CHUNKED_BODY
#include "../utils/LightString.hpp"
#include "../utils/http.hpp"
#include <vector>

// chunked本文
class ChunkedBody {
public:
    typedef HTTP::byte_string byte_string;
    typedef HTTP::light_string light_string;
    // チャンク拡張, トレイラーフィールドは無視する
    typedef byte_string::size_type size_type;

    struct Chunk {
        // チャンク全体
        light_string chunk_str;
        // チャンクの中のchunk-size部分
        light_string size_str;
        // チャンクの中のchunk-data部分
        light_string data_str;
        // size_str の整数化結果
        unsigned int chunk_size;
    };

private:
    std::vector<Chunk> chunks;
    byte_string::size_type body_size;

public:
    ChunkedBody();

    void add_chunk(Chunk &chunk);
    byte_string body() const;
    size_type size() const throw();
    // 完全か = 最終チャンクが受信済みか?
    bool is_complete() const throw();
};

#endif
