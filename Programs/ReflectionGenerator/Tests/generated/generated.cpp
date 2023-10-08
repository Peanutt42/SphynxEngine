#include "Core/CoreInclude.hpp"
#include "../../../../Programs/ReflectionGenerator/ReflectionInfo.hpp"
#include "../../../../Engine/Scene/Scene.hpp"
#include "..\src\subfolder\subfolder-Test.hpp"
#include "..\src\TestComponents.hpp"

extern "C" {
	DLL_EXPORT std::vector<Sphynx::Scripting::ComponentReflectionInfo>* GetComponents() {
		static std::vector<Sphynx::Scripting::ComponentReflectionInfo> s_Components = {
			{
				"NoNamespaceComponent",
				""../Tests/src\\TestComponents.hpp"",
				{
					{	"int", "AInteger", offsetof(NoNamespaceComponent, AInteger) },
				},
				sizeof(NoNamespaceComponent),
				[](void* _this) { std::construct_at(reinterpret_cast<NoNamespaceComponent*>(_this)); },
				[](void* _this) { std::destroy_at(reinterpret_cast<NoNamespaceComponent*>(_this)); },
				[](void* _this, void* other) { std::construct_at(reinterpret_cast<NoNamespaceComponent*>(_this), *reinterpret_cast<NoNamespaceComponent*>(other)); }
			},
			{
				"ANamespace::ComponentWithNamespace",
				""../Tests/src\\TestComponents.hpp"",
				{
					{	"float", "AFloat", offsetof(ANamespace::ComponentWithNamespace, AFloat) },
				},
				sizeof(ANamespace::ComponentWithNamespace),
				[](void* _this) { std::construct_at(reinterpret_cast<ANamespace::ComponentWithNamespace*>(_this)); },
				[](void* _this) { std::destroy_at(reinterpret_cast<ANamespace::ComponentWithNamespace*>(_this)); },
				[](void* _this, void* other) { std::construct_at(reinterpret_cast<ANamespace::ComponentWithNamespace*>(_this), *reinterpret_cast<ANamespace::ComponentWithNamespace*>(other)); }
			},
			{
				"ANamespace::NestedComponent",
				""../Tests/src\\TestComponents.hpp"",
				{
					{	"", "Nested", offsetof(ANamespace::NestedComponent, Nested) },
				},
				sizeof(ANamespace::NestedComponent),
				[](void* _this) { std::construct_at(reinterpret_cast<ANamespace::NestedComponent*>(_this)); },
				[](void* _this) { std::destroy_at(reinterpret_cast<ANamespace::NestedComponent*>(_this)); },
				[](void* _this, void* other) { std::construct_at(reinterpret_cast<ANamespace::NestedComponent*>(_this), *reinterpret_cast<ANamespace::NestedComponent*>(other)); }
			},
			{
				"SubfolderComponent",
				""../Tests/src\\subfolder\\subfolder-Test.hpp"",
				{
					{	"int", "AnotherInt", offsetof(SubfolderComponent, AnotherInt) },
				},
				sizeof(SubfolderComponent),
				[](void* _this) { std::construct_at(reinterpret_cast<SubfolderComponent*>(_this)); },
				[](void* _this) { std::destroy_at(reinterpret_cast<SubfolderComponent*>(_this)); },
				[](void* _this, void* other) { std::construct_at(reinterpret_cast<SubfolderComponent*>(_this), *reinterpret_cast<SubfolderComponent*>(other)); }
			},
			{
				"AnotherSubfolderComponent",
				""../Tests/src\\subfolder\\subfolder-Test.hpp"",
				{
					{	"float", "AnotherFloat", offsetof(AnotherSubfolderComponent, AnotherFloat) },
				},
				sizeof(AnotherSubfolderComponent),
				[](void* _this) { std::construct_at(reinterpret_cast<AnotherSubfolderComponent*>(_this)); },
				[](void* _this) { std::destroy_at(reinterpret_cast<AnotherSubfolderComponent*>(_this)); },
				[](void* _this, void* other) { std::construct_at(reinterpret_cast<AnotherSubfolderComponent*>(_this), *reinterpret_cast<AnotherSubfolderComponent*>(other)); }
			},
		};
		return &s_Components;
	}
	DLL_EXPORT bool IsDebugConfiguration() {
#ifdef DEBUG
		return true;
#else
		return false;
#endif
	}
}
