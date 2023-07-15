#pragma once

#include "../../../Engine/src/Scripting/ScriptingModuleIncludes.hpp"


namespace Namespace1 {
	Component();
	struct AComponent {
		float AFloat = 0.f;
		int AInt = 0;
	};

	namespace Namespace2 {
		Component();
		struct AComponent2 {
			int AInt = 0;
		};

		namespace Namespace3::Namespace4::Namespace5 {
			Component();
			struct AComponent3 {
				float AFloat = 0.f;
			};
		}
	}



	Config();
	struct AConfig {
		const char* AText = "This is a example text!";
	};


	System();
	class ASystem {

	};
}