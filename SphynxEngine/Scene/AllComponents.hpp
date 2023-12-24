#pragma once

#include "DefaultComponents.hpp"
#include "Physics/PhysicsComponents.hpp"

namespace Sphynx::ECS {
    template<typename... Component>
    struct ComponentList {};

    using AllComponents = ComponentList<
        NameComponent,
        UUIDComponent,
        TransformComponent,
        Physics::RigidbodyComponent,
        Physics::BoxCollider,
        Physics::SphereCollider
    >;
}