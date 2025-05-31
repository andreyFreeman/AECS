//
//  System.hpp
//  AECS
//
//  Created by ANDREY KLADOV on 25/05/2024.
//

#pragma once

#include <ECS/EntityManager.hpp>
#include <Updatable.hpp>

namespace ECS {

class System : public Updatable {
  protected:
    std::shared_ptr<EntityManager> entityManager;

  public:
    explicit System(const std::shared_ptr<EntityManager> &manager) : entityManager(manager) {}
};

} // namespace ECS
