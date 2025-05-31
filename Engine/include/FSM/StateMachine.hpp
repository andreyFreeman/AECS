//
//  StateMachine.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 30/04/2025.
//

#pragma once

#include <Updatable.hpp>
#include <Templates.hpp>
#include <memory>

namespace FSM {

class State: public Updatable {
public:
    ~State() override = default;
    virtual void enter() {}
    virtual void exit() {}
    virtual State* next() { return nullptr; }
};

template<Derived<State> T>
class StateMachine final: public Updatable {
    std::shared_ptr<T> current = nullptr;

public:
    StateMachine() = default;
    
    void setState(const std::shared_ptr<T>& newState) {
        if (current != nullptr) current->exit();
        current = newState;
        if (current != nullptr) current->enter();
    }

    bool update(float dt) override {
        if (current) {
            return current->update(dt);
        }
        return false;
    }

    const std::shared_ptr<T>& currentState() const { return current; }
};

}
