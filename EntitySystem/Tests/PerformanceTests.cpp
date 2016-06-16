//
//  PerformanceTests.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 12/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "PerformanceTests.hpp"
#include "World.hpp"
using namespace Pocket;

void PerformanceTests::RunTests() {
    
    AddTest("World ctor/dtor x 100000", [this]() {
        Begin();
        for(int i = 0; i<100000; ++i) {
            World world;
        }
        End();
    });
    
    AddTest("World ctor/dtor x 100000 x 10 objects", [this]() {
        Begin();
        for(int i = 0; i<100000; ++i) {
            World world;
            for(int j = 0; j<10; ++j) {
                world.CreateObject();
            }
        }
        End();
    });
    
    AddTest("CreateObject x 1000000", [this]() {
        Begin();
        World world;
        for(int i = 0; i<1000000; ++i) {
            world.CreateObject();
        }
        End();
    });
    
    
    AddTest("AddComponent x 100000", [this]() {
        struct Component { };
        World world;
        ObjectCollection objects;
        for(int i=0; i<100000; ++i) {
            objects.push_back(world.CreateObject());
        }
        Begin();
        for(int i = 0; i<objects.size(); ++i) {
            objects[i]->AddComponent<Component>();
        }
        End();
    });
    
    AddTest("GetComponent x 1000000", [this]() {
        struct Component { int x; };
        
        World world;
        ObjectCollection objects;
        for(int i=0; i<1000000; ++i) {
            objects.push_back(world.CreateObject());
        }
        
        for(int i = 0; i<objects.size(); ++i) {
            objects[i]->AddComponent<Component>();
        }
        
        Begin();
        for(int i = 0; i<objects.size(); ++i) {
            objects[i]->GetComponent<Component>()->x++;
        }
        End();
    });
    
    AddTest("GetComponent x 10000 x 10000", [this]() {
        struct Component { int x; };
        
        World world;
        ObjectCollection objects;
        for(int i=0; i<10000; ++i) {
            objects.push_back(world.CreateObject());
        }
        
        
        for(int i = 0; i<objects.size(); ++i) {
            objects[i]->AddComponent<Component>();
        }
        
        Begin();
        for(int j=0; j<10000; ++j) {
        for(int i = 0; i<objects.size(); ++i) {
            objects[i]->GetComponent<Component>()->x++;
        }
        }
        End();
    });

    

}