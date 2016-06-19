//
//  GameObject.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameIDHelper.hpp"
#include "Property.hpp"
#include "DirtyProperty.hpp"

namespace Pocket {
    
    template<typename T>
    class Container;
    
    class GameObject;
    
    using ObjectCollection = std::vector<GameObject*>;
    
    class GameWorld;
    
    struct IGameObject {
        virtual void* GetComponent(ComponentID id) = 0;
        virtual void AddComponent(ComponentID id) = 0;
        virtual void AddComponent(ComponentID id, GameObject* referenceObject) = 0;
        virtual void RemoveComponent(ComponentID id) = 0;
        virtual void CloneComponent(ComponentID id, GameObject* object) = 0;
        
        virtual void* GetScriptComponent(ComponentID id) = 0;
        virtual void AddScriptComponent(ComponentID id) = 0;
        virtual void RemoveScriptComponent(ComponentID id) = 0;
    };
    
    class GameObject : public IGameObject {
    public:
    
        template<typename T>
        bool HasComponent() const {
            return HasComponent(GameIDHelper::GetComponentID<T>());
        }
    
        template<typename T>
        T* GetComponent();
        
        template<typename T>
        T* AddComponent() {
            ComponentID id = GameIDHelper::GetComponentID<T>();
            TryAddComponentContainer(id, [](){ return new Container<T>(); });
            AddComponent(id);
            return GetComponent<T>();
        }
        
        template<typename T>
        T* AddComponent(GameObject* source) {
            AddComponent(GameIDHelper::GetComponentID<T>(), source);
            return GetComponent<T>();
        }
        
        template<typename T>
        T* CloneComponent(GameObject* source) {
            CloneComponent(GameIDHelper::GetComponentID<T>(), source);
            return GetComponent<T>();
        }
        
        template<typename T>
        void RemoveComponent() {
            RemoveComponent(GameIDHelper::GetComponentID<T>());
        };
        
        void Remove();
        
        const ObjectCollection& Children() const;
        Property<GameObject*>& Parent();
        Property<bool>& Enabled();
        DirtyProperty<bool>& WorldEnabled();
        
        GameObject();
        ~GameObject();
        
    private:
        
        GameObject(GameObject&& o) = delete;
        GameObject(const GameObject& o) = delete;
        GameObject& operator=(const GameObject& o) = delete;
        
        bool HasComponent(ComponentID id) const;
        void* GetComponent(ComponentID id) override;
        void AddComponent(ComponentID id) override;
        void AddComponent(ComponentID id, GameObject* source) override;
        void CloneComponent(ComponentID id, GameObject* source)  override;
        void RemoveComponent(ComponentID id) override;
        void TryAddComponentContainer(ComponentID id, std::function<IContainer*()>&& constructor);
        void SetWorldEnableDirty();
        void SetEnabled(bool enabled);
        void TrySetComponentEnabled(ComponentID id, bool enable);
        
        void* GetScriptComponent(ComponentID id) override;
        void AddScriptComponent(ComponentID id) override;
        void RemoveScriptComponent(ComponentID id) override;
        
        struct Data {
            Data() {}
            Bitset activeComponents;
            Bitset enabledComponents;
            Property<GameObject*> Parent;
            Property<bool> Enabled;
            DirtyProperty<bool> WorldEnabled;
            ObjectCollection children;
        };
        
        int index;
        GameWorld* world;
        Data* data;
        
        friend class Container<GameObject>;
        friend class GameWorld;
    };
}