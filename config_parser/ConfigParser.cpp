#include "ConfigParser.hpp"

#include <fstream>
#include <iostream>

ConfigParser::ConfigParser(const std::string &filepath) {
    std::ifstream fs(filepath);
    if (fs.fail()) {
        throw std::runtime_error("file not opened");
    }
    std::string line;
    while (getline(fs, line)) {
        data_ += (line + '\n');
    }
}

ConfigParser::~ConfigParser() {}

ConfigParser::ConfigParser(const ConfigParser &other) {
    *this = other;
}

ConfigParser &ConfigParser::operator=(const ConfigParser &other) {
    if (this == &other) {
        return *this;
    }
    return *this;
}

/**
 * 1. ファイルを1行ずつ読み込む
 * 2. 先頭のスペースを飛ばす
 * 3. セミコロンで終わっているか確認する
 * 4. 現在のコンテキストの情報を更新する
 * 5. スペースでスプリット
 */
void ConfigParser::parse(void) {
    // ファイルを1行ずつ読み込む
}
