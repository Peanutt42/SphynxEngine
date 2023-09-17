#pragma once

#include "Core/CoreInclude.hpp"
#include "Scene/Scene.hpp"
#include "Scene/DefaultComponents.hpp"
#include "TestComponent.hpp"

namespace TestProject {
	System()
	void TestSystem(Sphynx::Scene& scene) {
		scene.ForEach([&](Sphynx::ECS::EntityId entity) {
			scene.AddComponent<TestComponent>(entity, TestComponent{});
		});

		for (auto[entity, test] : scene.View<TestComponent>()) {
			SE_INFO(Sphynx::Logging::Game, "{} - {}", entity, test.AInt++);
		}
	}
}