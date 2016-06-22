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
    
    AddTest("Create Object", [this]() {
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
}