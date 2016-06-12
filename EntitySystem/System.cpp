//
//  System.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 08/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "System.hpp"
#include "World.hpp"

using namespace Pocket;

ISystem::ISystem() : world(0) {}
ISystem::~ISystem() {}

void ISystem::TryAddComponentContainer(ComponentID id, std::function<IContainer *()> constructor) {
    if (!world->components[id]) {
        world->components[id] = constructor();
    }
}

void ISystem::Initialize() {}
void ISystem::ObjectAdded(Pocket::Object *object) {}
void ISystem::ObjectRemoved(Pocket::Object *object) {}
void ISystem::Update(float dt) {}
void ISystem::Render() {}
const ObjectCollection& ISystem::Objects() const { return objects; }