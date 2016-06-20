

#include <iostream>
#include "GameSystem.hpp"

struct Position {
    int x;
    int y;
};

struct Velocity {
    int vx;
    int vy;
};

struct MovementSystem : public GameSystem<Position, Velocity> {

    void ObjectAdded(GameObject* object) {
        std::cout << "MovementSystem::ObjectAdded"<<std::endl;
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