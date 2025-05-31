//
//  World.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 23/04/2025.
//

#pragma once

#include <iomanip>
#include <Updatable.hpp>
#include <vector>
#include <ECS/EntityManager.hpp>
#include <FSM/StateMachine.hpp>

class WorldState final: public FSM::State {
    std::string tag;
    
public:
    
    std::vector<std::shared_ptr<Updatable>> updatables;
    
    using duration = std::chrono::duration<double>;

    WorldState(const std::string& tag): tag(tag) { }

    void enter() override { std::cout << tag << ": enter()" << std::endl; }

    bool update(float dt) override {
        if (updatables.empty()) {
            return false;
        }
        auto start = std::chrono::high_resolution_clock::now();
        bool updated = false;
        for (const auto &updatable : updatables) {
            auto startSystem = std::chrono::high_resolution_clock::now();
            bool flag = updatable->update(dt);
            duration elapsed = std::chrono::high_resolution_clock::now() - startSystem;
            double elapsedTime = elapsed.count() * 1000;
            std::cout << "\t[" << std::fixed << std::setprecision(3) << elapsedTime << "][SYSTEM]: " << flag << std::endl;
            updated = flag || updated;
        }
        duration elapsed = std::chrono::high_resolution_clock::now() - start;
        double elapsedTime = elapsed.count() * 1000;
        std::cout << "[" << std::fixed << std::setprecision(3) << elapsedTime << "][WORLD][" << tag << "]: " << updated << std::endl;
        return updated;
    }

    void exit() override { std::cout << tag << ": exit()" << std::endl; }
};

class World final: public Updatable {
    std::shared_ptr<ECS::EntityManager> entityManager;
    
    std::shared_ptr<WorldState> stateIdle;
    std::shared_ptr<WorldState> statePause;
    std::shared_ptr<WorldState> stateRunning;
    std::unique_ptr<FSM::StateMachine<WorldState>> stateMachine;
public:
    
    World(const std::shared_ptr<ECS::EntityManager>& entityManager)
        : entityManager(entityManager), stateIdle(std::make_shared<WorldState>("Idle")),
          statePause(std::make_shared<WorldState>("Pause")), stateRunning(std::make_shared<WorldState>("Running")),
          stateMachine(std::make_unique<FSM::StateMachine<WorldState>>()) {
        stateMachine->setState(stateIdle);
    }

    void addUpdatable(const std::shared_ptr<Updatable>& updatable) const { addUpdatable(updatable, true); }

    void addUpdatable(const std::shared_ptr<Updatable>& updatable, bool pausable) const {
        stateRunning->updatables.push_back(updatable);
        if (!pausable) {
            statePause->updatables.push_back(updatable);
        }
    }

    bool update(float dt) override { return stateMachine->currentState()->update(dt); }

    const std::shared_ptr<ECS::EntityManager> &getEntityManager() const { return entityManager; }

    void pause() const { stateMachine->setState(statePause); }
    void unpause() const { stateMachine->setState(stateRunning); }
    bool isPaused() const { return stateMachine->currentState() == statePause; }
    bool isIdle() const { return stateMachine->currentState() == stateIdle; }
};
