//
//  testEventCopy.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 29/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include "Property.hpp"
#include "DirtyProperty.hpp"
#include <string>
#include <deque>

using namespace Pocket;

struct Transform {

    Property<int> Position;
    DirtyProperty<int> World;
    std::string name;

    Transform() {
        World.Method = [this](int& value) {
            value = Position;
            std::cout << this->name<< "  "<< this << std::endl;
        };
        
        Position.Changed.Bind([this]() {
            World.MakeDirty();
            std::cout << this->name<< "  "<< this << " Position Changed " << std::endl;
        });
        
        std::cout<<"ctor" << this->name<< "  "<< this << std::endl;
    }

    
    
};

struct Velocity {
    Property<int> Vel;
    Velocity() {
        Vel.Changed.Bind([] () {
            std::cout << "Vel changed"<<std::endl;
        });
        Vel.Changed.MarkDefaults();
    }
    /*Velocity(const Velocity& other) {
        std::cout << "Copy"<<std::endl;
    }
    */
};



int main() {

    Velocity a;
    a.Vel = 232;
    
    std::deque<Velocity> bla;
    bla.resize(1);
    bla[0]=a;
    std::cout << bla[0].Vel << std::endl;

    return 0;
    std::deque<Transform> list;
    
    Transform original;
    original.name = "orig";
    
    list.emplace_back(original);
    
    //list[0].Position.Changed.Bind([]() {
    //    std::cout << "lambda"<<std::endl;
    //});
    
    //list[0] = original;
    list[0].Position = 455;
    
   // int oneWorld = list[0].World;
    

    return 0;
}