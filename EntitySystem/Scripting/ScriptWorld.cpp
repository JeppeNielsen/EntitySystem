//
//  ScriptWorld.cpp
//  ComponentSystem
//
//  Created by Jeppe Nielsen on 10/02/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "ScriptWorld.hpp"
#include <dlfcn.h>
#include "ScriptParser.hpp"
#include <set>
#include <iostream>

using namespace std;
using namespace Pocket;

vector<string> Compile(const string &cmd) {
    FILE*           fp;
    const int       SIZEBUF = 1234;
    char            buf [SIZEBUF];
    vector<string> out;
    
    if ((fp = popen(cmd.c_str (), "r")) == NULL) {
        return out;
    }
    string cur_string = "";
    while (fgets(buf, sizeof (buf), fp)) {
        cur_string += buf;
    }
    out.push_back(cur_string.substr (0, cur_string.size () - 1));
    pclose(fp);
    return out;
}

ScriptWorld::ScriptWorld() : libHandle(0) { }

string ScriptWorld::ExtractHeaderPath(const std::string &headerFile) {
    size_t lastPath = headerFile.rfind("/");
    return headerFile.substr(0, lastPath);
}

string ScriptWorld::ExtractHeaderName(const std::string &headerFile) {
    size_t lastPath = headerFile.rfind("/");
    return headerFile.substr(lastPath + 1, headerFile.size() - lastPath - 1);
}

void ScriptWorld::SetClangSdkPath(const std::string& clangSdkPath) {
    this->clangSdkPath = clangSdkPath;
}

void ScriptWorld::SetFiles(const std::string& dynamicLibPath, const std::string& scriptingIncludeDir, const std::vector<std::string> &sourceFiles, const std::vector<std::string> &headerFiles) {
    
    this->sourceFiles.clear();
    this->headerNames.clear();
    this->headerPaths.clear();
    
    this->dynamicLibPath = dynamicLibPath;
    this->scriptingIncludeDir = scriptingIncludeDir;
    this->sourceFiles = sourceFiles;
    
    for(auto& header : headerFiles) {
        headerNames.push_back(ExtractHeaderName(header));
    }
    
    //header paths
    {
        std::set<std::string> uniqueHeaderPaths;
        
        uniqueHeaderPaths.insert("-I" + scriptingIncludeDir);
        for(auto& header : headerFiles) {
            uniqueHeaderPaths.insert("-I" + ExtractHeaderPath(header));
        }
        
        for(auto& source : sourceFiles) {
            uniqueHeaderPaths.insert("-I" + ExtractHeaderPath(source));
        }
        
        for(auto& uniqueHeader : uniqueHeaderPaths) {
            headerPaths.push_back(uniqueHeader);
        }
    }
    
    /*
    std::cout << "headerPaths : "<<std::endl;
    for(auto& s : headerPaths) {
        std::cout << s << std::endl;
    }
    
    std::cout << "headerNames : "<<std::endl;
    for(auto& s : headerNames) {
        std::cout << s << std::endl;
    }
    */
}

bool ScriptWorld::Build(bool enableOutput) {

    UnloadLib();
    std::string mainCppFile = scriptingIncludeDir + "/scriptWorld_autogenerated.cpp";
    ExtractScriptClasses();
    
    WriteMainCppFile(mainCppFile);
    
    string compilerPath = clangSdkPath + "bin/clang++";
    string compilerFlags = "-dynamiclib -std=c++11 -stdlib=libc++ -g";
    if (enableOutput) {
        compilerFlags += " -v ";
    }
    string outputFile = "-o " + dynamicLibPath;
    
    string compilerArgs = compilerPath + " " + compilerFlags + " ";
    //compilerArgs += "-I"+clangSdkPath+"/include/c++/v1/ ";
    
    for(auto& header : headerPaths) {
        compilerArgs += header + " ";
    }
    compilerArgs += mainCppFile + " ";
    compilerArgs += outputFile;
    
    if (enableOutput) {
        std::cout << compilerArgs << std::endl;
    }
    
    auto out = Compile(compilerArgs);
    
    if (enableOutput) {
        for(auto s : out) {
            std::cout << s << std::endl;
        }
    }
    return LoadLib();
}

bool ScriptWorld::LoadLib() {
    
    libHandle = dlopen(dynamicLibPath.c_str(), RTLD_LAZY);
    
    if (!libHandle) {
        cerr << "Cannot open '" << dynamicLibPath << "' library: " << dlerror() << '\n';
        return false;
    }

    createSystem = (CreateSystem)dlsym(libHandle, "CreateSystem");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CreateSystem': " << dlsym_error << '\n';
        UnloadLib();
        return false;
    }
    
    countSystems = (CountSystems) dlsym(libHandle, "CountSystems");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CountSystems': " << dlsym_error << '\n';
        UnloadLib();
        return false;
    }
    
    deleteSystem = (DeleteSystem) dlsym(libHandle, "DeleteSystem");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'DeleteSystem': " << dlsym_error << '\n';
        UnloadLib();
        return false;
    }
    
    createComponent = (CreateComponent) dlsym(libHandle, "CreateComponent");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CreateComponent': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    countComponents = (CountComponents) dlsym(libHandle, "CountComponents");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CountComponents': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    deleteComponent = (DeleteComponent) dlsym(libHandle, "DeleteComponent");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'DeleteComponent': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    resetComponent = (ResetComponent) dlsym(libHandle, "ResetComponent");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'ResetComponent': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    getTypeInfo = (GetTypeInfoFunction) dlsym(libHandle, "GetTypeInfo");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'GetTypeInfo': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    deleteTypeInfo = (DeleteTypeInfo) dlsym(libHandle, "DeleteTypeInfo");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'DeleteTypeInfo': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    return true;
}

void ScriptWorld::UnloadLib() {
    if (!libHandle) return;
    dlclose(libHandle);
}
void ScriptWorld::ExtractScriptClasses() {
    ScriptClass allClasses;

    ScriptParser parser;

    parser.ParseCode(
        allClasses,
        sourceFiles[0],
        headerPaths
    );

    //allClasses.Print();
    scriptClasses = parser.ExtractSystemsAndComponents(allClasses);
}

void ScriptWorld::WriteMainCppFile(const std::string &path) {
    ofstream file;
    file.open(path);
    
    WriteMainGameObject(file);
    WriteMainIncludes(file);
    WriteMainSystems(file);
    WriteMainComponents(file);
    WriteMainSerializedComponents(file);
    
    file.close();
}

void ScriptWorld::WriteMainIncludes(std::ofstream &file) {

    file<<"class IGameSystem;"<<std::endl;
    
    auto& systems = scriptClasses.children["Systems"].children;
    
    std::set<std::string> uniqueIncludes;
    for(auto& system : systems) {
        uniqueIncludes.insert(system.second.sourceFile);
    }
    
    file<<"#include \"TypeInfo.hpp\""<<std::endl;
    
    for(auto& headerName : headerNames) {
        file<<"#include \""<<headerName<<"\""<<std::endl;
    }
    
    for(auto& include : uniqueIncludes) {
        file << "#include \"" << include << "\""<< std::endl;
    }
    file<<std::endl;
    
}

void ScriptWorld::WriteMainGameObject(std::ofstream &file) {
    
    const std::string namespaceName = "Pocket::";

    auto& scriptComponents = scriptClasses.children.find("Components")->second.children;
    
    file<<"namespace Pocket {"<<std::endl;
    for(auto& componentName : worldComponentNames) {
        file << "struct "<< componentName.name << ";"<<std::endl;
    }
    file<<"}"<<std::endl;
    
    //file<<"using namespace Pocket;"<<std::endl;
    
        
    for (auto component : scriptComponents) {
        file << "struct "<< component.second.name << ";"<<std::endl;
    }
    
    file << std::endl;
    
    file << "class GameObject {"<<std::endl;
    file << "private:"<<std::endl;
    file << "    virtual void* GetComponent(int componentID) = 0;"<<std::endl;
    file << "    virtual void AddComponent(int componentID) = 0;"<<std::endl;
    file << "    virtual void AddComponent(int componentID, GameObject* referenceObject) = 0;"<<std::endl;
    file << "    virtual void RemoveComponent(int componentID) = 0;"<<std::endl;
    file << "    virtual void CloneComponent(int componentID, GameObject* source) = 0;"<<std::endl;
    
    file << "public:" << std::endl;
    file << "    template<typename T> T* GetComponent() { return (T*)0; }"<<std::endl;
    file << "    template<typename T> T* AddComponent() { }"<<std::endl;
    file << "    template<typename T> void RemoveComponent() { }"<<std::endl;
    file << "    template<typename T> T* CloneComponent(GameObject* source) { }"<<std::endl;
    file << "};"<<std::endl;
    
    for(auto& componentName : worldComponentNames) {
        std::string nameWithNamespace = namespaceName + componentName.name;
        file<<"template<> " << nameWithNamespace  << "* GameObject::GetComponent<"<< nameWithNamespace << ">() { return ("<< nameWithNamespace <<"*) GetComponent("<<componentName.index<<"); }"<<std::endl;
        
        file<<"template<> " << nameWithNamespace  << "* GameObject::AddComponent<"<< nameWithNamespace << ">() { AddComponent("<<componentName.index<<"); return ("<< nameWithNamespace <<"*) GetComponent("<<componentName.index<<"); }"<<std::endl;
     
        file<<"template<> void GameObject::RemoveComponent<"<< nameWithNamespace << ">() { RemoveComponent("<<componentName.index<<"); }"<<std::endl;
        
        file<<"template<> " << nameWithNamespace  << "* GameObject::CloneComponent<"<< nameWithNamespace << ">(GameObject* source) { CloneComponent("<<componentName.index<<", source); return ("<< nameWithNamespace <<"*) GetComponent("<<componentName.index<<"); }"<<std::endl;
    }
    
    int index = (int)worldComponentNames.size();
    for (auto it : scriptComponents) {
        auto& component = it.second;
        file<<"template<> " << component.name  << "* GameObject::GetComponent<"<< component.name << ">() { return ("<< component.name <<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        file<<"template<> " << component.name  << "* GameObject::AddComponent<"<< component.name << ">() { AddComponent("<<index<<"); return ("<< component.name <<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        file<<"template<> void GameObject::RemoveComponent<"<< component.name << ">() { RemoveComponent("<<index<<"); }"<<std::endl;
        
        file<<"template<> " << component.name  << "* GameObject::CloneComponent<"<< component.name << ">(GameObject* source) { CloneComponent("<<index<<", source); return ("<< component.name <<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        index++;
    }
}

void ScriptWorld::WriteMainSystems(std::ofstream &file) {
    
    auto& systems = scriptClasses.children["Systems"].children;
    
    {
       file<<"extern \"C\" int CountSystems() {"<<std::endl;
       file<<"   return "<< systems.size() <<";"<<std::endl;
       file<<"}"<<std::endl;
    }
    
    {
        file<<"extern \"C\" IGameSystem* CreateSystem(int systemID) {"<<std::endl;
            file << "   switch (systemID) { " << std::endl;
                int index = 0;
                for(auto& system : systems) {
                    file<<"      case "<<index <<":"<<" return new "<<system.second.name<<"();"<<std::endl;
                    index++;
                }
        file<<"      default: return 0;"<<std::endl;
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }
    
    {
       file<<"extern \"C\" void DeleteSystem(IGameSystem* scriptSystem) {"<<std::endl;
       file<<"   delete scriptSystem; "<<std::endl;
       file<<"}"<<std::endl;
    }
}

void ScriptWorld::WriteMainComponents(std::ofstream &file) {
    
    auto& components = scriptClasses.children["Components"].children;

    {
       file<<"extern \"C\" int CountComponents() {"<<std::endl;
       file<<"   return "<< components.size() <<";"<<std::endl;
       file<<"}"<<std::endl;
    }
    
    {
        file<<"extern \"C\" void* CreateComponent(int componentID) {"<<std::endl;
        file << "   switch (componentID) { " << std::endl;
        int index = (int)worldComponentNames.size();
        for(auto& component : components) {
            file<<"      case "<<index <<":"<<" return new "<<component.second.name<<"();"<<std::endl;
            index++;
        }
        file<<"      default: return 0;"<<std::endl;
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }
    
    {
       file<<"extern \"C\" void DeleteComponent(int componentID, void* component) {"<<std::endl;
        file << "   switch (componentID) { " << std::endl;
        int index = (int)worldComponentNames.size();
        for(auto& component : components) {
            file<<"      case "<<index <<":"<<" { delete (("<<component.second.name<<"*)component); break; }"<<std::endl;
            index++;
        }
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }
    
    {
       file<<"extern \"C\" void ResetComponent(int componentID, void* c, void* s) {"<<std::endl;
            file << "   switch (componentID) { " << std::endl;
                int index = (int)worldComponentNames.size();
                for(auto& component : components) {
                    file<<"      case "<<index <<":"<<" { "<<component.second.name<<"* co = ("<<component.second.name<<"*)c; "<<std::endl;
                    file<<"      "<<component.second.name<<"* so = (("<<component.second.name<<"*)s);"<<std::endl;
                    file<<"        co->operator=(*so);             break; }"<<std::endl;
                    index++;
                }
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }

    file<<std::endl;
}

void ScriptWorld::WriteMainSerializedComponents(std::ofstream &file) {

    auto& components = scriptClasses.children["Components"].children;

    file<<"extern \"C\" Pocket::TypeInfo* GetTypeInfo(int componentID, void* componentPtr) {"<<std::endl;
        file << "   switch (componentID) { " << std::endl;
            int index = 0;
            for(auto& componentIt : components) {
                auto& component = componentIt.second;
                file<<"      case "<<index <<": {"<<std::endl;
                file<<"      "<<component.name<<"* component = ("<<component.name<<"*)componentPtr;"<<std::endl;
                file<<"	      Pocket::TypeInfo* info = new Pocket::TypeInfo();"<<std::endl;
                file<<"	      info->name = \""<<component.name<<"\";"<<std::endl;
                std::set<std::string> uniqueFields;
                for(auto& f : component.fields) {
                    uniqueFields.insert(f.name);
                }
                
                for(auto& field : uniqueFields) {
                    //if (field.type == "int" ||
                      //  field.type == "string" ||
                        //field.type == "float" ||
                       // field.type == "Vector3") {
                file<<"	      info->AddField(component->"<< field <<", \""<<field<<"\");"<<std::endl;
                //file<< "// field.type == " << field.type << std::endl;
                    //}
                }
                file<<"      return info;"<<std::endl;
                
                file<<"      break; }"<<std::endl;
                 // return new "<<system.second.name<<"();"<<std::endl;
                index++;
            }
    file<<"      default: return 0;"<<std::endl;
    file<<"   }"<<std::endl;
    file<<"}"<<std::endl;

    file<<"extern \"C\" void DeleteTypeInfo(Pocket::TypeInfo* typeInfo) {"<<std::endl;
    file << "delete typeInfo;"<<std::endl;
    file<<"}"<<std::endl;
}

bool ScriptWorld::FindComponentIndex(std::string componentName, bool &staticComponent, int& index) {
    auto& scriptComponents = scriptClasses.children["Components"].children;
    {
        index = 0;
        for(auto& scriptComponent : scriptComponents) {
            if (scriptComponent.second.name == componentName) {
                staticComponent = false;
                return true;
            }
            index++;
        }
    }
    
    index = -1;
    
    for(auto& worldComponentName : worldComponentNames) {
        if (componentName == worldComponentName.name) {
            index = worldComponentName.index;
            break;
        }
    }
    
    if (index>=0) {
        staticComponent = true;
        return true;
    }
    
    return false;
}

void ScriptWorld::SetWorldType(GameWorld& world) {
    worldComponentNames.clear();
    for(int i=0; i<world.componentNames.size(); ++i) {
        size_t namespaceColons = world.componentNames[i].find("::");
        std::string nameWithoutNamespace;
        if (namespaceColons!=std::string::npos) {
            nameWithoutNamespace = world.componentNames[i].substr(namespaceColons+2, world.componentNames[i].size() - namespaceColons-2);
        } else {
            nameWithoutNamespace = world.componentNames[i];
        }
        if (nameWithoutNamespace!="") {
            worldComponentNames.push_back({ nameWithoutNamespace, i });
        }
    }
}

bool ScriptWorld::AddGameWorld(GameWorld& world) {
    if (!libHandle) return false;
    
    int numberOfComponents = countComponents();
    componentCount = numberOfComponents;

    baseComponentIndex = (int)world.components.size();
    baseSystemIndex = (int)world.systems.size();
    
    auto& scriptSystems = scriptClasses.children["Systems"].children;
    
    int index = 0;
    for (auto& scriptSystem : scriptSystems) {
        int systemIndex = baseSystemIndex + index;
        world.TryAddSystem(systemIndex, [this, &scriptSystem, index](std::vector<int>& components) {
            
            for (auto& component : scriptSystem.second.templateArguments) {
                int componentIndex;
                bool staticComponent;
                if (FindComponentIndex(component, staticComponent, componentIndex)) {
                    components.push_back(staticComponent ? componentIndex : (baseComponentIndex + componentIndex));
                }
            }
            
            return createSystem(index);
        });
        world.deleteSystems[world.deleteSystems.size()-1] = [this, &world, systemIndex]() {
            deleteSystem(world.systems[systemIndex]);
        };
        index++;
    }

    for(int i=0; i<numberOfComponents; ++i) {
        int componentIndex = baseComponentIndex + i;
        world.TryAddComponentContainer(componentIndex, [this, componentIndex](std::string& componentName) {
            Container<ScriptComponent>* container = new Container<ScriptComponent>();
            container->defaultObject.world = this;
            container->defaultObject.componentID = componentIndex;
            container->defaultObject.data = createComponent(componentIndex);
            componentName = "ScriptComponent";
            return container;
        });
    }
    return true;
}

void ScriptWorld::RemoveGameWorld(GameWorld& world) {
    /*world.ClearScripingData([this] (auto scriptSystem){
        deleteSystem(scriptSystem);
    });
    */
}

TypeInfo ScriptWorld::GetTypeInfo(GameObject& object, ComponentID id) {
    void* component = object.GetComponent(id);
    if (!component) {
        return TypeInfo();
    }
    TypeInfo* info = getTypeInfo(id, component);
    TypeInfo t;
    t.UpdateFromPointer(info);
    deleteTypeInfo(info);
    return t;
}

int ScriptWorld::ComponentCount() { return componentCount; }

template<> void* Container<ScriptComponent>::Get(int index) {
    return entries[index].data;
}


