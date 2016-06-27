#include <string>
#include <vector>
#include "Property.hpp"
template<typename T> struct FieldInfoIndexer { static int Index() { return 0; } };
template<> struct FieldInfoIndexer<int> { static int Index() { return 0; } };
template<> struct FieldInfoIndexer<std::vector<int>> { static int Index() { return 1; } };
template<> struct FieldInfoIndexer<Pocket::Property<int>> { static int Index() { return 2; } };
template<> struct FieldInfoIndexer<float> { static int Index() { return 3; } };
template<> struct FieldInfoIndexer<std::vector<float>> { static int Index() { return 4; } };
template<> struct FieldInfoIndexer<Pocket::Property<float>> { static int Index() { return 5; } };
template<> struct FieldInfoIndexer<double> { static int Index() { return 6; } };
template<> struct FieldInfoIndexer<std::vector<double>> { static int Index() { return 7; } };
template<> struct FieldInfoIndexer<Pocket::Property<double>> { static int Index() { return 8; } };
template<> struct FieldInfoIndexer<std::__1::basic_string<char>> { static int Index() { return 9; } };
template<> struct FieldInfoIndexer<std::vector<std::__1::basic_string<char>>> { static int Index() { return 10; } };
template<> struct FieldInfoIndexer<Pocket::Property<std::__1::basic_string<char> >> { static int Index() { return 11; } };

namespace Pocket {
}
struct Position;

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
template<> Position* GameObject::GetComponent<Position>() { return (Position*) GetComponent(0); }
template<> Position* GameObject::AddComponent<Position>() { AddComponent(0); return (Position*) GetComponent(0); }
template<> void GameObject::RemoveComponent<Position>() { RemoveComponent(0); }
template<> Position* GameObject::CloneComponent<Position>(GameObject* source) { CloneComponent(0, source); return (Position*) GetComponent(0); }
class IGameSystem;
#include "TypeInfo.hpp"
#include "Property.hpp"
#include "example_temp.cpp"

extern "C" int CountSystems() {
   return 1;
}
extern "C" IGameSystem* CreateSystem(int systemID) {
   switch (systemID) { 
      case 0: return new PositionSystem();
      default: return 0;
   }
}
extern "C" void DeleteSystem(IGameSystem* scriptSystem) {
   delete scriptSystem; 
}
extern "C" int CountComponents() {
   return 1;
}
extern "C" void* CreateComponent(int componentID) {
   switch (componentID) { 
      case 0: return new Position();
      default: return 0;
   }
}
extern "C" void DeleteComponent(int componentID, void* component) {
   switch (componentID) { 
      case 0: { delete ((Position*)component); break; }
   }
}
extern "C" void ResetComponent(int componentID, void* c, void* s) {
   switch (componentID) { 
      case 0: { Position* co = (Position*)c; 
      Position* so = ((Position*)s);
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
      return info;
      break; }
      default: return 0;
   }
}
extern "C" void DeleteTypeInfo(Pocket::TypeInfo* typeInfo) {
delete typeInfo;
}
