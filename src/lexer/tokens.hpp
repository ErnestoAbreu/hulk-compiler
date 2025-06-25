#ifndef HULK_TOKENS_HPP
#define HULK_TOKENS_HPP 1

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace hulk {

namespace lexer {

// Enum con los tipos posibles par a los tokens
enum token_type {

  // Identificadores y literales
  IDENTIFIER,  // nombre de variable/funcion
  NUMBER,      // numero entero o decimal
  STRING,      // cadena entre comillas dobles
  CHAR,        // caracter unico entre comillas simples
  TRUE,        // true
  FALSE,       // false

  // Tipos reservados
  T_OBJECT,   // Object  todos los tipos heredan de este
  T_NUMBER,   // Number
  T_BOOLEAN,  // Boolean
  T_STRING,   // String
  T_CHAR,     // Char
  T_NULL,     // Null

  // Palabras clave
  KW_IF,    // if
  KW_ELIF,  // elif
  KW_ELSE,  // else

  KW_WHILE,  // while
  KW_FOR,    // for

  KW_FUNCTION,  // function
  KW_LET,       // let
  KW_IN,        // in

  KW_TYPE,      // type
  KW_NEW,       // new
  KW_INHERITS,  // inherits
  KW_SELF,      // aunque segun la definicion de Hulk no es una palabra rservada
  KW_BASE,  // base : se refiere a la implementación del padre (o el antepasado
            // más cercano que tiene una implementación)
  KW_PROTOCOL,  // protocol : interfaces de toda la vida
  KW_EXTENDS,  // extends : para decir que un protocolo implementa otro
                // protocolo

  // Operadores aritméticos
  OP_PLUS,      // +
  OP_MINUS,     // -
  OP_MULTIPLY,  // * tambien se usa para decir que una variabel es un Iterable
                // Ej: numbers = Number* se usa para decir number es un Iterable
                // de Number
  OP_DIVIDE,    // /
  OP_MODULE,    // %
  OP_INCREMENT,     // ++
  OP_DECREMENT,     // --
  OP_EXPONENT,      // ** o ^
  OP_CONCAT,        // @
  OP_DOBLE_CONCAT,  // @@ lo cual es equivalente a @ " " @

  // Operadores de asignacion
  OP_ASSIGN,           // =
  OP_DESTRUCT_ASSIGN,  // := asignacion destructiva que se usa para reasignarle
                       // un valor a una variable
  OP_PLUS_ASSIGN,      // +=
  OP_MINUS_ASSIGN,     // -=
  OP_MULT_ASSIGN,      // *=
  OP_DIV_ASSIGN,       // /=
  OP_MOD_ASSIGN,       // %=

  // Operadores de comparacion
  OP_EQUAL,       // ==
  OP_NOT_EQUAL,   // !=
  OP_LESS,        // <
  OP_LESS_EQ,     // <=
  OP_GREATER,     // >
  OP_GREATER_EQ,  // >=

  // Operador de tipo
  OP_IS,  // is : para comparar
  OP_AS,  // as : tratar como un tipo estatico dado (da errores en tiempo de
          // ejecucion)

  // Operadores logicos
  OP_AND,  // &
  OP_OR,   // | tambien se usa en la compresion de listas
  OP_NOT,  // !

  // Puntuacion
  SEMICOLON,  // ;
  COLON,      // :
  COMMA,      // ,
  DOT,        // .

  // Delimitadores
  LPAREN,    // (
  RPAREN,    // )
  LBRACE,    // {
  RBRACE,    // }
  LBRACKET,  // [
  RBRACKET,  // ]

  // Funciones reservadas *(No se si hacen falta)
  F_RANGE,   // range
  F_RANDOM,  // rand
  F_PRINT,   // print
  F_SIN,     // sin
  F_COS,     // cos
  F_LOG,     // log
  F_COT,     // cot
  F_TAN,     // tan

  // constants
  F_PI,

  // Otros
  ARROW,        // =>
  RT_ARROW,     // -> dice el Tipo de Retorno
  COMMENT,      // // o /* y */
  WHITESPACE,   // espacios, tabs
  NEWLINE,      // \n
  END_OF_FILE,  // fin de archivo
  UNKNOWN       // token no reconocido
};

const std::unordered_map<token_type, std::string> mapping = {
#define TOKEN_ENTRY(name) {token_type::name, #name}
    TOKEN_ENTRY(COMMENT),
    TOKEN_ENTRY(WHITESPACE),
    TOKEN_ENTRY(NEWLINE),
    TOKEN_ENTRY(STRING),
    TOKEN_ENTRY(CHAR),
    TOKEN_ENTRY(NUMBER),
    TOKEN_ENTRY(IDENTIFIER),
    TOKEN_ENTRY(TRUE),
    TOKEN_ENTRY(FALSE),

    TOKEN_ENTRY(T_NULL),
    TOKEN_ENTRY(T_OBJECT),
    TOKEN_ENTRY(T_NUMBER),
    TOKEN_ENTRY(T_BOOLEAN),
    TOKEN_ENTRY(T_STRING),
    TOKEN_ENTRY(T_CHAR),

    TOKEN_ENTRY(KW_IF),
    TOKEN_ENTRY(KW_ELIF),
    TOKEN_ENTRY(KW_ELSE),
    TOKEN_ENTRY(KW_WHILE),
    TOKEN_ENTRY(KW_FOR),
    TOKEN_ENTRY(KW_FUNCTION),
    TOKEN_ENTRY(KW_LET),
    TOKEN_ENTRY(KW_IN),
    TOKEN_ENTRY(KW_TYPE),
    TOKEN_ENTRY(KW_NEW),
    TOKEN_ENTRY(KW_INHERITS),
    TOKEN_ENTRY(KW_SELF),
    TOKEN_ENTRY(KW_BASE),
    TOKEN_ENTRY(KW_PROTOCOL),
    TOKEN_ENTRY(KW_EXTENDS),

    TOKEN_ENTRY(OP_PLUS),
    TOKEN_ENTRY(OP_MINUS),
    TOKEN_ENTRY(OP_MULTIPLY),
    TOKEN_ENTRY(OP_DIVIDE),
    TOKEN_ENTRY(OP_MODULE),
    TOKEN_ENTRY(OP_INCREMENT),
    TOKEN_ENTRY(OP_DECREMENT),
    TOKEN_ENTRY(OP_EXPONENT),
    TOKEN_ENTRY(OP_CONCAT),
    TOKEN_ENTRY(OP_DOBLE_CONCAT),

    TOKEN_ENTRY(OP_ASSIGN),
    TOKEN_ENTRY(OP_DESTRUCT_ASSIGN),
    TOKEN_ENTRY(OP_PLUS_ASSIGN),
    TOKEN_ENTRY(OP_MINUS_ASSIGN),
    TOKEN_ENTRY(OP_MULT_ASSIGN),
    TOKEN_ENTRY(OP_DIV_ASSIGN),
    TOKEN_ENTRY(OP_MOD_ASSIGN),

    TOKEN_ENTRY(OP_EQUAL),
    TOKEN_ENTRY(OP_NOT_EQUAL),
    TOKEN_ENTRY(OP_LESS),
    TOKEN_ENTRY(OP_LESS_EQ),
    TOKEN_ENTRY(OP_GREATER),
    TOKEN_ENTRY(OP_GREATER_EQ),

    TOKEN_ENTRY(OP_IS),
    TOKEN_ENTRY(OP_AS),
    TOKEN_ENTRY(OP_AND),
    TOKEN_ENTRY(OP_OR),
    TOKEN_ENTRY(OP_NOT),

    TOKEN_ENTRY(SEMICOLON),
    TOKEN_ENTRY(COLON),
    TOKEN_ENTRY(COMMA),
    TOKEN_ENTRY(DOT),

    TOKEN_ENTRY(LPAREN),
    TOKEN_ENTRY(RPAREN),
    TOKEN_ENTRY(LBRACE),
    TOKEN_ENTRY(RBRACE),
    TOKEN_ENTRY(LBRACKET),
    TOKEN_ENTRY(RBRACKET),

    TOKEN_ENTRY(ARROW),
    TOKEN_ENTRY(RT_ARROW),
    TOKEN_ENTRY(F_RANGE),
    TOKEN_ENTRY(F_RANDOM),
    TOKEN_ENTRY(F_PRINT),
    TOKEN_ENTRY(F_SIN),
    TOKEN_ENTRY(F_COS),
    TOKEN_ENTRY(F_LOG),
    TOKEN_ENTRY(F_COT),
    TOKEN_ENTRY(F_TAN),
    TOKEN_ENTRY(F_PI),

    TOKEN_ENTRY(END_OF_FILE),
    TOKEN_ENTRY(UNKNOWN)
#undef TOKEN_ENTRY
};

std::string token_type_to_string(token_type type) {
  auto it = mapping.find(type);
  return it != mapping.end() ? it->second : "???";
}

using literal = std::variant<std::nullptr_t, std::string, double, bool>;

// Token simple con: Type, Value, Line y Column
struct token {
  token_type type;
  std::string lexeme;
  literal value;
  unsigned int line, column;
  
  token() {}
  token(const token_type _type, std::string _lexeme,
                 const literal &_value, int _line, int _column)
      : type(_type),
        lexeme(_lexeme),
        value(std::move(_value)),
        line(_line),
        column(_column) {}

  token_type get_type() const { return type; }

  std::pair<unsigned int, unsigned int> get_location() const {
    return {line, column};
  }

  std::string get_lexeme() const { return lexeme; }
  literal get_literal() const { return value; }

  std::string to_string() const {
    return token_type_to_string(type) + " " + lexeme + " at " +
           "[line = " + std::to_string(line) +
           ", column = " + std::to_string(column) + "]";
  }
};

// Pares de <regex, token_type> orden importante
static const std::vector<std::pair<std::string, token_type>> regexs_tokens = {
    // Comentarios y espacios
    {"// *([ -\t]|[\v-~])*", token_type::COMMENT},
    {"/[*].*[*]/", token_type::COMMENT},
    {"[ \t]+", token_type::WHITESPACE},
    {"[\n]+", token_type::NEWLINE},

    // Literales
    {"\"([\t- ]|[#-~])*\"",
     token_type::STRING},  // falta evitar que se ponga \n y \" en .*
    {"'.'", token_type::CHAR},
    {"Null", token_type::T_NULL},

    {"[0-9]*([.][0-9]+)?", token_type::NUMBER},

    // Palabras clave
    {"if", token_type::KW_IF},
    {"elif", token_type::KW_ELIF},
    {"else", token_type::KW_ELSE},
    {"while", token_type::KW_WHILE},
    {"for", token_type::KW_FOR},
    {"function", token_type::KW_FUNCTION},
    {"let", token_type::KW_LET},
    {"in", token_type::KW_IN},
    {"type", token_type::KW_TYPE},
    {"new", token_type::KW_NEW},
    {"inherits", token_type::KW_INHERITS},
    // {"base", token_type::KW_BASE},
    {"protocol", token_type::KW_PROTOCOL},
    {"extends", token_type::KW_EXTENDS},

    {"true", token_type::TRUE},
    {"false", token_type::FALSE},
    // {"Object", token_type::T_OBJECT},
    // {"Number", token_type::T_NUMBER},
    // {"Boolean", token_type::T_BOOLEAN},
    // {"String", token_type::T_STRING},
    // {"Char", token_type::T_CHAR},

    {"is", token_type::OP_IS},
    {"as", token_type::OP_AS},

    // Operadores (mayores primero)
    {"[*][*]", token_type::OP_EXPONENT},
    {"[+][+]", token_type::OP_INCREMENT},
    {"--", token_type::OP_DECREMENT},
    {"@@", token_type::OP_DOBLE_CONCAT},
    {":=", token_type::OP_DESTRUCT_ASSIGN},
    {"[+]=", token_type::OP_PLUS_ASSIGN},
    {"-=", token_type::OP_MINUS_ASSIGN},
    {"[*]=", token_type::OP_MULT_ASSIGN},
    {"/=", token_type::OP_DIV_ASSIGN},
    {"%=", token_type::OP_MOD_ASSIGN},
    {"==", token_type::OP_EQUAL},
    {"!=", token_type::OP_NOT_EQUAL},
    {"<=", token_type::OP_LESS_EQ},
    {">=", token_type::OP_GREATER_EQ},

    {"=>", token_type::ARROW},
    {"->", token_type::RT_ARROW},

    // Símbolos sencillos
    {"=", token_type::OP_ASSIGN},
    {"<", token_type::OP_LESS},
    {">", token_type::OP_GREATER},
    {"[+]", token_type::OP_PLUS},
    {"-", token_type::OP_MINUS},
    {"[*]", token_type::OP_MULTIPLY},
    {"/", token_type::OP_DIVIDE},
    {"%", token_type::OP_MODULE},
    {"^", token_type::OP_EXPONENT},
    {"&", token_type::OP_AND},
    {"[|]", token_type::OP_OR},
    {"!", token_type::OP_NOT},
    {"@", token_type::OP_CONCAT},

    {";", token_type::SEMICOLON},
    {":", token_type::COLON},
    {",", token_type::COMMA},
    {"[.]", token_type::DOT},
    {"[(]", token_type::LPAREN},
    {")", token_type::RPAREN},
    {"{", token_type::LBRACE},
    {"}", token_type::RBRACE},
    {"[[]", token_type::LBRACKET},
    {"]", token_type::RBRACKET},

    // Identificadores al final
    {"[a-zA-Z]+[a-zA-Z0-9_]*", token_type::IDENTIFIER}};

}  // namespace lexer

}  // namespace hulk

#endif  // HULK_TOKENS_HPP