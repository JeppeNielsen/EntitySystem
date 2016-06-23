namespace Pocket {
struct EngineComponent;
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
template<> Pocket::EngineComponent* GameObject::GetComponent<Pocket::EngineComponent>() { return (Pocket::EngineComponent*) GetComponent(27); }
template<> Pocket::EngineComponent* GameObject::AddComponent<Pocket::EngineComponent>() { AddComponent(27); return (Pocket::EngineComponent*) GetComponent(27); }
template<> void GameObject::RemoveComponent<Pocket::EngineComponent>() { RemoveComponent(27); }
template<> Pocket::EngineComponent* GameObject::CloneComponent<Pocket::EngineComponent>(GameObject* source) { CloneComponent(27, source); return (Pocket::EngineComponent*) GetComponent(27); }
template<> Position* GameObject::GetComponent<Position>() { return (Position*) GetComponent(28); }
template<> Position* GameObject::AddComponent<Position>() { AddComponent(28); return (Position*) GetComponent(28); }
template<> void GameObject::RemoveComponent<Position>() { RemoveComponent(28); }
template<> Position* GameObject::CloneComponent<Position>(GameObject* source) { CloneComponent(28, source); return (Position*) GetComponent(28); }
template<> Velocity* GameObject::GetComponent<Velocity>() { return (Velocity*) GetComponent(29); }
template<> Velocity* GameObject::AddComponent<Velocity>() { AddComponent(29); return (Velocity*) GetComponent(29); }
template<> void GameObject::RemoveComponent<Velocity>() { RemoveComponent(29); }
template<> Velocity* GameObject::CloneComponent<Velocity>(GameObject* source) { CloneComponent(29, source); return (Velocity*) GetComponent(29); }
class IGameSystem;
#include "TypeInfo.hpp"
#include "Property.hpp"
#include "example_temp.cpp"

extern "C" int CountSystems() {
   return 1;
}
extern "C" IGameSystem* CreateSystem(int systemID) {
   switch (systemID) { 
      case 11: return new MovementSystem();
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
      case 28: return new Position();
      case 29: return new Velocity();
      default: return 0;
   }
}
extern "C" void DeleteComponent(int componentID, void* component) {
   switch (componentID) { 
      case 28: { delete ((Position*)component); break; }
      case 29: { delete ((Velocity*)component); break; }
   }
}
extern "C" void ResetComponent(int componentID, void* c, void* s) {
   switch (componentID) { 
      case 28: { Position* co = (Position*)c; 
      Position* so = ((Position*)s);
        co->operator=(*so);             break; }
      case 29: { Velocity* co = (Velocity*)c; 
      Velocity* so = ((Velocity*)s);
        co->operator=(*so);             break; }
   }
}

extern "C" Pocket::TypeInfo* GetTypeInfo(int componentID, void* componentPtr) {
   switch (componentID) { 
      case 28: {
      Position* component = (Position*)componentPtr;
	      Pocket::TypeInfo* info = new Pocket::TypeInfo();
	      info->name = "Position";
	      info->AddField(component->x, "x");
      return info;
      break; }
      case 29: {
      Velocity* component = (Velocity*)componentPtr;
	      Pocket::TypeInfo* info = new Pocket::TypeInfo();
	      info->name = "Velocity";
	      info->AddField(component->x, "x");
      return info;
      break; }
      default: return 0;
   }
}
extern "C" void DeleteTypeInfo(Pocket::TypeInfo* typeInfo) {
delete typeInfo;
}
