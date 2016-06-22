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
template<> Pocket::Transform* GameObject::GetComponent<Pocket::Transform>() { return (Pocket::Transform*) GetComponent(27); }
template<> Pocket::Transform* GameObject::AddComponent<Pocket::Transform>() { AddComponent(27); return (Pocket::Transform*) GetComponent(27); }
template<> void GameObject::RemoveComponent<Pocket::Transform>() { RemoveComponent(27); }
template<> Pocket::Transform* GameObject::CloneComponent<Pocket::Transform>(GameObject* source) { CloneComponent(27, source); return (Pocket::Transform*) GetComponent(27); }
template<> Pocket::Renderable* GameObject::GetComponent<Pocket::Renderable>() { return (Pocket::Renderable*) GetComponent(28); }
template<> Pocket::Renderable* GameObject::AddComponent<Pocket::Renderable>() { AddComponent(28); return (Pocket::Renderable*) GetComponent(28); }
template<> void GameObject::RemoveComponent<Pocket::Renderable>() { RemoveComponent(28); }
template<> Pocket::Renderable* GameObject::CloneComponent<Pocket::Renderable>(GameObject* source) { CloneComponent(28, source); return (Pocket::Renderable*) GetComponent(28); }
template<> Position* GameObject::GetComponent<Position>() { return (Position*) GetComponent(29); }
template<> Position* GameObject::AddComponent<Position>() { AddComponent(29); return (Position*) GetComponent(29); }
template<> void GameObject::RemoveComponent<Position>() { RemoveComponent(29); }
template<> Position* GameObject::CloneComponent<Position>(GameObject* source) { CloneComponent(29, source); return (Position*) GetComponent(29); }
template<> Velocity* GameObject::GetComponent<Velocity>() { return (Velocity*) GetComponent(30); }
template<> Velocity* GameObject::AddComponent<Velocity>() { AddComponent(30); return (Velocity*) GetComponent(30); }
template<> void GameObject::RemoveComponent<Velocity>() { RemoveComponent(30); }
template<> Velocity* GameObject::CloneComponent<Velocity>(GameObject* source) { CloneComponent(30, source); return (Velocity*) GetComponent(30); }
class IGameSystem;
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
      case 29: return new Position();
      case 30: return new Velocity();
      default: return 0;
   }
}
extern "C" void DeleteComponent(int componentID, void* component) {
   switch (componentID) { 
      case 29: { delete ((Position*)component); break; }
      case 30: { delete ((Velocity*)component); break; }
   }
}
extern "C" void ResetComponent(int componentID, void* c, void* s) {
   switch (componentID) { 
      case 29: { Position* co = (Position*)c; 
      Position* so = ((Position*)s);
        co->operator=(*so);             break; }
      case 30: { Velocity* co = (Velocity*)c; 
      Velocity* so = ((Velocity*)s);
        co->operator=(*so);             break; }
   }
}

extern "C" Pocket::TypeInfo* GetTypeInfo(int componentID, void* componentPtr) {
   switch (componentID) { 
      case 29: {
      Position* component = (Position*)componentPtr;
	      Pocket::TypeInfo* info = new Pocket::TypeInfo();
	      info->name = "Position";
	      info->AddField(component->x, "x");
	      info->AddField(component->y, "y");
      return info;
      break; }
      case 30: {
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
