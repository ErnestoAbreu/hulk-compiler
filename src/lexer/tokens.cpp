#include <vector>
#include <string>

// Token simple con: Type, Value, Line y Collumn
struct Token
{
    TokenType type;
    std::string value;
    int line;
    int column;
};

// Enum con los 'types' posibles par a los tokens
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
    // QUESTION,        // ?
    
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
