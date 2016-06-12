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
        virtual void Create(int& index) = 0;
        virtual void Reference(int index) = 0;
        virtual void Delete(int index) = 0;
        virtual void Clone(int source, int& index) = 0;
        virtual void* Get(int index) = 0;
        virtual void Clear() = 0;
    };

    template<typename T>
    class Container : public IContainer {
    public:
        Container() : count(0) {}
        virtual ~Container() { }
    
        void Create(int& index) override {
            if (freeEntries.empty()) {
                index = (int)entries.size();
                entries.resize(index + 1);
            } else {
                index = freeEntries.back();
                freeEntries.pop_back();
            }
            Entry& entry = entries[index];
            assert(entry.references == 0);
            entry.object = defaultObject;
            entry.references = 1;
            ++count;
        }
        
        void Reference(int index) override {
            Entry& entry = entries[index];
            assert(entry.references > 0);
            ++entry.references;
        }
        
        void Delete(int index) override {
            Entry& entry = entries[index];
            --entry.references;
            if (entry.references == 0) {
                freeEntries.push_back(index);
                --count;
            }
        }
        
        void Clone(int source, int& index) override {
            Create(index);
            entries[index].object = entries[source].object;
        }
        
        void* Get(int index) override {
            return &entries[index];
        }
        
        int Count() const { return count; }
        
        void Iterate(std::function<void(T* object)> function) {
            for(int i=0; i<entries.size(); ++i) {
                if (entries[i].references > 0) {
                    function(&entries[i].object);
                }
            }
        }
        
        void Clear() override {
            entries.clear();
            freeEntries.clear();
            count = 0;
        }
    private:
    
        struct Entry {
            Entry() : references(0) { }
            T object;
            int references;
        };
        
        int count;
        T defaultObject;
    
        using Entries = std::deque<Entry>;
        Entries entries;
        
        using FreeEntries = std::vector<int>;
        FreeEntries freeEntries;
    };
}