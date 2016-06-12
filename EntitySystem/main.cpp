//
//  main.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "LogicTests.hpp"
#include "PerformanceTests.hpp"

int main(int argc, const char * argv[]) {
    LogicTests logicTests;
    logicTests.Run();
    
    PerformanceTests performanceTests;
    performanceTests.Run();
    
    return 0;
}
