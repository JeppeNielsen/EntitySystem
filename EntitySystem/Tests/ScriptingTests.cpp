//
//  ScriptingTests.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 21/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "ScriptingTests.hpp"
#include <fstream>
#include <sstream>

void ScriptingTests::WriteFile(const std::string &path, const std::string &content) {
    std::ofstream file;
    file.open (path);
    file<<content;
    file.close();
}

bool ScriptingTests::CompileScriptingWorld(Pocket::GameWorld &world, Pocket::ScriptWorld &scriptWorld, const std::string &scriptCode) {
    std::string pathToTempFile = "/Projects/EntitySystem/Build/Build/Products/Debug/example_temp.cpp";
    
    WriteFile(pathToTempFile, scriptCode);
    
    scriptWorld.SetClangSdkPath("/Users/Jeppe/Downloads/clang+llvm-3.7.0-x86_64-apple-darwin/");
    
    static int counter = 0;
    counter++;
    
    std::stringstream s;
    s<<"Example"<<counter<<".so";
    
    scriptWorld.SetFiles(
        s.str(),
        "/Projects/EntitySystem/EntitySystem/Scripting/ScriptInclude",
        {
            pathToTempFile
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
    
    scriptWorld.SetWorldType(world);
    if (!scriptWorld.Build(false)) {
        return false;
    }
    scriptWorld.AddGameWorld(world);
    return true;
}

void ScriptingTests::RunTests() {
    
    AddTest("Compile Script", [this]() {
        GameWorld world;
        ScriptWorld scriptWorld;
        bool succes = this->CompileScriptingWorld(world, scriptWorld,
            " struct Position { int x; }; "
        );
        return succes;
    });
    
    
    AddTest("Compiler Error", [this]() {
        GameWorld world;
        ScriptWorld scriptWorld;
        bool succes = this->CompileScriptingWorld(world, scriptWorld,
            " struct Position { int x; } "
        );
        return !succes;
    });
    
    AddTest("AddComponent", [this]() {
        GameWorld world;
        ScriptWorld scriptWorld;
        if (!this->CompileScriptingWorld(world, scriptWorld,
            "#include \"GameSystem.hpp\"\n"
            " struct Position { int x; }; \n"
            " struct System : public GameSystem<Position> {};\n"
        )) {
            return false;
        }
        GameObject* object = world.CreateObject();
        object->AddComponent(0);
        void* ptr = object->GetComponent(0);
        return object && ptr;
    });
    
    
    AddTest("Recompile", [this]() {
        
        struct EngineComponent { int data; };
        struct EngineSystem : public GameSystem<EngineComponent> {
            void ObjectAdded(GameObject* object) {}
            void ObjectRemoved(GameObject* object) {}
        };
        
        GameWorld world;
        world.CreateSystem<EngineSystem>();
        GameObject* object = world.CreateObject();
        object->AddComponent<EngineComponent>();
        
        ScriptWorld scriptWorld;
        if (!this->CompileScriptingWorld(world, scriptWorld,
            "#include \"GameSystem.hpp\"\n"
            " struct Position { int x; }; \n"
            " struct System : public GameSystem<Position> {};\n"
        )) {
            return false;
        }
        int numberOfComponentsPreviously = (int)scriptWorld.Components().size();
        scriptWorld.RemoveGameWorld(world);
        if (!this->CompileScriptingWorld(world, scriptWorld,
            "#include \"GameSystem.hpp\"\n"
            " struct Position { int x; }; \n"
            " struct Velocity { int x; }; \n"
            " struct MovementSystem : public GameSystem<Position, Velocity> {};\n"
        )) {
            return false;
        }
        return scriptWorld.Components().size() == 2 && numberOfComponentsPreviously == 1;
    });

    AddTest("Added To System", [this]() {
        
        GameWorld world;
        GameObject* object = world.CreateObject();
        ScriptWorld scriptWorld;
               if (!this->CompileScriptingWorld(world, scriptWorld,
            "#include \"GameSystem.hpp\"\n"
            "#include <iostream>\n"
            " struct Position { int x; }; \n"
            " struct Velocity { int x; }; \n"
            " struct MovementSystem : public GameSystem<Position, Velocity> {"
            "     void ObjectAdded(GameObject* object) { \n"
            "          std::cout<< \"ObjectAdded\"<<std::endl;     \n"
            " } \n"
            "};\n"
        )) {
            return false;
        }
        object->AddComponent(scriptWorld.Components()["Position"]);
        object->AddComponent(scriptWorld.Components()["Velocity"]);
        world.Update(0);
        return object->GetComponent(scriptWorld.Components()["Position"])!=0 ||
            object->GetComponent(scriptWorld.Components()["Velocity"])!=0;
    });

    AddTest("AddComponent", [this]() {
        
        GameWorld world;
        GameObject* object = world.CreateObject();
        ScriptWorld scriptWorld;
               if (!this->CompileScriptingWorld(world, scriptWorld,
            "#include \"GameSystem.hpp\"\n"
            "#include <iostream>\n"
            " struct Position { int x; Position() : x(123) {} }; \n"
            " struct PositionSystem : public GameSystem<Position> { }; \n"
        )) {
            return false;
        }
        object->AddComponent(scriptWorld.Components()["Position"]);
        world.Update(0);
        auto typeInfo = scriptWorld.GetTypeInfo(*object, scriptWorld.Components()["Position"]);
        IFieldInfo* fieldInfo = typeInfo.GetField("x");
        FieldInfo<int>* intFieldInfo = (FieldInfo<int>*)fieldInfo;
        return *intFieldInfo->field == 123;
    });

    
}