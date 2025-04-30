#include <vector>
#include <string>
#include "tokens.cpp"

// Cada par patrón regex y TokenType; orden importante
static const std::vector<std::pair<std::string, TokenType>> RegexsTokens = {
    // Comentarios y espacios

    {"/[*].*[*]/",  TokenType::COMMENT}, //Solo este tipo de momento
    {"[ \t]+",      TokenType::WHITESPACE},
    {"\n+",         TokenType::NEWLINE},

    // Literales
    {"\".*\"",      TokenType::STRING},
    {"'.'",         TokenType::CHAR},
    {"[0-9]*([.][0-9]+)?", TokenType::NUMBER},

    // Palabras clave
    {"if",          TokenType::KW_IF},
    {"else",        TokenType::KW_ELSE},
    {"while",       TokenType::KW_WHILE},
    {"for",         TokenType::KW_FOR},
    {"function",    TokenType::KW_FUNCTION},
    {"let",         TokenType::KW_LET},
    {"in",          TokenType::KW_IN},
    {"type",        TokenType::KW_TYPE},
    {"new",         TokenType::KW_NEW},
    {"inherits",    TokenType::KW_INHERITS},
    {"self",        TokenType::KW_SELF},
    {"base",        TokenType::KW_BASE},
    {"protocol",    TokenType::KW_PROTOCOL},
    {"extends",     TokenType::KW_EXTECNDS},

    {"true",        TokenType::TRUE},
    {"false",       TokenType::FALSE},
    {"Object",      TokenType::T_OBJECT},
    {"Number",      TokenType::T_NUMBER},
    {"Boolean",     TokenType::T_BOOLEAN},
    {"String",      TokenType::T_STRING},
    {"Char",        TokenType::T_CHAR},

    {"is",        TokenType::OP_IS},
    {"as",        TokenType::OP_AS},

    // Operadores (mayores primero)
    {"**",          TokenType::OP_EXPONENT},
    {"++",          TokenType::OP_INCREMENT},
    {"--",          TokenType::OP_DECREMENT},
    {"@@",          TokenType::OP_DOBLE_CONCAT},
    {":=",          TokenType::OP_DESTRUCT_ASSIGN},
    {"+=",          TokenType::OP_PLUS_ASSIGN},
    {"-=",          TokenType::OP_MINUS_ASSIGN},
    {"*=",          TokenType::OP_MULT_ASSIGN},
    {"/=",          TokenType::OP_DIV_ASSIGN},
    {"%=",          TokenType::OP_MOD_ASSIGN},
    {"==",          TokenType::OP_EQUAL},
    {"!=",          TokenType::OP_NOT_EQUAL},
    {"<=",          TokenType::OP_LESS_EQ},
    {">=",          TokenType::OP_GREATER_EQ},
    {"=>",          TokenType::ARROW},
    {"->",          TokenType::RT_ARROW},
    
    // Símbolos sencillos
    {"=",           TokenType::OP_ASSIGN},
    {"<",           TokenType::OP_LESS},
    {">",           TokenType::OP_GREATER},
    {"+",           TokenType::OP_PLUS},
    {"-",           TokenType::OP_MINUS},
    {"*",           TokenType::OP_MULTIPLY},
    {"/",           TokenType::OP_DIVIDE},
    {"%",           TokenType::OP_MODULE},
    {"^",           TokenType::OP_EXPONENT},
    {"&",           TokenType::OP_AND},
    {"|",           TokenType::OP_OR},
    {"!",           TokenType::OP_NOT},
    {"@",           TokenType::OP_CONCAT},
    {";",           TokenType::SEMICOLON},
    {":",           TokenType::COLON},
    {"," ,          TokenType::COMMA},
    {".",           TokenType::DOT},
    {"(",           TokenType::LPAREN},
    {")",           TokenType::RPAREN},
    {"{",           TokenType::LBRACE},
    {"}",           TokenType::RBRACE},
    {"[",           TokenType::LBRACKET},
    {"]",           TokenType::RBRACKET},

    // Identificadores al final
    {"[a-zA-Z_][a-zA-Z0-9_]*", TokenType::IDENTIFIER}
};