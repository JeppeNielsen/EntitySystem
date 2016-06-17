//
//  UnitTests.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 11/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "LogicTests.hpp"
#include "World.hpp"
#include <cstdlib>

using namespace Pocket;

void LogicTests::RunTests() {

    AddTest("CreateObject", []() {
        World world;
        world.CreateObject();
        return world.ObjectCount()==1;
    });
    
    AddTest("Remove GameObject", []() {
        World world;
        GameObject* object = world.CreateObject();
        object->Remove();
        world.Update(0);
        return world.ObjectCount() == 0;
    });
    
    AddTest("Add/Remove GameObject in root.Children ", []() {
        World world;
        GameObject* object = world.CreateObject();
        bool wasOne = world.ObjectCount() == 1 && world.Root()->Children().size() == 1;
        object->Remove();
        world.Update(0);
        bool wasNone = world.ObjectCount() == 0 && world.Root()->Children().size() == 0;
        return wasOne && wasNone;
    });
    
    AddTest("World::Clear", []() {
        World world;
        world.CreateObject();
        bool wasOne = world.ObjectCount() == 1;
        world.Clear();
        bool wasNone = world.ObjectCount() == 0;
        return wasOne && wasNone;
    });
    
    AddTest("Default parent should be Root", []() {
        World world;
        GameObject* object = world.CreateObject();
        return object->Parent() == world.Root();
    });
    
    AddTest("Parent set to 0 should be child of root", []() {
        World world;
        GameObject* object = world.CreateObject();
        bool parentWasRoot = object->Parent() == world.Root();
        GameObject* parent = world.CreateObject();
        object->Parent() = parent;
        bool objectNotRootChild = std::find(world.Root()->Children().begin(), world.Root()->Children().end(), object)==world.Root()->Children().end();
        bool parentWasParent = object->Parent() == parent;
        object->Parent() = 0;
        bool objectIsRootChild = std::find(world.Root()->Children().begin(), world.Root()->Children().end(), object)!=world.Root()->Children().end();
        return parentWasRoot && parentWasParent && objectIsRootChild && objectNotRootChild;
    });
    
    AddTest("GameObject::AddComponent", []() {
        struct Transform { int x; };
        World world;
        auto object = world.CreateObject();
        auto transform = object->AddComponent<Transform>();
        transform->x = 4;
        return object && transform && transform->x == 4;
    });

    AddTest("double GameObject::AddComponent", []() {
        struct Transform { int x; };
        World world;
        auto object = world.CreateObject();
        auto transform = object->AddComponent<Transform>();
        transform->x = 4;
        auto* transform2 = object->AddComponent<Transform>();
        return object && transform && transform->x == 4 && transform == transform2;
    });
    
    AddTest("GameObject::HasComponent", []() {
        struct Transform { int x; };
        World world;
        auto object = world.CreateObject();
        auto transform = object->AddComponent<Transform>();
        return object && transform && object->HasComponent<Transform>();
    });
    
    AddTest("GameObject::GetComponent", []() {
        struct Transform { int x; };
        World world;
        auto object = world.CreateObject();
        auto transform = object->AddComponent<Transform>();
        auto transform2 = object->GetComponent<Transform>();
        return object && transform && transform == transform2;
    });
    
    
    AddTest("GameObject::RemoveComponent", []() {
        struct Transform { int x; };
        World world;
        auto object = world.CreateObject();
        auto transform = object->AddComponent<Transform>();
        transform->x = 123;
        bool firstTest = object && transform && transform->x == 123;
        object->RemoveComponent<Transform>();
        bool secondTest = object->HasComponent<Transform>() && object->GetComponent<Transform>()->x == 123;
        world.Update(0);
        bool thirdTest = !object->HasComponent<Transform>() && object->GetComponent<Transform>() == 0;
        return firstTest && secondTest && thirdTest;
    });
    
    AddTest("Component reset", []() {
        struct Transform { Transform() : x(666) {} int x; };
        World world;
        auto object = world.CreateObject();
        auto transform = object->AddComponent<Transform>();
        transform->x = 123;
        bool firstTest = object && transform && transform->x == 123;
        object->RemoveComponent<Transform>();
        world.Update(0);
        object->AddComponent<Transform>();
        bool thirdTest = object->GetComponent<Transform>()->x == 666;
        return firstTest && thirdTest;
    });
    
    AddTest("Multiple random GameObject::Add/RemoveComponent", []() {
        struct Transform { int x; };
        World world;
        auto object = world.CreateObject();
        
        for(int i=0; i<100; i++) {
            if (std::rand() % 2==0) {
                object->AddComponent<Transform>();
            } else {
                object->RemoveComponent<Transform>();
            }
        }
        object->RemoveComponent<Transform>();
        world.Update(0);
        return !object->HasComponent<Transform>();
    });
    
    AddTest("Reference component", [](){
        struct Transform { int x; };
        World world;
        auto source = world.CreateObject();
        auto sourceTransform = source->AddComponent<Transform>();
        sourceTransform->x = 123;
        
        auto copy = world.CreateObject();
        auto copyTransform = copy->AddComponent<Transform>(source);
        
        return sourceTransform && copyTransform && sourceTransform == copyTransform;
    });
    
    AddTest("Clone component", [](){
        struct Transform { Transform() : x(666) {} int x; };
        World world;
        auto source = world.CreateObject();
        auto sourceTransform = source->AddComponent<Transform>();
        sourceTransform->x = 123;
        
        auto copy = world.CreateObject();
        auto copyTransform = copy->CloneComponent<Transform>(source);
        
        return sourceTransform &&
        copyTransform &&
        sourceTransform != copyTransform &&
        sourceTransform->x == 123 &&
        copyTransform->x == 123;
    });
    
    AddTest("GameSystem::GameObject Count", [] () {
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> { };
        World world;
        RenderSystem* system = world.CreateSystem<RenderSystem>();
        bool wasNone = system->Objects().size() == 0;
        GameObject* object = world.CreateObject();
        object->AddComponent<Transform>();
        object->AddComponent<Renderable>();
        world.Update(0);
        bool wasOne = system->Objects().size() == 1;
        return wasNone && wasOne;
    });
    
    AddTest("World::Clear -> GameSystem::ObjectRemoved", [] () {
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> { };
        World world;
        RenderSystem* system = world.CreateSystem<RenderSystem>();
        bool wasNone = system->Objects().size() == 0;
        GameObject* object = world.CreateObject();
        object->AddComponent<Transform>();
        object->AddComponent<Renderable>();
        world.Update(0);
        bool wasOne = system->Objects().size() == 1;
        world.Clear();
        bool isNone = system->Objects().size() == 0;
        return wasNone && wasOne && isNone;
    });

    AddTest("World destructor cleanup ", [] () {
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> {
            int* Counter;
            int* ConstuctorDestructorCounter;
            ~RenderSystem() {
                (*ConstuctorDestructorCounter)++;
            }
            void ObjectAdded(GameObject* object) { (*Counter)++; }
            void ObjectRemoved(GameObject* object) { (*Counter)--; }
        };
        int Counter = 0;
        int ConstuctorDestructorCounter = 0;
        bool wasOne;
        {
            World world;
            RenderSystem* system = world.CreateSystem<RenderSystem>();
            system->Counter = &Counter;
            system->ConstuctorDestructorCounter = &ConstuctorDestructorCounter;
            GameObject* object = world.CreateObject();
            object->AddComponent<Transform>();
            object->AddComponent<Renderable>();
            world.Update(0);
            wasOne = Counter == 1;
        }
        return wasOne && Counter == 0 && ConstuctorDestructorCounter == 1;
    });

    AddTest("GameObject::Enabled", [] () {
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> { };
        World world;
        RenderSystem* system = world.CreateSystem<RenderSystem>();
        bool wasNone = system->Objects().size() == 0;
        GameObject* object = world.CreateObject();
        object->AddComponent<Transform>();
        object->AddComponent<Renderable>();
        world.Update(0);
        bool wasOne = system->Objects().size() == 1;
        object->Enabled() = false;
        world.Update(0);
        bool isNone = system->Objects().size() == 0;
        return wasNone && wasOne && isNone;
    });

    AddTest("Add GameSystem", []() {
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> {
        public:
            int numberOfObjects;
            void Initialize() { numberOfObjects = 0; }
            void ObjectAdded(GameObject* o) { numberOfObjects++; }
            void ObjectRemoved(GameObject* o) { numberOfObjects--; }
        };
        
        World world;
        auto object = world.CreateObject();
        object->AddComponent<Transform>();
        object->AddComponent<Renderable>();
        auto system = world.CreateSystem<RenderSystem>();
        bool wasOneObject = world.ObjectCount() == 1;
        world.Update(0);
        bool oneObjectInSystem = system->numberOfObjects == 1;
        world.Clear();
        bool wasZeroObjects = system->numberOfObjects == 0 && world.ObjectCount() == 0;
        auto object1 = world.CreateObject();
        object1->AddComponent<Transform>();
        object1->AddComponent<Renderable>();
        object1->Enabled() = false;
        auto object2 = world.CreateObject();
        object2->AddComponent<Transform>();
        object2->AddComponent<Renderable>();
        bool lastTest = world.ObjectCount() == 2;
        system = world.CreateSystem<RenderSystem>();
        world.Update(0);
        bool systemHasOnlyOneObject = system->numberOfObjects == 1;
        return wasOneObject && oneObjectInSystem && wasZeroObjects && lastTest && systemHasOnlyOneObject;
    });
    
    AddTest("Remove system", []() {
        static int ObjectCount = 0;
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> {
        public:
            void ObjectAdded(GameObject* o) { ObjectCount++; }
            void ObjectRemoved(GameObject* o) {ObjectCount--; }
        };
        
        World world;
        auto object = world.CreateObject();
        object->AddComponent<Transform>();
        object->AddComponent<Renderable>();
        world.Update(0);
        world.CreateSystem<RenderSystem>();
        bool wasOneObject = ObjectCount == 1;
        world.RemoveSystem<RenderSystem>();
        bool hasNoObjects = ObjectCount == 0;
        return wasOneObject && hasNoObjects;
    });
    
    AddTest("World::Trim", []() {
        World world;
        world.CreateObject();
        GameObject* o2 = world.CreateObject();
        GameObject* o3 = world.CreateObject();
        
        bool wasThree = world.ObjectCount() == 3;
        bool wasThreeCapacity = world.CapacityCount() == 3;
        o2->Remove();
        o3->Remove();
        world.Trim();
        bool isCapacityStillThree = world.CapacityCount() == 3;
        world.Update(0);
        world.Trim();
        bool isCapacityOne = world.CapacityCount() == 1;
        bool oneObjectLeft = world.ObjectCount() == 1;
        return wasThree && wasThreeCapacity && isCapacityStillThree && isCapacityOne && oneObjectLeft;
    });
    
    AddTest("World::Trim2", []() {
        World world;
        world.CreateObject();
        GameObject* o2 = world.CreateObject();
        world.CreateObject();
        bool wasThree = world.ObjectCount() == 3;
        bool wasThreeCapacity = world.CapacityCount() == 3;
        o2->Remove();
        world.Trim();
        bool isCapacityStillThree = world.CapacityCount() == 3;
        world.Update(0);
        world.Trim();
        bool twoObjectsLeft = world.ObjectCount() == 2;
        bool isCapacityStillThree2 = world.CapacityCount() == 3;
        return wasThree && wasThreeCapacity && isCapacityStillThree && twoObjectsLeft && isCapacityStillThree2;
    });
    
    AddTest("GameObject.Parent", []() {
        World world;
        auto parent = world.CreateObject();
        auto child = world.CreateObject();
        bool twoObjects = world.ObjectCount() == 2;
        child->Parent() = parent;
        bool hasOneChild = parent->Children().size() == 1;
        return twoObjects && hasOneChild;
    });
    
    AddTest("GameObject.Parent set to null", []() {
        World world;
        auto parent = world.CreateObject();
        auto child = world.CreateObject();
        bool twoObjects = world.ObjectCount() == 2;
        child->Parent() = parent;
        bool hasOneChild = parent->Children().size() == 1;
        child->Parent() = 0;
        bool hasNoChildren = parent->Children().empty();
        return twoObjects && hasOneChild && hasNoChildren;
    });

    AddTest("Hierarchical removal ", []() {
        World world;
        auto parent = world.CreateObject();
        auto child = world.CreateObject();
        auto grandChild = world.CreateObject();
        child->Parent() = parent;
        grandChild->Parent() = child;
        bool threeObjectsInWorld = world.ObjectCount() == 3;
        parent->Remove();
        world.Update(0);
        bool zeroObjectsInWorld = world.ObjectCount() == 0;
        return threeObjectsInWorld && zeroObjectsInWorld;
    });
    
    AddTest("Hierarchical Enabled ", []() {
        static int ObjectCount = 0;
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> {
            void ObjectAdded(GameObject* o) { ObjectCount++; }
            void ObjectRemoved(GameObject* o) {ObjectCount--; }
        };
        World world;
        auto create = [&]() {
            auto o = world.CreateObject();
            o->AddComponent<Transform>();
            o->AddComponent<Renderable>();
            return o;
        };
        
        world.CreateSystem<RenderSystem>();
        auto parent = create();
        auto child = create();
        auto grandChild = create();
        child->Parent() = parent;
        grandChild->Parent() = child;
        world.Update(0);
        bool renderSystemHasThreeObjects = ObjectCount == 3;
        parent->Enabled() = false;
        parent->Enabled() = true;
        parent->Enabled() = false;
        world.Update(0);
        bool renderSystemHasZeroObjects = ObjectCount == 0;
        return renderSystemHasThreeObjects && renderSystemHasZeroObjects;
    });
    
    AddTest("Parent set to disabled object ", []() {
        static int ObjectCount = 0;
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> {
            void ObjectAdded(GameObject* o) { ObjectCount++; }
            void ObjectRemoved(GameObject* o) {ObjectCount--; }
        };
        World world;
        auto create = [&]() {
            auto o = world.CreateObject();
            o->AddComponent<Transform>();
            o->AddComponent<Renderable>();
            return o;
        };
        
        world.CreateSystem<RenderSystem>();
        auto o1 = create();
        auto o2 = create();
        world.Update(0);
        bool renderSystemHasThreeObjects = ObjectCount == 2;
        o1->Enabled() = false;
        o2->Parent() = o1;
        world.Update(0);
        bool renderSystemHasZeroObjects = ObjectCount == 0;
        o2->Parent() = 0;
        world.Update(0);
        bool renderSystemHasOneObject = ObjectCount == 1;
        return renderSystemHasThreeObjects && renderSystemHasZeroObjects && renderSystemHasOneObject;
    });
    
    AddTest("World dtor remove component from system", []() {
        static int ObjectCount = 0;
        struct Transform { int x; };
        struct Renderable { int imageNo; };
        struct RenderSystem : public GameSystem<Transform, Renderable> {
            void ObjectAdded(GameObject* o) { ObjectCount++; }
            void ObjectRemoved(GameObject* o) {ObjectCount--; }
        };
        
        bool oneObjectCreated;
        {
            World world;
            world.CreateSystem<RenderSystem>();
            auto o = world.CreateObject();
            o->AddComponent<Transform>();
            o->AddComponent<Renderable>();
            world.Update(0);
            oneObjectCreated = ObjectCount == 1;
        }
        return ObjectCount == 0;
    });

    
}
