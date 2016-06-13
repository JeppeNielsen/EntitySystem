//
//  Object.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "Object.hpp"
#include <assert.h>
#include "World.hpp"

using namespace Pocket;

const ObjectCollection& Object::Children() const { return data->children; }
Property<Object*>& Object::Parent() { return data->Parent; }
Property<bool>& Object::Enabled() { return data->Enabled; }
DirtyProperty<bool>& Object::WorldEnabled() { return data->WorldEnabled; }

Object::Object()
    :
    world(0), index(-1)
{
    data = new Data();

    data->Enabled = true;
    data->Parent = 0;
    data->Parent.Changed.Bind([this]() {
        assert(data->Parent!=this);
        Object* prevParent = data->Parent.PreviousValue();
        Object* currentParent = data->Parent;
        
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
            prevParent->data->WorldEnabled.HasBecomeDirty.Unbind(this, &Object::SetWorldEnableDirty);
        }
        
        if (currentParent) {
            auto& children = currentParent->data->children;
            children.push_back(this);
            currentParent->data->WorldEnabled.HasBecomeDirty.Bind(this, &Object::SetWorldEnableDirty);
        }
    });
    
    data->WorldEnabled.Method = [this](bool& value) {
        value = (data->Parent) ? data->Parent()->data->WorldEnabled && data->Enabled : data->Enabled;
    };
    
    data->Enabled.Changed.Bind(this, &Object::SetWorldEnableDirty);
}

Object::~Object() { delete data; }

bool Object::HasComponent(ComponentID id) const {
    assert(id<MaxComponents);
    return data->activeComponents[id];
}

void* Object::GetComponent(ComponentID id) {
    //assert(id<MaxComponents);
    //IContainer* container = world->components[id];
    return world->objectComponents[id][index];
}

void Object::AddComponent(ComponentID id) {
    assert(id<MaxComponents);
    if (HasComponent(id)) {
        return;
    }
    IContainer* container = world->components[id];
    world->objectComponents[id][index] = container->Create();
    data->activeComponents[id] = true;
    
    world->createActions.emplace_back([this, id]() {
        TrySetComponentEnabled(id, true);
    });
}

void Object::AddComponent(ComponentID id, const Object* source) {
    assert(id<MaxComponents);
    assert(source->HasComponent(id));
    if (HasComponent(id)) {
        return;
    }
    IContainer* container = world->components[id];
    container->Reference((void*)world->objectComponents[id][index]);
    data->activeComponents[id] = true;
    
    world->createActions.emplace_back([this, id]() {
        TrySetComponentEnabled(id, true);
    });
}

void Object::CloneComponent(ComponentID id, const Object* source) {
    assert(id<MaxComponents);
    assert(source->HasComponent(id));
    if (HasComponent(id)) {
        return;
    }
    
    IContainer* container = world->components[id];
    world->objectComponents[id][index] = container->Clone((void*)source);
    data->activeComponents[id] = true;
    
    world->createActions.emplace_back([this, id]() {
        TrySetComponentEnabled(id, true);
    });
}

void Object::RemoveComponent(ComponentID id) {
    assert(id<MaxComponents);
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
        data->activeComponents[id] = false;
    });
}

void Object::Remove() {
    if (index==-1) return;
    int localIndex = index;
    world->removeActions.emplace_back([this, localIndex]() {
        SetEnabled(false);
        //world->objects.Delete(localIndex);
    });
    index = -1;
    data->Parent = 0;
    for(auto child : data->children) {
        child->Remove();
    }
}

void Object::TryAddComponentContainer(ComponentID id, std::function<IContainer *()> constructor) {
    if (!world->components[id]) {
        world->components[id] = constructor();
    }
}

void Object::SetWorldEnableDirty() {
    data->WorldEnabled.MakeDirty();
    world->createActions.emplace_back([this](){
        SetEnabled(data->WorldEnabled);
    });
}

void Object::SetEnabled(bool enabled) {
    for(int i=0; i<MaxComponents; ++i) {
        if (data->activeComponents[i]) {
            TrySetComponentEnabled(i, enabled);
        }
    }
}

void Object::TrySetComponentEnabled(ComponentID id, bool enable) {
    
    enable = enable && WorldEnabled();

    bool isEnabled = data->enabledComponents[id];
    if (isEnabled==enable) {
        return; //cannot double enable/disable components
    }
    
    if (enable) {
        data->enabledComponents[id] = enable;
        if (id>=world->systemsPerComponent.size()) return; // component id is beyond systems
        auto systemsUsingComponent = world->systemsPerComponent[id];
        for(auto s : systemsUsingComponent) {
            bool isInterest = (data->enabledComponents & s->componentMask) == s->componentMask;
            if (isInterest) {
                s->objects.push_back(this);
                s->ObjectAdded(this);
            }
        }
    } else {
        if (id>=world->systemsPerComponent.size()) return; // component id is beyond systems
        auto systemsUsingComponent = world->systemsPerComponent[id];
        for(auto s : systemsUsingComponent) {
            bool wasInterest = (data->enabledComponents & s->componentMask) == s->componentMask;
            if (wasInterest) {
                s->ObjectRemoved(this);
                auto& objects = s->objects;
                objects.erase(std::find(objects.begin(), objects.end(), this));
            }
        }
        data->enabledComponents[id] = enable;
    }
}
