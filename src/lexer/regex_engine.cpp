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
enum class TransitionType
{
    CHAR,   // consume el caracter especifico
    ANY,    // '.' consume cualquier caracter
    EPSILON // '𝛆' no consume nada
};

// Representa una transicion: <tipo, caracter, destino>
struct Transition
{
    TransitionType type;
    char c;        // solo cuando type == CHAR
    StatePtr next; // destimo
};

// Estado de un NFA que tiene las transiciones posibles y si es un estado final
struct State
{
    std::vector<Transition> out;
    bool is_accept = false;
};

//  Un NFA se define como un estado de inicio y uno de final,
// para poder usarlo en la construccion de Thompson
struct NFA
{
    StatePtr start;
    StatePtr accept; // no tiene xq ser un estado aceptado necesariamete ya q cambia con la construccion
};

// =============================== Construcción de Thompson ===================================

/*NFA para un caracter simple
s → t
con transicion de tipo CHAR
*/
NFA char_nfa(char c)
{
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
NFA any_nfa()
{
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
NFA concat_nfa(NFA a, NFA b)
{
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
NFA alt_nfa(NFA a, NFA b)
{
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
NFA star_nfa(NFA a)
{
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
a → a* → t
con transiciones EPSILON
*/
NFA plus_nfa(NFA a)
{
    // A+ = A seguido de A*
    NFA a_star = star_nfa(a);
    return concat_nfa(a, a_star);
}

/*A? == A | ε
. ↗ → ↘
s →a→ t
*/
NFA question_nfa(NFA a)
{

    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    s->out.push_back({TransitionType::EPSILON, 0, a.start});
    s->out.push_back({TransitionType::EPSILON, 0, t});
    a.accept->is_accept = false;
    a.accept->out.push_back({TransitionType::EPSILON, 0, t});
    t->is_accept = true;
    return {s, t};
}

/*Clase de caracteres [abc] o rangos [a-z]
.. ↗ char_1 ↘
s →char_2→t
.. ↘ char_n ↗
Agregar transicion de tipo CHAR para cada caracter de chars */
NFA char_class_nfa(const std::set<char> &chars)
{
    auto s = std::make_shared<State>();
    auto t = std::make_shared<State>();
    for (char c : chars)
    {
        s->out.push_back({TransitionType::CHAR, c, t});
    }
    t->is_accept = true;
    return {s, t};
}

// =========================== Parser de expresiones ===================================

// Se encarga de parsear el patron y generar el NFA correspondiente siguiendo
// la prioridad de (nomeacuerdoelnombre XD) y la construccion de Thompson.
class Parser
{
    const std::string &pattern;
    size_t pos = 0;

    public:
        Parser(const std::string &pat) : pattern(pat) {}

        // Punto de entrada: hace un parseo recursivo descendente :)
        NFA parse()
        {
            NFA expr = parse_expr();
            if (pos != pattern.size())
            {
                throw std::runtime_error("Caracter inesperado al final del patrón");
            }
            return expr;
        }

    private:

        // Consume el caracter actual y deveulve el siguiente en caso de haber, de lo contrario '\0'
        char advance()
        {
            return pos < pattern.size() ? pattern[pos++] : '\0';
        }

        // expr -> term ('|' term)*
        NFA parse_expr()
        {
            NFA left = parse_term();
            while (pos < pattern.size() && pattern[pos] == '|')
            {
                pos++;
                NFA right = parse_term();
                left = alt_nfa(left, right);
            }
            return left;
        }

        // term -> factor*
        NFA parse_term()
        {
            NFA result = parse_factor();
            while (pos < pattern.size() && pattern[pos] != ')' && pattern[pos] != '|')
            {
                NFA next = parse_factor();
                result = concat_nfa(result, next);
            }
            return result;
        }

        // factor -> primary ('*' | '+' | '?')*
        NFA parse_factor()
        {
            NFA base = parse_primary();
            while (pos < pattern.size())
            {
                char op = pattern[pos];
                if (op == '*')
                {
                    base = star_nfa(base);
                    pos++;
                }
                else if (op == '+')
                {
                    base = plus_nfa(base);
                    pos++;
                }
                else if (op == '?')
                {
                    base = question_nfa(base);
                    pos++;
                }
                else
                    break;
            }
            return base;
        }

        // primary -> '(' expr ')' | '.' | char_class | literal
        NFA parse_primary()
        {
            if (pos >= pattern.size())
            {
                throw std::runtime_error("Fin de patrón inesperado");
            }
            char c = pattern[pos];
            if (c == '(')
            { // agrupación
                pos++;
                NFA inside = parse_expr();
                if (pos >= pattern.size() || pattern[pos] != ')')
                {
                    throw std::runtime_error("Falta ')'");
                }
                pos++;
                return inside;
            }
            if (c == '\\')
            {
                char escaped = advance();
                return char_nfa(escaped); // interpreta el carácter escapado literalmente
            }
            if (c == '.')
            { // comodin
                pos++;
                return any_nfa();
            }
            if (c == '[')
            { // clase de caracteres
                pos++;
                std::set<char> chars;
                bool neg = false;
                if (pattern[pos] == '^')
                {
                    neg = true;
                    pos++;
                }
                while (pos < pattern.size() && pattern[pos] != ']')
                {
                    if (pos + 2 < pattern.size() && pattern[pos + 1] == '-')//esto soporta poser [a--]
                    {
                        char start = pattern[pos];
                        char end = pattern[pos + 2];
                        for (char x = start; x <= end; ++x)
                            chars.insert(x);
                        pos += 3;
                    }
                    else
                    {
                        chars.insert(pattern[pos]);
                        pos++;
                    }
                }
                if (pos >= pattern.size() || pattern[pos] != ']')
                {
                    throw std::runtime_error("Falta ']' en la posición " + pos);
                }
                pos++;

                // Aqui me trabe y no se bien como hacer la parte del ^, 
                // pero para las regex de Hulk creo que no hace falta :)

                return char_class_nfa(chars);
            }
            // caracter literal
            pos++;
            return char_nfa(c);
        }
};

// ========================== Simulación del NFA ====================================

// Añade un estado y recorre las transiciones epsilon
void add_state(StatePtr s, std::set<StatePtr> &states)
{
    if (states.count(s))
        return;
    states.insert(s);
    for (auto &tr : s->out)
    {
        if (tr.type == TransitionType::EPSILON)
        {
            add_state(tr.next, states);
        }
    }
}

// Verifica si el NFA acepta todo el texto
bool match(const NFA &nfa, const std::string &text)
{
    std::set<StatePtr> current;
    add_state(nfa.start, current);
    for (char c : text)
    {
        std::set<StatePtr> next_states;
        for (auto s : current)
        {
            for (auto &tr : s->out)
            {
                if ((tr.type == TransitionType::CHAR && tr.c == c) || tr.type == TransitionType::ANY)
                {
                    add_state(tr.next, next_states);
                }
            }
        }
        current = std::move(next_states);
    }
    // al llegar al final del texto, ver si es estado final valido
    for (auto s : current)
    {
        if (s->is_accept)
            return true;
    }
    return false;
}
