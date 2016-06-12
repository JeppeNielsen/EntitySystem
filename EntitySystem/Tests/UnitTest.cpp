//
//  UnitTest.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 11/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "UnitTest.hpp"
#include <iostream>

UnitTest::~UnitTest() {}

void UnitTest::Run() {
    tests.clear();
    RunTests();
    
    for(auto& test : tests) {
        bool succes = test.method();
        std::cout<<test.name << " -> "<<((succes) ? "SUCCES!" : "FAILED!")<<std::endl;
    }
}

void UnitTest::AddTest(const std::string &name, TestMethod test) {
    tests.push_back({name, test });
}