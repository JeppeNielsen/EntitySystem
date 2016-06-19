//
//  main.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//
#include "PerformanceTests.hpp"
#include "LogicTests.hpp"
#include "GameWorld.hpp"
#include <iostream>
using namespace Pocket;

int main(int argc, const char * argv[]) {

    std::cout << sizeof(GameObject)<<std::endl;

    LogicTests logicTests;
    logicTests.Run();
    
    PerformanceTests performance;
    performance.Run();
    
    return 0;
}
