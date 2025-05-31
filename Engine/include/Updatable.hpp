//
//  Updatable.hpp
//  arcade_ninja
//
//  Created by ANDREY KLADOV on 23/04/2025.
//

#pragma once

class Updatable {
public:
    virtual bool update(float dt) = 0;
    virtual ~Updatable() = default;
};
