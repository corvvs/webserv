#include "Lexer.h"

const CharFilter Lexer::space = " \t\n";
const CharFilter Lexer::left_brace = "{";
const CharFilter Lexer::right_brace = "}";
const CharFilter Lexer::semi_colon = ";";
const CharFilter Lexer::single_quote = "'";
const CharFilter Lexer::double_quote = "\"";
const CharFilter Lexer::delimiter = " \n\t'\"{};";