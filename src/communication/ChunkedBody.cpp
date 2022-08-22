#include "ChunkedBody.hpp"

ChunkedBody::ChunkedBody() : body_size(0) {}

void ChunkedBody::add_chunk(ChunkedBody::Chunk &chunk) {
    chunks.push_back(chunk);
}

ChunkedBody::byte_string ChunkedBody::body() const {
    byte_string rv;
    rv.reserve(size());
    for (unsigned int i = 0; i < chunks.size(); ++i) {
        rv.insert(rv.end(), chunks[i].data_str.begin(), chunks[i].data_str.end());
    }
    return rv;
}

ChunkedBody::size_type ChunkedBody::size() const throw() {
    return body_size;
}

bool ChunkedBody::is_complete() const throw() {
    return chunks.size() > 0 && chunks.back().chunk_size == 0;
}
