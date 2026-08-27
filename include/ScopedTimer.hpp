//
// Created by ANDREY KLADOV on 27/08/2026.
//

#pragma once
#include <chrono>
#include <ECS/Entity.h>

#if TIME_PROFILE_MODE
    class ScopedTimer {
        std::chrono::steady_clock::time_point start;
        double& outResult;
    public:
        explicit ScopedTimer(double& outMs)
            : start(std::chrono::steady_clock::now()), outResult(outMs) {}

        ~ScopedTimer() {
            outResult = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count();
        }
    };
#define PROFILE_SCOPE(outVar) ScopedTimer scopedTimerVar(outVar)
#else
#define PROFILE_SCOPE(outVar)
#endif