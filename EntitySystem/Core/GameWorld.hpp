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
        
        struct GameObjectEntry {
            GameObject object;
        };
    
        using Objects = std::deque<GameObjectEntry>;
        Objects objects;
        using ObjectsFreeIndicies = std::vector<int>;
        ObjectsFreeIndicies objectsFreeIndicies;
        
        using Components = std::vector<IContainer*>;
        Components components;
        
        struct ObjectComponent {
            int index;
            IContainer* container;
        };
        
        using ObjectComponents = std::vector<std::vector<ObjectComponent>>;
        ObjectComponents objectComponents;
        
        using Systems = std::vector<IGameSystem*>;
        Systems systemsIndexed;
        Systems systems;
        using SystemsPerComponent = std::vector<std::vector<int>>;
        SystemsPerComponent systemsPerComponent;
        using DeleteSystems = std::vector<std::function<void()>>;
        DeleteSystems deleteSystems;
        
        using SystemBitsets = std::vector<Bitset>;
        SystemBitsets systemBitsets;
        
        using Action = std::function<void()>;
        using Actions = std::deque<Action>;
        Actions delayedActions;
        
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
        if (id>=world->numComponentTypes) return 0;
        auto& objectComponent = world->objectComponents[id][index];
        if (objectComponent.index == -1) return 0;
        Container<T>* container = static_cast<Container<T>*>(objectComponent.container);
        return &container->entries[objectComponent.index];
    }

    
    
}