#include "Core/CoreInclude.hpp"
#include "../../Programs/ReflectionGenerator/src/ReflectionInfo.hpp"
#include "../../Engine/src/Scene/Scene.hpp"
#include "..\TestProject\TestComponent.hpp"
#include "..\TestProject\TestConfig.hpp"
#include "..\TestProject\TestSystem.hpp"

extern "C" {
	__declspec(dllexport) std::vector<Sphynx::Scripting::ComponentReflectionInfo>* GetComponents() {
		static std::vector<Sphynx::Scripting::ComponentReflectionInfo> s_Components = {
			{
				0,
				"TestProject::TestComponent",
				{
					{
						"int", "AInt", offsetof(TestProject::TestComponent, AInt), sizeof(decltype(TestProject::TestComponent::AInt)),
						[](void* ptr, void* other) { std::construct_at(reinterpret_cast<decltype(TestProject::TestComponent::AInt)*>(ptr), *reinterpret_cast<decltype(TestProject::TestComponent::AInt)*>(other)); }
					},
				},
				sizeof(TestProject::TestComponent),
				[](void* ptr) { std::construct_at(reinterpret_cast<TestProject::TestComponent*>(ptr)); },
				[](void* ptr) { std::destroy_at(reinterpret_cast<TestProject::TestComponent*>(ptr)); },
				[](void* ptr, void* other) { std::construct_at(reinterpret_cast<TestProject::TestComponent*>(ptr), *reinterpret_cast<TestProject::TestComponent*>(other)); }
			},
		};
		return &s_Components;
	}
	__declspec(dllexport) std::vector<Sphynx::Scripting::ConfigReflectionInfo>* GetConfigs() {
		static std::vector<Sphynx::Scripting::ConfigReflectionInfo> s_Configs = {
			{
				0,
				"TestProject::TestConfig",
				{
					{
						"std::string", "ATestName", offsetof(TestProject::TestConfig, ATestName), sizeof(decltype(TestProject::TestConfig::ATestName)),
						[](void* ptr, void* other) { std::construct_at(reinterpret_cast<decltype(TestProject::TestConfig::ATestName)*>(ptr), *reinterpret_cast<decltype(TestProject::TestConfig::ATestName)*>(other)); }
					},
				},
				sizeof(TestProject::TestConfig),
				[](void* ptr) { std::construct_at(reinterpret_cast<TestProject::TestConfig*>(ptr)); },
				[](void* ptr) { std::destroy_at(reinterpret_cast<TestProject::TestConfig*>(ptr)); },
				[](void* ptr, void* other) { std::construct_at(reinterpret_cast<TestProject::TestConfig*>(ptr), *reinterpret_cast<TestProject::TestConfig*>(other)); }
			},
		};
		return &s_Configs;
	}
	__declspec(dllexport) std::vector<Sphynx::Scripting::SystemReflectionInfo>* GetSystems() {
		static std::vector<Sphynx::Scripting::SystemReflectionInfo> s_Systems = {
			{
				0,
				"TestProject::TestSystem",
				[](void* scene) { TestProject::TestSystem::Update(*(Sphynx::Scene*)scene); }
			},
		};
		return &s_Systems;
	}
}
