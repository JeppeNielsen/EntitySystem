//
//  mainTypeToIndex.cpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 25/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include <vector>
#include "Property.hpp"
#include "TypeInfo.hpp"
#include "TypeIndexList.hpp"

using namespace Pocket;

int main_nonon() {

    TypeIndexList types;
    
    types.Add<int>();
    types.Add<float>();
    types.Add<double>();
    
    return 0;
}