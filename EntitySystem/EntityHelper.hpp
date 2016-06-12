//
//  EntityHelper.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//
#pragma once
#include <bitset>
#include <array>
#include <functional>
#include "Container.hpp"

namespace Pocket {
    
    using ComponentID = int;
    const ComponentID MaxComponents = 64;
    using ComponentMask = std::bitset<MaxComponents>;

    using SystemID = int;
    
    class EntityHelper {
    private:
        static ComponentID componentIDCounter;
        static SystemID systemIDCounter;
        
    public:
    
        template<typename T>
        static ComponentID GetComponentID() {
            static ComponentID id = componentIDCounter++;
            return id;
        }
        
        template<typename T>
        static SystemID GetSystemID() {
            static SystemID id = systemIDCounter++;
            return id;
        }
    };
}