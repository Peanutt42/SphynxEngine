#include "Core/CoreInclude.hpp"
#include "../../Programs/ReflectionGenerator/src/ReflectionInfo.hpp"
#include "C:\dev\cpp\SphynxEngine\TestProject\Source\TestProject\TestComponent.hpp"

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
		};
		return &s_Configs;
	}
	__declspec(dllexport) std::vector<Sphynx::Scripting::SystemReflectionInfo>* GetSystems() {
		static std::vector<Sphynx::Scripting::SystemReflectionInfo> s_Systems = {
		};
		return &s_Systems;
	}
}
