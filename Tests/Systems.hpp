//
// Created by ANDREY KLADOV on 24/05/2025.
//

#pragma once

#include <ECS/System/SystemComponentView.hpp>
#include <utility>
#include <vector>

struct PositionComponent {
    float x{0.0F};
    float y{0.0F};
};

struct VelocityComponent {
    float x{1.0F};
    float y{1.0F};
};

struct SpriteComponent {
    char character{' '};
};

enum class PlayerType { NPC, Monster, Hero };

struct PlayerComponent {
    ecs::benchmarks::base::random_xoshiro128 rng{};
    PlayerType type{PlayerType::NPC};
};

enum class StatusEffect { Spawn, Dead, Alive };

struct HealthComponent {
    int32_t hp{0};
    int32_t maxhp{0};
    StatusEffect status{StatusEffect::Spawn};
};

struct DamageComponent {
    int32_t atk{0};
    int32_t def{0};
};

struct DataComponent {
    inline static constexpr uint32_t DefaultSeed = 340383L;

    int thingy{0};
    double dingy{0.0};
    bool mingy{false};

    uint32_t seed{DefaultSeed};
    ecs::benchmarks::base::random_xoshiro128 rng;
    uint32_t numgy;

    DataComponent() : rng(seed), numgy(rng()) {
    }
};

class DamageSystem final : public ECS::SystemComponentView<HealthComponent, DamageComponent> {
    using SystemComponentView::SystemComponentView;

public:
    bool update(float dt) override {
        componentView.forEach([&](auto &health, auto &damage) {
            const int totalDamage = damage.atk - damage.def;
            if (health.hp > 0 && totalDamage > 0) {
                health.hp = std::max(health.hp - totalDamage, 0);
            }
        });
        return true;
    }
};

class DataSystem final : public ECS::SystemComponentView<DataComponent> {
    using SystemComponentView::SystemComponentView;

public:
    bool update(float dt) override {
        componentView.forEach([&](auto &data) {
            data.thingy = (data.thingy + 1) % 1'000'000;
            data.dingy += 0.0001 * dt;
            data.mingy = !data.mingy;
            data.numgy = data.rng();
        });
        return true;
    }
};

class HealthSystem final : public ECS::SystemComponentView<HealthComponent> {
    using SystemComponentView::SystemComponentView;

public:
    bool update(float dt) override {
        componentView.forEach([&](auto &health) {
            if (health.hp <= 0 && health.status != StatusEffect::Dead) {
                health.hp = 0;
                health.status = StatusEffect::Dead;
            } else if (health.status == StatusEffect::Dead && health.hp == 0) {
                health.hp = health.maxhp;
                health.status = StatusEffect::Spawn;
            } else if (health.hp >= health.maxhp && health.status != StatusEffect::Alive) {
                health.hp = health.maxhp;
                health.status = StatusEffect::Alive;
            } else {
                health.status = StatusEffect::Alive;
            }
        });
        return true;
    }
};

class MoreComplexSystem final : public ECS::SystemComponentView<const PositionComponent, VelocityComponent, DataComponent> {
    using SystemComponentView::SystemComponentView;

public:
    bool update(float dt) override {
        componentView.forEach([&](auto &position, auto &direction, auto &data) {
            if ((data.thingy % 10) == 0) {
                if (position.x > position.y) {
                    direction.x = data.rng.range(3, 19) - 10.0F;
                    direction.y = data.rng.range(0, 5);
                } else {
                    direction.x = data.rng.range(0, 5);
                    direction.y = data.rng.range(3, 19) - 10.0F;
                }
            }
        });
        return true;
    }
};

class MovementSystem final : public ECS::SystemComponentView<PositionComponent, const VelocityComponent> {
    using SystemComponentView::SystemComponentView;

public:
    bool update(float dt) override {
        componentView.forEach([&](auto &position, auto &direction) {
            position.x += direction.x * dt;
            position.y += direction.y * dt;
        });
        return true;
    }
};

class RenderSystem final : public ECS::SystemComponentView<const PositionComponent, const SpriteComponent> {
    using SystemComponentView::SystemComponentView;

public:
    bool update(float dt) override {
        componentView.forEach([&](auto &position, auto &sprite) {
        });
        return true;
    }
};

class SpriteSystem final : public ECS::SystemComponentView<SpriteComponent, const PlayerComponent, const HealthComponent> {
    using SystemComponentView::SystemComponentView;

public:
    inline static constexpr char PlayerSprite = '@';
    inline static constexpr char MonsterSprite = 'k';
    inline static constexpr char NPCSprite = 'h';
    inline static constexpr char GraveSprite = '|';
    inline static constexpr char SpawnSprite = '_';
    inline static constexpr char NoneSprite = ' ';

    bool update(float dt) override {
        componentView.forEach([&](auto &sprite, auto &player, auto &health) {
            sprite.character = [&]() {
                switch (health.status) {
                    case StatusEffect::Alive:
                        switch (player.type) {
                            case PlayerType::Hero:
                                return PlayerSprite;
                            case PlayerType::Monster:
                                return MonsterSprite;
                            case PlayerType::NPC:
                                return NPCSprite;
                        }
                        break;
                    case StatusEffect::Dead:
                        return GraveSprite;
                    case StatusEffect::Spawn:
                        return SpawnSprite;
                }
                return NoneSprite;
            }();
        });
        return true;
    }
};

class FrameBuffer {
public:
    FrameBuffer(uint32_t w, uint32_t h)
        : m_width{w}, m_height{h}, m_buffer(static_cast<size_t>(m_width) * static_cast<size_t>(m_height)) {
    }

    [[nodiscard]] auto width() const noexcept { return m_width; }
    [[nodiscard]] auto height() const noexcept { return m_height; }

    void draw(int x, int y, char c) {
        if (y >= 0 && std::cmp_less(y, m_height)) {
            if (x >= 0 && std::cmp_less(x, m_width)) {
                m_buffer[static_cast<size_t>(x) + static_cast<size_t>(y) * m_width] = c;
            }
        }
    }

private:
    uint32_t m_width;
    uint32_t m_height;
    std::vector<char> m_buffer;
};
