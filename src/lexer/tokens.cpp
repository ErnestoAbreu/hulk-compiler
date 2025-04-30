#include <vector>
#include <string>

// Enum con los tipos posibles par a los tokens
enum class TokenType {

    // Identificadores y literales
    IDENTIFIER,         // nombre de variable/funcion
    NUMBER,             // numero entero o decimal
    STRING,             // cadena entre comillas dobles
    CHAR,               // caracter unico entre comillas simples
    TRUE,               // true
    FALSE,              // false

    // Tipos reservados 
    T_OBJECT,           // Object  todos los tipos heredan de este
    T_NUMBER,           // Number
    T_BOOLEAN,          // Boolean
    T_STRING,           // String
    T_CHAR,             // Char

    // Palabras clave
    KW_IF,              // if
    KW_ELIF,            // elif
    KW_ELSE,            // else

    KW_WHILE,           // while
    KW_FOR,             // for

    KW_FUNCTION,        // function
    KW_LET,             // let
    KW_IN,              // in

    KW_TYPE,            // type
    KW_NEW,             // new
    KW_INHERITS,        // inherits
    KW_SELF,            // aunque segun la definicion de Hulk no es una palabra rservada
    KW_BASE,            // base : se refiere a la implementación del padre (o el antepasado más cercano que tiene una implementación)
    KW_PROTOCOL,        // protocol : interfaces de toda la vida
    KW_EXTECNDS,        // extends : para decir que un protocolo implementa otro protocolo 

    // Operadores aritméticos
    OP_PLUS,            // +
    OP_MINUS,           // -
    OP_MULTIPLY,        // * tambien se usa para decir que una variabel es un Iterable Ej: numbers = Number* se usa para decir number es un Iterable de Number
    OP_DIVIDE,          // /
    OP_MODULE,          // %
    OP_INCREMENT,       // ++
    OP_DECREMENT,       // --
    OP_EXPONENT,        // ** o ^
    OP_CONCAT,          // @
    OP_DOBLE_CONCAT,    // @@ lo cual es equivalente a @ " " @
    
    // Operadores de asignacion
    OP_ASSIGN,          // =
    OP_DESTRUCT_ASSIGN, // := asignacion destructiva que se usa para reasignarle un valor a una variable
    OP_PLUS_ASSIGN,     // +=
    OP_MINUS_ASSIGN,    // -=
    OP_MULT_ASSIGN,     // *=
    OP_DIV_ASSIGN,      // /=
    OP_MOD_ASSIGN,      // %=
    
    // Operadores de comparacion
    OP_EQUAL,           // ==
    OP_NOT_EQUAL,       // !=
    OP_LESS,            // <
    OP_LESS_EQ,         // <=
    OP_GREATER,         // >
    OP_GREATER_EQ,      // >=

    // Operador de tipo
    OP_IS,              // is : para comparar
    OP_AS,              // as : tratar como un tipo estatico dado (da errores en tiempo de ejecucion)
    
    // Operadores logicos
    OP_AND,             // &
    OP_OR,              // | tambien se usa en la compresion de listas
    OP_NOT,             // !
    
    // Puntuacion
    SEMICOLON,          // ;
    COLON,              // :
    COMMA,              // ,
    DOT,                // .
    
    // Delimitadores
    LPAREN,             // (
    RPAREN,             // )
    LBRACE,             // {
    RBRACE,             // }
    LBRACKET,           // [
    RBRACKET,           // ]

    // Funciones reservadas *(No se si hacen falta)
    F_RANGE,            // range
    F_RANDOM,           // rand
    F_PRINT,            // print
    F_SIN,              // sin
    F_COS,              // cos
    F_LOG,              // log
    F_COT,              // cot
    F_TAN,              // tan

    // constants
    F_PI, 
    
    // Otros
    ARROW,              // => 
    RT_ARROW,           // -> dice el Tipo de Retorno
    COMMENT,            // // o /* y */
    WHITESPACE,         // espacios, tabs
    NEWLINE,            // \n
    END_OF_FILE,        // fin de archivo
    UNKNOWN             // token no reconocido
};

// Token simple con: Type, Value, Line y Collumn
struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType type, std::string value, int line, int column): type(type), value(value), line(line), column(column) {}
};

// Pares de <regex, TokenType> orden importante
static const std::vector<std::pair<std::string, TokenType>> RegexsTokens = {
    // Comentarios y espacios

    {"/[*].*[*]/",  TokenType::COMMENT}, //Solo este tipo de momento
    {"[ \t]+",      TokenType::WHITESPACE},
    {"[\n]+",       TokenType::NEWLINE},

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

    {"is",          TokenType::OP_IS},
    {"as",          TokenType::OP_AS},

    // Operadores (mayores primero)
    {"[*][*]",      TokenType::OP_EXPONENT},
    {"[+][+]",      TokenType::OP_INCREMENT},
    {"--",          TokenType::OP_DECREMENT},
    {"@@",          TokenType::OP_DOBLE_CONCAT},
    {":=",          TokenType::OP_DESTRUCT_ASSIGN},
    {"[+]=",        TokenType::OP_PLUS_ASSIGN},
    {"-=",          TokenType::OP_MINUS_ASSIGN},
    {"[*]=",        TokenType::OP_MULT_ASSIGN},
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
    {"[+]",         TokenType::OP_PLUS},
    {"-",           TokenType::OP_MINUS},
    {"[*]",         TokenType::OP_MULTIPLY},
    {"/",           TokenType::OP_DIVIDE},
    {"%",           TokenType::OP_MODULE},
    {"^",           TokenType::OP_EXPONENT},
    {"&",           TokenType::OP_AND},
    {"[|]",         TokenType::OP_OR},
    {"!",           TokenType::OP_NOT},
    {"@",           TokenType::OP_CONCAT},

    {";",           TokenType::SEMICOLON},
    {":",           TokenType::COLON},
    {"," ,          TokenType::COMMA},
    {"[.]",         TokenType::DOT},
    {"[(]",         TokenType::LPAREN},
    {")",           TokenType::RPAREN},
    {"{",           TokenType::LBRACE},
    {"}",           TokenType::RBRACE},
    {"[[]",         TokenType::LBRACKET},
    {"]",           TokenType::RBRACKET},

    // Identificadores al final
    {"[a-zA-Z]+[a-zA-Z0-9_]*", TokenType::IDENTIFIER}
};

//
std::string TokenType_to_string(TokenType type){
    switch(type){
        case TokenType::COMMENT: return "COMMENT";
        case TokenType::WHITESPACE : return "WHITESPACE";
        case TokenType::NEWLINE : return "NEWLINE";
        case TokenType::STRING : return "STRING";
        case TokenType::CHAR : return "CHAR";
        case TokenType::NUMBER : return "NUMBER";
        case TokenType::KW_IF : return "KW_IF";
        case TokenType::KW_ELSE : return "KW_ELSE";
        case TokenType::KW_WHILE : return "KW_WHILE";
        case TokenType::KW_FOR : return "KW_FOR";
        case TokenType::KW_FUNCTION : return "KW_FUNCTION";
        case TokenType::KW_LET : return "KW_LET";
        case TokenType::KW_IN : return "KW_IN";
        case TokenType::KW_TYPE : return "KW_TYPE";
        case TokenType::KW_NEW : return "KW_NEW";
        case TokenType::KW_INHERITS : return "KW_INHERITS";
        case TokenType::KW_SELF : return "KW_SELF";
        case TokenType::KW_BASE : return "KW_BASE";
        case TokenType::KW_PROTOCOL : return "KW_PROTOCOL";
        case TokenType::KW_EXTECNDS : return "KW_EXTECNDS";
        case TokenType::TRUE : return "TRUE";
        case TokenType::FALSE : return "FALSE";
        case TokenType::T_OBJECT : return "T_OBJECT";
        case TokenType::T_NUMBER : return "T_NUMBER";
        case TokenType::T_BOOLEAN : return "T_BOOLEAN";
        case TokenType::T_STRING : return "T_STRING";
        case TokenType::T_CHAR : return "T_CHAR";
        case TokenType::OP_IS : return "OP_IS";
        case TokenType::OP_AS : return "OP_AS";
        case TokenType::OP_EXPONENT : return "OP_EXPONENT";
        case TokenType::OP_INCREMENT : return "OP_INCREMENT";
        case TokenType::OP_DECREMENT : return "OP_DECREMENT";
        case TokenType::OP_DOBLE_CONCAT : return "OP_DOBLE_CONCAT";
        case TokenType::OP_DESTRUCT_ASSIGN : return "OP_DESTRUCT_ASSIGN";
        case TokenType::OP_PLUS_ASSIGN : return "OP_PLUS_ASSIGN";
        case TokenType::OP_MINUS_ASSIGN : return "OP_MINUS_ASSIGN";
        case TokenType::OP_MULT_ASSIGN : return "OP_MULT_ASSIGN";
        case TokenType::OP_DIV_ASSIGN : return "OP_DIV_ASSIGN";
        case TokenType::OP_MOD_ASSIGN : return "OP_MOD_ASSIGN";
        case TokenType::OP_EQUAL : return "OP_EQUAL";
        case TokenType::OP_NOT_EQUAL : return "OP_NOT_EQUAL";
        case TokenType::OP_LESS_EQ : return "OP_LESS_EQ";
        case TokenType::OP_GREATER_EQ : return "OP_GREATER_EQ";
        case TokenType::ARROW : return "ARROW";
        case TokenType::RT_ARROW : return "RT_ARROW";
        case TokenType::OP_ASSIGN : return "OP_ASSIGN";
        case TokenType::OP_LESS : return "OP_LESS";
        case TokenType::OP_GREATER : return "OP_GREATER";
        case TokenType::OP_PLUS : return "OP_PLUS";
        case TokenType::OP_MINUS : return "OP_MINUS";
        case TokenType::OP_MULTIPLY : return "OP_MULTIPLY";
        case TokenType::OP_DIVIDE : return "OP_DIVIDE";
        case TokenType::OP_MODULE : return "OP_MODULE";
        case TokenType::OP_AND : return "OP_AND";
        case TokenType::OP_OR : return "OP_OR";
        case TokenType::OP_NOT : return "OP_NOT";
        case TokenType::OP_CONCAT : return "OP_CONCAT";
        case TokenType::SEMICOLON : return "SEMICOLON";
        case TokenType::COLON : return "COLON";
        case TokenType::COMMA : return "COMMA";
        case TokenType::DOT : return "DOT";
        case TokenType::LPAREN : return "LPAREN";
        case TokenType::RPAREN : return "RPAREN";
        case TokenType::LBRACE : return "LBRACE";
        case TokenType::RBRACE : return "RBRACE";
        case TokenType::LBRACKET : return "LBRACKET";
        case TokenType::RBRACKET : return "RBRACKET";
        case TokenType::IDENTIFIER : return "IDENTIFIER";
        default : return "Unknown";
    }
}