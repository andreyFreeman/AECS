//
//  World.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 23/04/2025.
//

#pragma once

#include <chrono>
#include <iomanip>
#include <Updatable.hpp>
#include <utility>
#include <vector>
#include <ECS/EntityManager.hpp>
#include <FSM/StateMachine.hpp>

#include "ScopedTimer.hpp"

class WorldState final : public FSM::State {
    std::string tag;

public:
    std::vector<std::shared_ptr<Updatable> > updatables;
    std::vector<UpdatableFn> updatableFns;
    std::vector<double> updateTimesMs;
    double updateTimeMs = 0.0;

    explicit WorldState(std::string tag): tag(std::move(tag)) {
    }

    template<Derived<Updatable> T>
    void addUpdatable(const std::shared_ptr<T> &updatable) {
        updatables.push_back(updatable);
        updatableFns.push_back(makeUpdatable<T>(updatable.get()));
        updateTimesMs.emplace_back(0.0);
    }

    void enter() override {
        // std::cout << tag << ": enter()" << std::endl;
    }

    bool update(const float dt) override {
        PROFILE_SCOPE(updateTimeMs);
        if (updatables.empty()) {
            return false;
        }
        bool updated = false;
        for (auto i = 0; i < updatableFns.size(); ++i) {
            const auto& update = updatableFns[i];
            PROFILE_SCOPE(updateTimesMs[i]);
            const auto flag = callUpdate(update, dt);
            updated = flag || updated;
        }
        return updated;
    }

    void exit() override {
        // std::cout << tag << ": exit()" << std::endl;
    }
};

class World final : public Updatable {
    std::shared_ptr<ECS::EntityManager> entityManager;

    std::shared_ptr<WorldState> stateIdle;
    std::shared_ptr<WorldState> statePause;
    std::shared_ptr<WorldState> stateRunning;
    std::unique_ptr<FSM::StateMachine<WorldState> > stateMachine;

public:
    explicit World(const std::shared_ptr<ECS::EntityManager> &entityManager)
        : entityManager(entityManager), stateIdle(std::make_shared<WorldState>("Idle")),
          statePause(std::make_shared<WorldState>("Pause")), stateRunning(std::make_shared<WorldState>("Running")),
          stateMachine(std::make_unique<FSM::StateMachine<WorldState> >()) {
        stateMachine->setState(stateIdle);
    }

    template<Derived<Updatable> T>
    void addUpdatable(const std::shared_ptr<T> &updatable) const {
        addUpdatable<T>(updatable, true);
    }

    template<Derived<Updatable> T>
    void addUpdatable(const std::shared_ptr<T> &updatable, const bool pausable) const {
        stateRunning->addUpdatable<T>(updatable);
        if (!pausable) {
            statePause->addUpdatable<T>(updatable);
        }
    }

    bool update(const float dt) override { return stateMachine->currentState()->update(dt); }

    [[nodiscard]] const std::shared_ptr<ECS::EntityManager> &getEntityManager() const { return entityManager; }

    void pause() const { stateMachine->setState(statePause); }
    void unpause() const { stateMachine->setState(stateRunning); }
    [[nodiscard]] bool isPaused() const { return stateMachine->currentState() == statePause; }
    [[nodiscard]] bool isIdle() const { return stateMachine->currentState() == stateIdle; }

    [[nodiscard]] double getLastUpdateTime() const { return stateMachine->currentState()->updateTimeMs; }
    [[nodiscard]] const std::vector<double>& getUpdateTimesMs() const { return stateMachine->currentState()->updateTimesMs; }
};
