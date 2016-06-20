//
//  GameWorld.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameObject.hpp"
#include "Container.hpp"
#include "GameSystem.hpp"

namespace Pocket {
    class ScriptWorld;
    class GameWorld {
    public:
        GameWorld();
        ~GameWorld();
        
        const GameObject* Root();
        
        GameObject* CreateObject();
        
        template<typename T>
        T* CreateSystem() {
            return static_cast<T*>(
            TryAddSystem(
            GameIDHelper::GetSystemID<T>(),
            [this](std::vector<int>& components)
                {
                    T* system = new T;
                    GameWorld** worldC = ((GameWorld**)&system->world);
                    *(worldC) = this;
                    system->ExtractAllComponents(components);
                    return system;
                }
            ));
        }
        
        template<typename T>
        void RemoveSystem() {
            TryRemoveSystem(GameIDHelper::GetSystemID<T>());
        }
        
        void Update(float dt);
        void Render();
        
        int ObjectCount() const;
        int CapacityCount() const;
        
        void Clear();
        void Trim();
        
    private:
    
        GameObject root;
    
        using Objects = std::deque<GameObject>;
        Objects objects;
        using ObjectsFreeIndicies = std::vector<int>;
        ObjectsFreeIndicies objectsFreeIndicies;
        
        using Components = std::vector<IContainer*>;
        Components components;
        
        using ObjectComponents = std::vector<std::vector<int>>;
        ObjectComponents objectComponents;
        
        using Systems = std::vector<IGameSystem*>;
        Systems systemsIndexed;
        Systems systems;
        using SystemsPerComponent = std::vector<std::vector<int>>;
        SystemsPerComponent systemsPerComponent;
        
        using SystemBitsets = std::vector<Bitset>;
        SystemBitsets systemBitsets;
        
        using Action = std::function<void()>;
        using Actions = std::vector<Action>;
        Actions createActions;
        Actions removeActions;
        
        using ComponentNames = std::vector<std::string>;
        ComponentNames componentNames;
        
        int objectCount;
        int numComponentTypes;
        
        IGameSystem* TryAddSystem(SystemID id, std::function<IGameSystem*(std::vector<int>& components)> constructor);
        void TryRemoveSystem(SystemID id);
        void DoActions(Actions& actions);
        void IterateObjects(std::function<void(GameObject*)> callback);
        void TryAddComponentContainer(ComponentID id, std::function<IContainer *(std::string&)>&& constructor);
        
        friend class GameObject;
        friend class GameSystemBase;
        friend class ScriptWorld;
    };
    
    
    template<typename T>
    T* GameObject::GetComponent() {
        ComponentID id = GameIDHelper::GetComponentID<T>();
        int componentIndex = world->objectComponents[id][index];
        if (componentIndex == -1) return 0;
        Container<T>* container = static_cast<Container<T>*>(world->components[id]);
        return &container->entries[componentIndex];
    }

    
    
}