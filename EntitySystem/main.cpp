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

using namespace Pocket;

int main(int argc, const char * argv[]) {
    
    GameWorld world;
    
    ScriptWorld scriptWorld;
    
    scriptWorld.SetFiles(
        "EntitySystem.so",
        "/Projects/EntitySystem/EntitySystem/Scripting/ScriptInclude",
        {
            "/Projects/EntitySystem/EntitySystem/ScriptExample.cpp"
        },{
            "/Projects/EntitySystem/EntitySystem/Data/Property.hpp"
        }
        /*{
            "/Projects/PocketEngine/Pocket/Logic/Spatial/Transform.hpp",
            "/Projects/PocketEngine/Pocket/Logic/Rendering/Mesh.hpp",
            "/Projects/PocketEngine/Pocket/Data/Property.hpp",
            "/Projects/PocketEngine/Pocket/Math/Vector2.hpp",
            "/Projects/PocketEngine/Pocket/Math/Vector3.hpp",
            "/Projects/PocketEngine/Pocket/Logic/Gui/Sizeable.hpp",
            "/Projects/PocketEngine/Pocket/Rendering/VertexMesh.hpp",
            "/Projects/PocketEngine/Pocket/Rendering/TextureAtlas.hpp",
            "/Projects/PocketEngine/Pocket/Rendering/Colour.hpp",
            "/Projects/PocketEngine/Pocket/Logic/Interaction/Touchable.hpp",
        }
        */
        );
    scriptWorld.Build();
    scriptWorld.AddGameWorld(world);
    
    auto object = world.CreateObject();
    object->AddComponent(0);
    object->AddComponent(1);
    /*
    auto type = scriptWorld.GetTypeInfo(*object, 1);
    
    FieldInfo<int>* info =  ((FieldInfo<int>*) type.GetField("vx"));
    *info->field = 2;
    
    
    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(std::cout, config);
    
    type.Serialize(writer);
    writer.close();
    */
    
    
    world.Update(0);
    world.Update(0);
    world.Update(0);
    
    return 0;
}
