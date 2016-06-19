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
#include "Bitset.hpp"

using namespace Pocket;

int main(int argc, const char * argv[]) {

    Bitset components;
    Bitset system;
    
    system.Resize(12);
    components.Resize(12);
    
    system.Set(0, true);
    system.Set(1, true);
    system.Set(3, true);
    
    components.Set(0, true);
    components.Set(1, true);
    components.Set(2, true);
    components.Set(2, true);
    
    
    bool contains = system.Contains(components);
    
    std::cout << sizeof(system)<<std::endl;
    
    std::cout << sizeof(GameObject)<<std::endl;

    LogicTests logicTests;
    logicTests.Run();
    
    PerformanceTests performance;
    performance.Run();
    
    return 0;
}
