

#include <iostream>
#include <vector>
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
    std::string name;
    std::vector<std::string> children;
};

struct MovementSystem : public GameSystem<Position, Velocity> {

    void ObjectAdded(GameObject* object) {
    
        std::vector<int> bla;
        bla.push_back(2);
        bla.push_back(3);
        
        int sum = 0;
        for(auto i : bla) sum+=i;
        
        int x = object->GetComponent<Position>()->x;
        
        object->AddComponent<Pocket::Transform>();
        object->AddComponent<Pocket::Renderable>();
        
        object->GetComponent<Velocity>()->children.push_back("First");
        object->GetComponent<Velocity>()->children.push_back("Second");
        object->GetComponent<Velocity>()->children.push_back("Third");
        
        
        std::cout << "MovementSystem::ObjectAdded "<<sum<<std::endl;
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