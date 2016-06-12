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

const ObjectCollection& Object::Children() const { return children; }

Object::Object()
    :
    activeComponents(0), enabledComponents(0), world(0), index(-1)
{
    Enabled = true;
    Parent = 0;
    Parent.Changed.Bind([this]() {
        assert(Parent!=this);
        Object* prevParent = Parent.PreviousValue();
        Object* currentParent = Parent;
        
        if (index>=0) {
            if (!prevParent) {
                prevParent = &world->root;
            }
            if (!currentParent) {
                currentParent = &world->root;
            }
        }
        
        if (prevParent) {
            auto& children = prevParent->children;
            Object* lastChild = children.back();
            lastChild->childIndex = childIndex;
            children[childIndex] = lastChild;
            children.pop_back();
            prevParent->WorldEnabled.HasBecomeDirty.Unbind(this, &Object::SetWorldEnableDirty);
        }
        
        if (currentParent) {
            childIndex = (int)currentParent->children.size();
            currentParent->children.push_back(this);
            currentParent->WorldEnabled.HasBecomeDirty.Bind(this, &Object::SetWorldEnableDirty);
        }
    });
    
    WorldEnabled.Method = [this](bool& value) {
        value = (Parent) ? Parent()->WorldEnabled && Enabled : Enabled;
    };
    
    Enabled.Changed.Bind(this, &Object::SetWorldEnableDirty);
}

Object::~Object() {}

bool Object::HasComponent(ComponentID id) const {
    assert(id<MaxComponents);
    return activeComponents[id];
}

void* Object::GetComponent(ComponentID id) {
    assert(id<MaxComponents);
    if (!HasComponent(id)) {
        return 0;
    }
    IContainer* container = world->components[id];
    return container->Get(components[id]);
}

void Object::AddComponent(ComponentID id) {
    assert(id<MaxComponents);
    if (HasComponent(id)) {
        return;
    }
    IContainer* container = world->components[id];
    container->Create(components[id]);
    activeComponents[id] = true;
    
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
    components[id] = source->components[id];
    container->Reference(components[id]);
    activeComponents[id] = true;
    
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
    container->Clone(source->components[id], components[id]);
    activeComponents[id] = true;
    
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
        if (!activeComponents[id]) {
           return; // might have been removed by earlier remove action, eg if two consecutive RemoveComponent<> was called
        }
        TrySetComponentEnabled(id, false);
        IContainer* container = world->components[id];
        container->Delete(components[id]);
        activeComponents[id] = false;
    });
}

void Object::Remove() {
    if (index==-1) return;
    int localIndex = index;
    world->removeActions.emplace_back([this, localIndex]() {
        SetEnabled(false);
        world->objects.Delete(localIndex);
    });
    index = -1;
    Parent = 0;
    for(auto child : children) {
        child->Remove();
    }
}

void Object::TryAddComponentContainer(ComponentID id, std::function<IContainer *()> constructor) {
    if (!world->components[id]) {
        world->components[id] = constructor();
    }
}

void Object::SetWorldEnableDirty() {
    WorldEnabled.MakeDirty();
    world->createActions.emplace_back([this](){
        SetEnabled(WorldEnabled);
    });
}

void Object::SetEnabled(bool enabled) {
    for(int i=0; i<MaxComponents; ++i) {
        if (activeComponents[i]) {
            TrySetComponentEnabled(i, enabled);
        }
    }
}

void Object::TrySetComponentEnabled(ComponentID id, bool enable) {
    
    enable = enable && WorldEnabled();

    bool isEnabled = enabledComponents[id];
    if (isEnabled==enable) {
        return; //cannot double enable/disable components
    }
    if (id>=world->systemsPerComponent.size()) return; // component id is beyond systems
    auto systemsUsingComponent = world->systemsPerComponent[id];
    
    if (enable) {
        enabledComponents[id] = enable;
        for(auto s : systemsUsingComponent) {
            bool isInterest = (enabledComponents & s->componentMask) == s->componentMask;
            if (isInterest) {
                s->objects.push_back(this);
                s->ObjectAdded(this);
            }
        }
    } else {
        for(auto s : systemsUsingComponent) {
            bool wasInterest = (enabledComponents & s->componentMask) == s->componentMask;
            if (wasInterest) {
                s->ObjectRemoved(this);
                auto& objects = s->objects;
                objects.erase(std::find(objects.begin(), objects.end(), this));
            }
        }
        enabledComponents[id] = enable;
    }
}
