//
//  World.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "World.hpp"
#include <iostream>

using namespace Pocket;

World::World() {
    for(int i=0; i<MaxComponents; ++i) {
        components[i] = 0;
    }
    root.world = this;
}

World::~World() {
    Clear();
    for(auto s : systems) {
        delete s;
    }
    for(int i=0; i<MaxComponents; ++i) {
        delete components[i];
    }
}

const Object* World::Root() { return &root; }

Object* World::CreateObject() {
    int index = (int)objects.size();
    objects.resize(index + 1);
    //objectComponents.resize(index + 1);
    if (index>=objectComponents[0].size()) {
        for(int i=0; i<MaxComponents; i++) {
            objectComponents[i].resize(index + 16);
        }
    }
    Object* object = &objects.back();
    object->Parent() = &root;
    object->index = index;
    object->world = this;
    return object;
}

void World::Update(float dt) {
    for(auto system : systems) {
        system->Update(dt);
    }
    DoActions(createActions);
    DoActions(removeActions);
}

void World::Render() {
    for(auto system : systems) {
        system->Render();
    }
}

int World::ObjectCount() const {
    return (int)objects.size();
}

void World::Clear() {
    for(auto& o : objects) {
        o.SetEnabled(false);
    }
    objects.clear();
    for(int i=0; i<MaxComponents; ++i) {
        if (components[i]) {
            components[i]->Clear();
        }
    }
}

ISystem* World::TryAddSystem(SystemID id, std::function<ISystem *(std::vector<int>& components)> constructor) {
    if (id>=systemsIndexed.size()) {
        systemsIndexed.resize(id + 1, 0);
    }
    ISystem* system = systemsIndexed[id];
    if (!system) {
        std::vector<int> componentIndices;
        system = constructor(componentIndices);
        for(auto c : componentIndices) {
            system->componentMask[c] = true;
            if (c>=systemsPerComponent.size()) {
                systemsPerComponent.resize(c + 1);
            }
            systemsPerComponent[c].push_back(system);
        }
        systemsIndexed[id] = system;
        systems.push_back(system);
        system->Initialize();
        
        for(auto& o : objects) {
            if ((o.data->enabledComponents & system->componentMask) == system->componentMask) {
                system->objects.push_back(&o);
                system->ObjectAdded(&o);
            }
        }
    }
    return system;
}

void World::TryRemoveSystem(SystemID id) {
    if (id>=systemsIndexed.size()) return;
    ISystem* system = systemsIndexed[id];
    if (!system) return;
    
    for(auto& o : objects) {
        if ((o.data->enabledComponents & system->componentMask) == system->componentMask) {
            system->ObjectRemoved(&o);
            auto& objects = system->objects;
            objects.erase(std::find(objects.begin(), objects.end(), &o));
        }
    }

    
    for(int i=0; i<MaxComponents; ++i) {
        if (system->componentMask[i]) {
            auto& list = systemsPerComponent[i];
            list.erase(std::find(list.begin(), list.end(), system));
        }
    }
    
    systems.erase(std::find(systems.begin(), systems.end(), system));
    systemsIndexed[id] = 0;
    delete system;
}

void World::DoActions(Actions &actions) {
    for(auto action : actions) {
        action();
    }
    actions.clear();
}