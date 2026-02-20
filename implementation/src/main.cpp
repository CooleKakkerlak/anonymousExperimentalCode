#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include "tests/2d.h"
#include "tests/1d.h"
#include "shared/defs.h"
#include "2D/mode_query.h"

int main() {    
    Tester1D tester1;
    //tester1.checkCorrectness();
    tester1.run();
    Tester2D tester2;
    //tester2.checkCorrectness();
    tester2.run();
    string s;
    std::cout << "waiting for user input";
    std::cin >> s;
}
