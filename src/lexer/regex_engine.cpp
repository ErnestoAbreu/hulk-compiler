#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <memory>
#include <stack>

/*  Motor de expresiones regulares basado en la construcción de Thompson
    Soporta:
        literales            (cualquier caracter),
        concatenaciones      (regex1 regex2 ...),
        alternar             (regex1|regex2|...),
        clasura de Kleene    (regex1*),
        uno o mas            (regex1+),
        clases de caracteres ([]),
        agrupaciones         (()),
        comodin              (.)
*/

// ======================== Estructuras de datos para el NFA ======================================

struct State;
using StatePtr = std::shared_ptr<State>;

// Su nombre esta claro XD
enum class TransitionType {
    CHAR,      // consume el caracter especifico
    ANY,       // '.' consume cualquier caracter
    EPSILON    // '𝛆' no consume nada
};

// Representa una transicion: <tipo, caracter, destino>
struct Transition {
    TransitionType type;
    char c;             // solo cuando type == CHAR
    StatePtr next;      // destimo
};

// Estado de un NFA que tiene las transiciones posibles y si es un estado final 
struct State {
    std::vector<Transition> out;
    bool is_accept = false;
};

//  Un NFA se define como un estado de inicio y uno de final,
// para poder usarlo en la construccion de Thompson
struct NFA {
    StatePtr start;
    StatePtr accept; //no tiene xq ser un estado aceptado necesariamete ya q cambia con la construccion
};


// =============================== Construcción de Thompson ===================================

/*NFA para un caracter simple
s → t
con transicion de tipo CHAR
*/ 
NFA char_nfa(char c) {
    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    s->out.push_back({TransitionType::CHAR, c, t});
    t->is_accept = true;
    return {s, t};
}

/*NFA para comodin '.'
s → t
con transicion de tipo ANY
*/ 
NFA any_nfa() {
    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    s->out.push_back({TransitionType::ANY, 0, t});
    t->is_accept = true;
    return {s, t};
}

/*Concatenacion: NFA que representa: a seguido de b
a → b
con transicion EPSILON
*/ 
NFA concat_nfa(NFA a, NFA b) {
    a.accept->is_accept = false;
    a.accept->out.push_back({TransitionType::EPSILON, 0, b.start});
    return {a.start, b.accept};
}

/* Alternacion: a | b
 ↗ a ↘
s . . . t
 ↘ b ↗
con transiciones EPSILON
*/
NFA alt_nfa(NFA a, NFA b) {
    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    s->out.push_back({TransitionType::EPSILON, 0, a.start});
    s->out.push_back({TransitionType::EPSILON, 0, b.start});
    a.accept->is_accept = false;
    b.accept->is_accept = false;
    a.accept->out.push_back({TransitionType::EPSILON, 0, t});
    b.accept->out.push_back({TransitionType::EPSILON, 0, t});
    t->is_accept = true;
    return {s, t};
}

/*Kleene star: a*
↗ → → ↘ 
s → a → t
. . . ↻
con transiciones EPSILON
*/ 
NFA star_nfa(NFA a) {
    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    // desde nuevo inicio a A y a nuevo fin
    s->out.push_back({TransitionType::EPSILON, 0, a.start});
    s->out.push_back({TransitionType::EPSILON, 0, t});
    // desde fin de A a inicio de A y a nuevo fin
    a.accept->is_accept = false;
    a.accept->out.push_back({TransitionType::EPSILON, 0, a.start});
    a.accept->out.push_back({TransitionType::EPSILON, 0, t});
    t->is_accept = true;
    return {s, t};
}

/*Uno o más: a+
s → a → t
. . . ↻
con transiciones EPSILON
*/ 
NFA plus_nfa(NFA a) {
    // A+ = A seguido de A*
    NFA a_star = star_nfa(a);
    return concat_nfa(a, a_star);
}

/*Clase de caracteres [abc] o rangos [a-z]
.. ↗ char_1 ↘ 
s →char_2→t
.. ↘ char_n ↗
Agregar transicion de tipo CHAR para cada caracter de chars */ 
NFA char_class_nfa(const std::set<char>& chars) {
    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    for (char c: chars) {
        s->out.push_back({TransitionType::CHAR, c, t});
    }
    t->is_accept = true;
    return {s, t};
}

