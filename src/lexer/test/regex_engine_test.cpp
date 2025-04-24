#include <iostream>
#include <vector>
#include "../regex_engine.cpp"

// Tests unitarios
struct Test { std::string pat, txt; bool exp; };
void run_tests() {
    std::vector<Test> tests = {
        {"a","a",true},{"a","b",false},
        {"ab","ab",true},{"ab","a",false},
        {"a|b","a",true},{"a|b","b",true},{"a|b","c",false},
        {"a*","",true},{"a*","aaa",true},{"a*","b",false},
        {"a+","",false},{"a+","a",true},{"a+","aaa",true},
        {".","a",true},{".","",false},
        {"[abc]","a",true},{"[abc]","d",false},
        {"[a-c]","b",true},
        {"(ab)*","",true},{"(ab)*","ab",true},{"(ab)*","abab",true},{"(ab)*","aba",false},
        {"a.b","acb",true},{"a.b","ab",false},
        {"a|bc","bc",true},{"a|bc","a",true},
        {"a(b|c)*d","ad",true},{"a(b|c)*d","abcd",true},{"a(b|c)*d","abd",true},
        {"a(b|c)*d","acd",true},{"a(b|c)*d","abxd",false}
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