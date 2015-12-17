#include <iostream>

#include "problem.hh"

using namespace scanner;
using namespace problem;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cout << "Usage: benchmark <problems-filename>" << std::endl;
    }
    problem::Parser  parser;

    std::ifstream istream(argv[1]);
    std::string line;
    int line_no = 0;
    while (std::getline(istream, line, '\n')) {
        ++line_no;
        Scanner scanner(&line[0], &line[line.size()]);
        auto ok = parser.parse(scanner);

        if (ok) {
            std::cout << "ok " << line_no << std::endl;

            std::unique_ptr<Problem> problem;
            parser.swap_problems(problem);

            std::cout << "#ranks: " << problem->_ranks.size() << std::endl;

        }
        else {
            std::cout << "problem " << line_no << std::endl;
        }
        break;
    }
}