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
#include "dynamic_bitset.hpp"

using namespace Pocket;

int main(int argc, const char * argv[]) {

    boost::dynamic_bitset<> components;
    components.resize(128);
    
    components.set(0);
    components.set(3);
    components.set(4);
    
    boost::dynamic_bitset<> system;
    system.resize(128);
    
    system.set(0);
    system.set(3);
    system.set(4);
    
    
    bool contains = (system & components) == system;
    
    std::cout << sizeof(system)<<std::endl;
    
    std::cout << sizeof(GameObject)<<std::endl;

    LogicTests logicTests;
    logicTests.Run();
    
    PerformanceTests performance;
    performance.Run();
    
    return 0;
}
