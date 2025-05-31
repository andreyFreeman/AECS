//
// Created by ANDREY KLADOV on 23/05/2025.
//

#pragma once

#include <functional>
#include <vector>
#include <utility>
#include <algorithm>
#include <future>
#include <memory>
#include "Archetype.hpp"

namespace ECS {
    class ArchetypeStoreChangeNotifier final {
    public:
        using ArchetypeCallback = std::function<void(const Archetype *)>;
        using CallbackId = size_t;

        ArchetypeStoreChangeNotifier() {
            addEventSubscribers.reserve(100);
            updateEventSubscribers.reserve(100);
        }

        CallbackId subscribeToAdd(const ArchetypeCallback &callback) {
            CallbackId id = nextId++;
            addEventSubscribers.emplace_back(id, callback);
            return id;
        }

        CallbackId subscribeToUpdate(const ArchetypeCallback &callback) {
            CallbackId id = nextId++;
            updateEventSubscribers.emplace_back(id, callback);
            return id;
        }

        void unsubscribeFromAdd(CallbackId id) {
            auto it = std::find_if(addEventSubscribers.begin(), addEventSubscribers.end(),
                                   [id](const auto &pair) { return pair.id == id; });
            if (it != addEventSubscribers.end()) {
                addEventSubscribers.erase(it);
            }
        }

        void unsubscribeFromUpdate(CallbackId id) {
            auto it = std::find_if(updateEventSubscribers.begin(), updateEventSubscribers.end(),
                                   [id](const auto &pair) { return pair.id == id; });
            if (it != updateEventSubscribers.end()) {
                updateEventSubscribers.erase(it);
            }
        }

    private:
        struct CallbackEntry {
            CallbackId id;
            ArchetypeCallback callback;
        };

        std::vector<CallbackEntry> addEventSubscribers;
        std::vector<CallbackEntry> updateEventSubscribers;
        CallbackId nextId = 0;

        void notifyAdd(const Archetype *archetype) const {
            // std::thread([this, archetype] {
                for (const auto &subscriber: addEventSubscribers) {
                    subscriber.callback(archetype);
                }
            // }).detach();
        }

        void notifyUpdate(const Archetype *archetype) const {
            // std::thread([this, archetype] {
                for (const auto &subscriber: updateEventSubscribers) {
                    subscriber.callback(archetype);
                }
            // }).detach();
        }

        friend class ArchetypeStore;
    };
}
