//
//  Object.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "EntityHelper.hpp"
#include "Property.hpp"
#include "DirtyProperty.hpp"

namespace Pocket {
    class World;
    
    template<typename T>
    class Container;
    
    class Object;
    
    using ObjectCollection = std::vector<Object*>;
    
    class Object {
    public:
    
        template<typename T>
        bool HasComponent() const {
            return HasComponent(EntityHelper::GetComponentID<T>());
        }
    
        template<typename T>
        T* GetComponent() {
            return static_cast<T*>(GetComponent(EntityHelper::GetComponentID<T>()));
        }
        
        template<typename T>
        T* AddComponent() {
            ComponentID id = EntityHelper::GetComponentID<T>();
            TryAddComponentContainer(id, [](){ return new Container<T>(); });
            AddComponent(id);
            return GetComponent<T>();
        }
        
        template<typename T>
        T* AddComponent(Object* source) {
            AddComponent(EntityHelper::GetComponentID<T>(), source);
            return GetComponent<T>();
        }
        
        template<typename T>
        T* CloneComponent(Object* source) {
            CloneComponent(EntityHelper::GetComponentID<T>(), source);
            return GetComponent<T>();
        }
        
        template<typename T>
        void RemoveComponent() {
            RemoveComponent(EntityHelper::GetComponentID<T>());
        };
        
        void Remove();
        
        Property<Object*> Parent;
        const ObjectCollection& Children() const;
        
        Property<bool> Enabled;
        DirtyProperty<bool> WorldEnabled;
    private:
        Object();
        ~Object();
        Object(const Object& o) = delete;
        Object(Object&& o) = delete;
        Object& operator=(const Object& o) = default;
        
        bool HasComponent(ComponentID id) const;
        void* GetComponent(ComponentID id);
        void AddComponent(ComponentID id);
        void AddComponent(ComponentID id, const Object* source);
        void CloneComponent(ComponentID id, const Object* source);
        void RemoveComponent(ComponentID id);
        void TryAddComponentContainer(ComponentID id, std::function<IContainer*()> constructor);
        void SetWorldEnableDirty();
        void SetEnabled(bool enabled);
        void TrySetComponentEnabled(ComponentID id, bool enable);
        
        World* world;
        int index;
        ComponentMask activeComponents;
        ComponentMask enabledComponents;
        std::array<int, MaxComponents> components;
        ObjectCollection children;
        int childIndex;

        friend class Container<Object>;
        friend class World;
    };
}