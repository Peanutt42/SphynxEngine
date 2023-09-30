#pragma once

#include "Core/CoreInclude.hpp"
#include "Scene/Scene.hpp"
#include "Scene/DefaultComponents.hpp"
#include "TestComponent.hpp"

namespace TestProject {
	System()
	void TestSystem(Sphynx::Scene& scene) {
		for (Sphynx::ECS::EntityId entity : scene) {
			scene.AddComponent<TestProject::TestComponent>(entity);
		}

		for (auto [entity, test] : scene.View<TestProject::TestComponent>()) {
			SE_INFO(Sphynx::Logging::Game, "{}: {}", entity, test.AInt++);
		}
	}
}