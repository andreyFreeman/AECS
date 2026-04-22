//
// Created by ANDREY KLADOV on 24/10/2025.
//

#pragma once

#pragma once
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <iomanip>
#include <World.hpp>

class WorldUpdater {
public:
    WorldUpdater(std::shared_ptr<World> world, float fixedDtSeconds)
        : world(std::move(world)),
          fixedDt(fixedDtSeconds),
          running(false) {}

    void start() {
        if (running) return;
        running = true;

        updateThread = std::thread([this]() { this->run(); });
    }

    void stop() {
        if (!running) return;
        running = false;
        if (updateThread.joinable())
            updateThread.join();
    }

    ~WorldUpdater() {
        stop();
    }

private:
    std::shared_ptr<World> world;
    float fixedDt;
    std::atomic<bool> running;
    std::thread updateThread;

    void run() const {
        using clock = std::chrono::steady_clock;
        while (running) {
            const auto startUpdate = clock::now();
            world->update(fixedDt);
            const auto endUpdate = clock::now();
            const double updateMs = std::chrono::duration<double, std::milli>(endUpdate - startUpdate).count();
            const double targetMs = fixedDt * 1000.0;
            const double sleepMs = (targetMs - updateMs) * 0.9;
            if (sleepMs > 0.0) {
                // std::cout << "[WorldUpdateThread] Sleeping for " << std::fixed << std::setprecision(3) << sleepMs << " ms\n";
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepMs));
            } else {
                std::cout << "[WorldUpdateThread] Update took longer than frame ("<< std::fixed << std::setprecision(3) << updateMs << " ms)\n";
            }
        }
    }
};
