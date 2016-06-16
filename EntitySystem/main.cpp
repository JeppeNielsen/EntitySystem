//
//  main.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//
#include <iostream>
#include "Timer.hpp"
#include "World.hpp"
#include "PerformanceTests.hpp"
#include "LogicTests.hpp"

using namespace Pocket;

struct Component { int x; };

int main(int argc, const char * argv[]) {

    LogicTests logicTests;
    logicTests.Run();
    
    PerformanceTests performance;
    performance.Run();
    
    return 0;

    Timer timer;
    
    timer.Begin();
    World world;
    ObjectCollection objects;
    for(int i=0; i<1000000; ++i) {
        objects.push_back(world.CreateObject());
    }
    double createObjectTime = timer.End();
    std::cout << "CreateObject Time = " << createObjectTime <<std::endl;
    
    timer.Begin();
    for(int i = 0; i<objects.size(); ++i) {
        objects[i]->AddComponent<Component>();
    }
    double addComponentTime = timer.End();
    std::cout << "AddComponent Time = " << addComponentTime <<std::endl;
    
    timer.Begin();
    //for (int j = 0; j<1000000; ++j) {
    for(int i = 0; i<objects.size(); ++i) {
        objects[i]->GetComponent<Component>()->x++;
    }
    //}
    double time = timer.End();
    std::cout << "GetComponent Time = " << time<<std::endl;
//Time = 3.77209
//       2.84425

//0.0272441
//0.0439391

    std::cout << "sizeof(Object) = "<< sizeof(Object) << std::endl;
    std::cout << "alignof(Object) = "<< alignof(Object) << std::endl;
    return 0;
}
