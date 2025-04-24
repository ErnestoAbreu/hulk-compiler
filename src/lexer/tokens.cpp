#include <vector>
#include <string>

// Token simple con: Type, Value, Line y Collumn
struct tokens
{
    TokenType type;
    std::string value;
    int line;
    int column;
};

// Enum con los 'types' posibles par a los tokens
enum class TokenType {

    // Identificadores y literales
    IDENTIFIER,     // nombre de variable/función
    NUMBER,         // número entero o decimal
    STRING,         // cadena entre comillas
    CHAR,           // carácter entre comillas simples
    
    // Palabras clave
    KEYWORD_IF,     // if
    KEYWORD_ELSE,   // else
    KEYWORD_WHILE,  // while
    KEYWORD_FOR,    // for
    KEYWORD_RETURN, // return
    KEYWORD_FUNC,   // function
    KEYWORD_LET,    // let
    KEYWORD_CONST,  // const
    KEYWORD_TRUE,   // true
    KEYWORD_FALSE,  // false
    
    // Operadores aritméticos
    OP_PLUS,        // +
    OP_MINUS,       // -
    OP_MULTIPLY,    // *
    OP_DIVIDE,      // /
    OP_MODULO,      // %
    OP_INCREMENT,   // ++
    OP_DECREMENT,   // --
    OP_EXPONENT,    // **
    
    // Operadores de asignación
    OP_ASSIGN,      // =
    OP_PLUS_ASSIGN, // +=
    OP_MINUS_ASSIGN, // -=
    OP_MULT_ASSIGN, // *=
    OP_DIV_ASSIGN,  // /=
    OP_MOD_ASSIGN,  // %=
    
    // Operadores de comparación
    OP_EQUAL,       // ==
    OP_NOT_EQUAL,   // !=
    OP_STRICT_EQ,   // ===
    OP_STRICT_NEQ,  // !==
    OP_LESS,        // <
    OP_LESS_EQ,     // <=
    OP_GREATER,     // >
    OP_GREATER_EQ,  // >=
    
    // Operadores lógicos
    OP_AND,         // &&
    OP_OR,          // ||
    OP_NOT,         // !
    
    // Operadores a nivel de bits
    // OP_BIT_AND,     // &
    // OP_BIT_OR,      // |
    // OP_BIT_XOR,     // ^
    // OP_BIT_NOT,     // ~
    // OP_LEFT_SHIFT,  // <<
    // OP_RIGHT_SHIFT, // >>
    
    // Puntuación
    SEMICOLON,      // ;
    COLON,          // :
    COMMA,          // ,
    DOT,            // .
    // QUESTION,       // ?
    
    // Delimitadores
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    
    // Otros
    ARROW,          // =>
    // SPREAD,         // ...
    COMMENT,        // // o /* y */
    WHITESPACE,     // espacios, tabs
    NEWLINE,        // \n
    END_OF_FILE,    // fin de archivo
    UNKNOWN         // token no reconocido
};
