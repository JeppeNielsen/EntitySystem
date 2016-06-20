//
//  GameObject.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "GameObject.hpp"
#include <assert.h>
#include "GameWorld.hpp"

using namespace Pocket;

const ObjectCollection& GameObject::Children() const { return data->children; }
Property<GameObject*>& GameObject::Parent() { return data->Parent; }
Property<bool>& GameObject::Enabled() { return data->Enabled; }
DirtyProperty<bool>& GameObject::WorldEnabled() { return data->WorldEnabled; }

GameObject::GameObject()
    :
    world(0), index(-1), data(0)
{
    data = new Data();

    data->Enabled = true;
    data->Parent = 0;
    
    data->Parent.Changed.Bind([this]() {
        assert(data->Parent!=this);
        GameObject* prevParent = data->Parent.PreviousValue();
        GameObject* currentParent = data->Parent;
        
        if (index>=0) {
            if (!prevParent) {
                prevParent = &world->root;
            }
            if (!currentParent) {
                currentParent = &world->root;
            }
        }
        
        if (prevParent) {
            auto& children = prevParent->data->children;
            children.erase(std::find(children.begin(), children.end(), this));
            prevParent->data->WorldEnabled.HasBecomeDirty.Unbind(this, &GameObject::SetWorldEnableDirty);
        }
        
        if (currentParent) {
            auto& children = currentParent->data->children;
            children.push_back(this);
            currentParent->data->WorldEnabled.HasBecomeDirty.Bind(this, &GameObject::SetWorldEnableDirty);
            
            bool prevWorldEnabled = data->WorldEnabled;
            data->WorldEnabled.MakeDirty();
            if (data->WorldEnabled()!=prevWorldEnabled) {
                SetWorldEnableDirty();
            }
        }
    });
    
    data->WorldEnabled.Method = [this](bool& value) {
        value = (data->Parent) ? data->Parent()->data->WorldEnabled && data->Enabled : data->Enabled;
    };
    
    data->Enabled.Changed.Bind(this, &GameObject::SetWorldEnableDirty);
}

GameObject::~GameObject() {
    delete data;
}

bool GameObject::HasComponent(ComponentID id) const {
    assert(id<world->numComponentTypes);
    return data->activeComponents[id];
}

void* GameObject::GetComponent(ComponentID id) {
    assert(id<world->numComponentTypes);
    if (!data->activeComponents[id]) return 0;
    IContainer* container = world->components[id];
    return container->Get(world->objectComponents[id][index]);
}

void GameObject::AddComponent(ComponentID id) {
    assert(id<world->numComponentTypes);
    if (HasComponent(id)) {
        return;
    }
    IContainer* container = world->components[id];
    world->objectComponents[id][index] = container->Create();
    data->activeComponents.Set(id, true);
    world->createActions.emplace_back([this, id]() {
        TrySetComponentEnabled(id, true);
    });
}

void GameObject::AddComponent(ComponentID id, GameObject* source) {
    assert(id<world->numComponentTypes);
    assert(source->HasComponent(id));
    if (HasComponent(id)) {
        return;
    }
    IContainer* container = world->components[id];
    int referenceIndex = world->objectComponents[id][source->index];
    world->objectComponents[id][index] = referenceIndex;
    container->Reference(referenceIndex);
    data->activeComponents.Set(id, true);
    
    world->createActions.emplace_back([this, id]() {
        TrySetComponentEnabled(id, true);
    });
}

void GameObject::CloneComponent(ComponentID id, GameObject* source) {
    assert(id<world->numComponentTypes);
    assert(source->HasComponent(id));
    if (HasComponent(id)) {
        return;
    }
    
    IContainer* container = world->components[id];
    world->objectComponents[id][index] = container->Clone(world->objectComponents[id][source->index]);
    data->activeComponents.Set(id, true);
    
    world->createActions.emplace_back([this, id]() {
        TrySetComponentEnabled(id, true);
    });
}

void GameObject::RemoveComponent(ComponentID id) {
    if (id>=world->numComponentTypes) return;
    if (!HasComponent(id)) {
        return;
    }
    world->removeActions.emplace_back([this, id]() {
        if (!data->activeComponents[id]) {
           return; // might have been removed by earlier remove action, eg if two consecutive RemoveComponent<> was called
        }
        TrySetComponentEnabled(id, false);
        IContainer* container = world->components[id];
        container->Delete(world->objectComponents[id][index]);
        world->objectComponents[id][index] = -1;
        data->activeComponents.Set(id, false);
    });
}

void GameObject::Remove() {
    if (index<0) return;
    int localIndex = index;
    world->removeActions.emplace_back([this, localIndex]() {
        SetEnabled(false);
        world->objectsFreeIndicies.push_back(localIndex);
        --world->objectCount;
        index = -2;
    });
    index = -1;
    data->Parent = 0;
    for(auto child : data->children) {
        child->Remove();
    }
}

void GameObject::TryAddComponentContainer(ComponentID id, std::function<IContainer *(std::string&)>&& constructor) {
    world->TryAddComponentContainer(id, std::move(constructor));
}

void GameObject::SetWorldEnableDirty() {
    data->WorldEnabled.MakeDirty();
    world->createActions.emplace_back([this](){
        SetEnabled(data->WorldEnabled);
    });
}

void GameObject::SetEnabled(bool enabled) {
    for(int i=0; i<world->numComponentTypes; ++i) {
        if (data->activeComponents[i]) {
            TrySetComponentEnabled(i, enabled);
        }
    }
}

void GameObject::TrySetComponentEnabled(ComponentID id, bool enable) {
    
    enable = enable && WorldEnabled();

    bool isEnabled = data->enabledComponents[id];
    if (isEnabled==enable) {
        return; //cannot double enable/disable components
    }
    
    if (enable) {
        data->enabledComponents.Set(id, true);
        if (id>=world->systemsPerComponent.size()) return; // component id is beyond systems
        auto& systemsUsingComponent = world->systemsPerComponent[id];
        for(auto systemIndex : systemsUsingComponent) {
            auto& bitset = world->systemBitsets[systemIndex];
            bool isInterest = bitset.Contains(data->enabledComponents);
            if (isInterest) {
                auto* system = world->systems[systemIndex];
                system->AddObject(this);
                system->ObjectAdded(this);
            }
        }
    } else {
        if (id>=world->systemsPerComponent.size()) return; // component id is beyond systems
        auto& systemsUsingComponent = world->systemsPerComponent[id];
        for(auto systemIndex : systemsUsingComponent) {
            auto& bitset = world->systemBitsets[systemIndex];
            bool wasInterest = bitset.Contains(data->enabledComponents);
            if (wasInterest) {
                auto* system = world->systems[systemIndex];
                system->ObjectRemoved(this);
                system->RemoveObject(this);
            }
        }
        data->enabledComponents.Set(id, false);
    }
}