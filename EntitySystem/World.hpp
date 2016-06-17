//
//  World.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "Object.hpp"
#include "Container.hpp"
#include "GameSystem.hpp"

namespace Pocket {
    class World {
    public:
        World();
        ~World();
        
        const Object* Root();
        
        Object* CreateObject();
        
        template<typename T>
        T* CreateSystem() {
            return static_cast<T*>(
            TryAddSystem(
            EntityHelper::GetSystemID<T>(),
            [this](std::vector<int>& components)
                {
                    T* system = new T;
                    World** worldC = ((World**)&system->world);
                    *(worldC) = this;
                    system->ExtractAllComponents(components);
                    return system;
                }
            ));
        }
        
        template<typename T>
        void RemoveSystem() {
            TryRemoveSystem(EntityHelper::GetSystemID<T>());
        }
        
        void Update(float dt);
        void Render();
        
        int ObjectCount() const;
        int CapacityCount() const;
        
        void Clear();
        void Trim();
        
    private:
    
        Object root;
    
        using Objects = std::deque<Object>;
        Objects objects;
        using ObjectsFreeIndicies = std::vector<int>;
        ObjectsFreeIndicies objectsFreeIndicies;
        
        using Components = std::array<IContainer*, MaxComponents>;
        Components components;
        
        using ObjectComponents = std::array<std::vector<int>, MaxComponents>;
        ObjectComponents objectComponents;
        
        using Systems = std::vector<IGameSystem*>;
        Systems systemsIndexed;
        Systems systems;
        using SystemsPerComponent = std::vector<Systems>;
        SystemsPerComponent systemsPerComponent;
        
        using Action = std::function<void()>;
        using Actions = std::vector<Action>;
        Actions createActions;
        Actions removeActions;
        
        int objectCount;
        
        IGameSystem* TryAddSystem(SystemID id, std::function<IGameSystem*(std::vector<int>& components)> constructor);
        void TryRemoveSystem(SystemID id);
        void DoActions(Actions& actions);
        void IterateObjects(std::function<void(Object*)> callback);
        
        friend class Object;
        friend class IGameSystem;
    };
    
    
    template<typename T>
    T* Object::GetComponent() {
        ComponentID id = EntityHelper::GetComponentID<T>();
        int componentIndex = world->objectComponents[id][index];
        if (componentIndex == -1) return 0;
        Container<T>* container = static_cast<Container<T>*>(world->components[id]);
        return &container->entries[componentIndex];
    }

    
    
}