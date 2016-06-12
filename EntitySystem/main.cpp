//
//  main.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include "World.hpp"
#include "EntitySystemTests.hpp"

using namespace Pocket;


struct Transform {
    int x;
    int y;
};

struct Renderable {
    int imageNo;
};

struct Touchable {
    bool clickable;
};


struct RenderSystem : public System <Transform, Renderable> {
    
    void ObjectAdded(Object* object) {
        std::cout << "RenderSystem::ObjectAdded ImageNo"<< object->GetComponent<Renderable>()->imageNo <<std::endl;
    }

    void ObjectRemoved(Object* object) {
        std::cout << "RenderSystem::ObjectRemoved ImageNo"<< object->GetComponent<Renderable>()->imageNo<<std::endl;
    }
};

struct TouchSystem : public System <Transform, Touchable> {
    void ObjectAdded(Object* object) {
        std::cout << "TouchSystem::ObjectAdded "<<std::endl;
    }

    void ObjectRemoved(Object* object) {
        std::cout << "TouchSystem::ObjectRemoved "<<std::endl;
    }
};

int main(int argc, const char * argv[]) {
    EntitySystemTests tests;
    tests.Run();

    World world;
    
    world.CreateSystem<RenderSystem>();
    //world.CreateSystem<TouchSystem>();

    auto root = world.CreateObject();
    root->Enabled = true;
    

    auto child = world.CreateObject();
    child->Parent = root;
    
    child->AddComponent<Renderable>()->imageNo = 1;
    child->AddComponent<Transform>();
    child->AddComponent<Touchable>();
    
    auto other = world.CreateObject();
    other->Parent = root;
    other->AddComponent<Transform>();
    other->AddComponent<Renderable>()->imageNo = 2;
    
    world.Update(0);
    
    other->Remove();
    
    root->Remove();
    
    
    world.Update(0);
    
    return 0;
}
