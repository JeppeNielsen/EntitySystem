

#include <iostream>
#include "GameSystem.hpp"

struct Position {
    Position() : x(0), y(0) {}
    int x;
    int y;
};

struct Velocity {
    Velocity() : vx(0), vy(0) {}
    int vx;
    int vy;
};

struct MovementSystem : public GameSystem<Position, Velocity> {

    void ObjectAdded(GameObject* object) {
        int x = object->GetComponent<Position>()->x;
        
        std::cout << "MovementSystem::ObjectAdded "<<x<<std::endl;
    }
 
    void ObjectRemoved(GameObject* object) {
        std::cout << "MovementSystem::ObjectRemoved"<<std::endl;
    }
 
    void Update(float dt) {
        for(auto o : Objects()) {
            o->GetComponent<Position>()->x += o->GetComponent<Velocity>()->vx;
            o->GetComponent<Position>()->y += o->GetComponent<Velocity>()->vy;
            std::cout << " x:" << o->GetComponent<Position>()->x<<
                " y:" << o->GetComponent<Position>()->y<<std::endl;
        }
    }
};