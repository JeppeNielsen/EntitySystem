//
//  ScriptingTests.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 21/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "UnitTest.hpp"
#include "GameWorld.hpp"
#include "ScriptWorld.hpp"

using namespace Pocket;

class ScriptingTests : public UnitTest {
protected:
    virtual void RunTests();
private:
    bool CompileScriptingWorld(GameWorld& world, ScriptWorld& scriptWorld, const std::string& scriptCode);
    void WriteFile(const std::string& path, const std::string& content);
};