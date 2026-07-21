#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include "tests/2d.h"
#include "tests/1d.h"
#include "shared/defs.h"

int main(int argc, char* argv[]) {
    Tester1D tester1;
    tester1.checkCorrectness();
    //tester1.run();
    Tester2D tester2;
    //tester2.checkCorrectness();
    //tester2.run();
    //tester2.visualizePointsets();
    //string s;
    //::cout << std::endl << "done";
}
