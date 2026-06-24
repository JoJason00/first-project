#include <iostream>
#include "add.h"

int compute::division(int a, int b) {

    if (b == 0) {
        std::cout<< "Error: Division by zero!" << "\n";
        return 0;
    }

    return a / b;
}