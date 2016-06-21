namespace Pocket {
struct Transform;
struct Renderable;
}
struct Position;
struct Velocity;

class GameObject {
private:
    virtual void* GetComponent(int componentID) = 0;
    virtual void AddComponent(int componentID) = 0;
    virtual void AddComponent(int componentID, GameObject* referenceObject) = 0;
    virtual void RemoveComponent(int componentID) = 0;
    virtual void CloneComponent(int componentID, GameObject* source) = 0;
public:
    template<typename T> T* GetComponent() { return (T*)0; }
    template<typename T> T* AddComponent() { }
    template<typename T> void RemoveComponent() { }
    template<typename T> T* CloneComponent(GameObject* source) { }
};
template<> Pocket::Transform* GameObject::GetComponent<Pocket::Transform>() { return (Pocket::Transform*) GetComponent(0); }
template<> Pocket::Transform* GameObject::AddComponent<Pocket::Transform>() { AddComponent(0); return (Pocket::Transform*) GetComponent(0); }
template<> void GameObject::RemoveComponent<Pocket::Transform>() { RemoveComponent(0); }
template<> Pocket::Transform* GameObject::CloneComponent<Pocket::Transform>(GameObject* source) { CloneComponent(0, source); return (Pocket::Transform*) GetComponent(0); }
template<> Pocket::Renderable* GameObject::GetComponent<Pocket::Renderable>() { return (Pocket::Renderable*) GetComponent(1); }
template<> Pocket::Renderable* GameObject::AddComponent<Pocket::Renderable>() { AddComponent(1); return (Pocket::Renderable*) GetComponent(1); }
template<> void GameObject::RemoveComponent<Pocket::Renderable>() { RemoveComponent(1); }
template<> Pocket::Renderable* GameObject::CloneComponent<Pocket::Renderable>(GameObject* source) { CloneComponent(1, source); return (Pocket::Renderable*) GetComponent(1); }
template<> Position* GameObject::GetComponent<Position>() { return (Position*) GetComponent(2); }
template<> Position* GameObject::AddComponent<Position>() { AddComponent(2); return (Position*) GetComponent(2); }
template<> void GameObject::RemoveComponent<Position>() { RemoveComponent(2); }
template<> Position* GameObject::CloneComponent<Position>(GameObject* source) { CloneComponent(2, source); return (Position*) GetComponent(2); }
template<> Velocity* GameObject::GetComponent<Velocity>() { return (Velocity*) GetComponent(3); }
template<> Velocity* GameObject::AddComponent<Velocity>() { AddComponent(3); return (Velocity*) GetComponent(3); }
template<> void GameObject::RemoveComponent<Velocity>() { RemoveComponent(3); }
template<> Velocity* GameObject::CloneComponent<Velocity>(GameObject* source) { CloneComponent(3, source); return (Velocity*) GetComponent(3); }
#include "TypeInfo.hpp"
#include "Property.hpp"
#include "ScriptExample.cpp"

extern "C" int CountSystems() {
   return 1;
}
extern "C" IGameSystem* CreateSystem(int systemID) {
   switch (systemID) { 
      case 0: return new MovementSystem();
      default: return 0;
   }
}
extern "C" void DeleteSystem(IGameSystem* scriptSystem) {
   delete scriptSystem; 
}
extern "C" int CountComponents() {
   return 2;
}
extern "C" void* CreateComponent(int componentID) {
   switch (componentID) { 
      case 2: return new Position();
      case 3: return new Velocity();
      default: return 0;
   }
}
extern "C" void DeleteComponent(int componentID, void* component) {
   switch (componentID) { 
      case 2: { delete ((Position*)component); break; }
      case 3: { delete ((Velocity*)component); break; }
   }
}
extern "C" void ResetComponent(int componentID, void* c, void* s) {
   switch (componentID) { 
      case 2: { Position* co = (Position*)c; 
      Position* so = ((Position*)s);
        co->operator=(*so);             break; }
      case 3: { Velocity* co = (Velocity*)c; 
      Velocity* so = ((Velocity*)s);
        co->operator=(*so);             break; }
   }
}

extern "C" Pocket::TypeInfo* GetTypeInfo(int componentID, void* componentPtr) {
   switch (componentID) { 
      case 0: {
      Position* component = (Position*)componentPtr;
	      Pocket::TypeInfo* info = new Pocket::TypeInfo();
	      info->name = "Position";
	      info->AddField(component->x, "x");
	      info->AddField(component->y, "y");
      return info;
      break; }
      case 1: {
      Velocity* component = (Velocity*)componentPtr;
	      Pocket::TypeInfo* info = new Pocket::TypeInfo();
	      info->name = "Velocity";
	      info->AddField(component->vx, "vx");
	      info->AddField(component->vy, "vy");
      return info;
      break; }
      default: return 0;
   }
}
extern "C" void DeleteTypeInfo(Pocket::TypeInfo* typeInfo) {
delete typeInfo;
}
