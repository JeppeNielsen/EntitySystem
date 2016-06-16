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
    
    template<typename T>
    class Container;
    
    class Object;
    
    using ObjectCollection = std::vector<Object*>;
    
    class World;
    
    class Object {
    public:
    
        template<typename T>
        bool HasComponent() const {
            return HasComponent(EntityHelper::GetComponentID<T>());
        }
    
        template<typename T>
        T* GetComponent();
        
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
        
        const ObjectCollection& Children() const;
        Property<Object*>& Parent();
        Property<bool>& Enabled();
        DirtyProperty<bool>& WorldEnabled();
        
        Object();
        ~Object();
        
    private:
        
        Object(Object&& o) = delete;
        Object(const Object& o) = delete;
        Object& operator=(const Object& o) = delete;
        
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
        
        struct Data {
            Data() : activeComponents(0), enabledComponents(0) {}
            ComponentMask activeComponents;
            ComponentMask enabledComponents;
            Property<Object*> Parent;
            Property<bool> Enabled;
            DirtyProperty<bool> WorldEnabled;
            ObjectCollection children;
        };
        
        int index;
        World* world;
        Data* data;
        
        friend class Container<Object>;
        friend class World;
    };
}