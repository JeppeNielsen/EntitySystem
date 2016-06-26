//
//  main.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//
#include "GameWorld.hpp"
#include "ScriptWorld.hpp"
#include <iostream>
#include "ScriptingTests.hpp"
#include "LogicTests.hpp"

using namespace Pocket;

struct Transform {
    int position;
};

struct Renderable {
    int imageNo;
};

struct RenderSystem : public GameSystem<Transform, Renderable> {

    void ObjectAdded(GameObject* object) {
        std::cout << "RenderSystem::ObjectAdded" << std::endl;
    }
    
    void ObjectRemoved(GameObject* object) {
        std::cout << "RenderSystem::ObjectRemoved" << std::endl;
    }

};

int main(int argc, const char * argv[]) {
    /*
    LogicTests logicTests;
    logicTests.Run();
    
    ScriptingTests tests;
    tests.Run();

    return 0;
    */
    
    GameWorld world;
    world.CreateSystem<RenderSystem>();
    
    ScriptWorld scriptWorld;
    scriptWorld.SetClangSdkPath("/Users/Jeppe/Downloads/clang+llvm-3.7.0-x86_64-apple-darwin/");
    
    scriptWorld.SetFiles(
        "EntitySystem.so",
        "/Projects/EntitySystem/EntitySystem/Scripting/ScriptInclude",
        {
            "/Projects/EntitySystem/EntitySystem/ScriptExample.cpp"
        },{
            "/Projects/EntitySystem/EntitySystem/Data/Property.hpp"
        }
        );
    
    scriptWorld.SetWorldType(world);
    scriptWorld.Build(true);
    scriptWorld.AddGameWorld(world);
    
    for(auto c : scriptWorld.Components()) {
        std::cout << " " << c.first << ": " << c.second<<std::endl;
    }

    auto object = world.CreateObject();
    object->AddComponent(scriptWorld.Components()["Position"]);
    object->AddComponent(scriptWorld.Components()["Velocity"]);
    
    world.Update(0);
    world.Update(0);
    world.Update(0);
    
    
    auto type = scriptWorld.GetTypeInfo(*object, scriptWorld.Components()["Velocity"]);
    
    FieldInfo<int>* info =  ((FieldInfo<int>*) type.GetField("vx"));
    *info->field = 2;
    
    
    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(std::cout, config);
    
    type.Serialize(writer);
    writer.close();
    
    
    return 0;
}
