#include <iostream>
#include <vector>
#include "../regex_engine.cpp"

// Tests unitarios
struct Test { std::string pat, txt; bool exp; };
void run_tests() {
    std::vector<Test> tests = {
        // Basicos 
        {"a","a",true}  , {"a","b",false}  ,
        {"a?","a",true} , {"a?","",true }  , {"a?","aa",false},
        {"ab","ab",true}, {"ab","a",false} ,
        {"a|b","a",true}, {"a|b","b",true} , {"a|b","c",false},
        {"a*","",true}  , {"a*","aaa",true}, {"a*","b",false},
        {"a+","",false} , {"a+","a",true}  , {"a+","aaa",true},
        {".","a",true}  , {".","",false}   ,

        {"[abc]","a",true} , {"[abc]","d",false},
        {"[a-c]","b",true} , {"[a-c]","d",false},
        {"(ab)*","",true}  , {"(ab)*","ab",true}, {"(ab)*","abab",true}, {"(ab)*","aba",false},
        {"a.b","acb",true} , {"a.b","ab",false} ,
        {"a|bc","bc",true} , {"a|bc","a",true}  ,

        {"a(b|c)*d","ad",true}  , {"a(b|c)*d","abcd",true}  , {"a(b|c)*d","abd",true},
        {"a(b|c)*d","acd",true} , {"a(b|c)*d","abxd",false},

        // Identificadores
        {"[a-zA-Z]+[a-zA-Z0-9_]*","aA_x",true},
        {"[a-zA-Z]+[a-zA-Z0-9_]*","1",false},
        {"[a-zA-Z]+[a-zA-Z0-9_]*","a1_x",true},
        {"[a-zA-Z]+[a-zA-Z0-9_]*","AAa1_x",true},

        // Numero con o sin .  (Descubri un Hack, poner [.] en vez de \. y asi no salen los warning que le pone c++ a los escapes)
        {"[0-9]+([.][0-9]+)?","1",true},
        {"[0-9]+([.][0-9]+)?","123",true},
        {"[0-9]+([.][0-9]+)?",".123",false},
        {"[0-9]+([.][0-9]+)?","123.",false},
        {"[0-9]+([.][0-9]+)?","0.123",true},
        
        // Palabras reservadas (No todas)
        {"if","if",true}, {"else","else",true}, {"while","while",true},

        // Operadores 
        {"[+]","+",true}, {"-","-",true}, {"[*]","*",true}, {"/","/",true}, {"@","@",true},
        {"%","%",true}, {"[|]","|",true}, {"&","&",true}, {"^","^",true}, {"+=","+=",true}, 
        {"-=","-=",true}, {"[*]=","*=",true}, {"@@","@@",true},
        {"/=","/=",true}, {"%=","%=",true}, {":=",":=",true},
        {"[+][+]","++",true}, {"--","--",true}, {"[*][*]","**",true},
    };
    int passed=0;
    for(size_t i=0;i<tests.size();++i) {
        bool ok=false;
        try {
            Parser p(tests[i].pat);
            NFA n = p.parse();
            ok = match(n, tests[i].txt);
        } catch(...) { ok=false; }
        if(ok==tests[i].exp) ++passed;
        else std::cout<<"Test "<<i+1<<" FALLO: /"<<tests[i].pat<<"/ vs \""<<tests[i].txt<<"\" esperaba="<<tests[i].exp<<"\n";
    }
    std::cout<<"Resultados: "<<passed<<"/"<<tests.size()<<" tests pasaron\n";
    if(passed!=tests.size()) exit(1);
}

int main() {
    run_tests();
    std::cout<<"Todos los tests pasaron!"<<std::endl;
    return 0;
}