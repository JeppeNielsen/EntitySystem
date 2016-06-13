//
//  Container.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <deque>
#include <vector>
#include <assert.h>
#include <functional>

namespace Pocket {

    class IContainer {
    public:
        virtual ~IContainer();
        virtual void* Create() = 0;
        virtual void Reference(void* element) = 0;
        virtual void Delete(void* element) = 0;
        virtual void* Clone(void* source) = 0;
        virtual void* Get(int index) = 0;
        virtual void Clear() = 0;
    };

    template<typename T>
    class Container : public IContainer {
    public:
        Container() : count(0) {}
        virtual ~Container() { }
    
        void* Create() override {
            int freeIndex;
            if (freeIndicies.empty()) {
                freeIndex = (int)references.size();
                references.resize(freeIndex + 1);
                entries.resize(freeIndex + 1, defaultObject);
            } else {
                freeIndex = freeIndicies.back();
                freeIndicies.pop_back();
            }
            
            ++count;
        
            references[freeIndex] = 1;
            return &entries[freeIndex];;
        }
        
        void Reference(void* element) override {
            for(int i=0; i<entries.size(); ++i) {
                if (&entries[i]==element) {
                    assert(references[i] > 0);
                    ++references[i];
                    return;
                }
            }
        }
        
        void Delete(void* element) override {
            return;
            for(int i=0; i<entries.size(); ++i) {
                if (&entries[i]==element) {
                    assert(references[i] > 0);
                    --references[i];
                    if (references[i]==0) {
                        --count;
                        freeIndicies.push_back(i);
                    }
                    return;
                }
            }
        }
        
        void* Clone(void* source) override {
            T* clone = (T*)Create();
            T* s = (T*)source;
            (*clone) = (*s);
            return clone;
        }
        
        void* Get(int index) override {
            return &entries[index];
        }
        
        int Count() const { return count; }
        
        void Iterate(std::function<void(T* object)> function) {
            for(int i=0; i<references.size(); ++i) {
                if (references[i] > 0) {
                    function(&entries[i]);
                }
            }
        }
        
        void Clear() override {
            entries.clear();
            references.clear();
            count = 0;
        }
    private:
        
        int count;
        T defaultObject;
    
        using Entries = std::deque<T>;
        Entries entries;
        
        using References = std::vector<int>;
        References references;
        
        using FreeIndicies = std::vector<int>;
        FreeIndicies freeIndicies;
        
    };
}