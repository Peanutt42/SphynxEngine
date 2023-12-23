#include "pch.hpp"
#include "CommandHandler.hpp"
#include "Engine.hpp"
#include "Profiling/Profiling.hpp"

namespace Sphynx {
	void CommandHandler::Update() {
		SE_PROFILE_FUNCTION();

		while (!s_QueuedCommands.empty()) {
			const std::string& command = s_QueuedCommands.front();
			if (command == "foo")
				SE_INFO("bar");
			if (command == "exit")
				Engine::CloseNextFrame();

			s_QueuedCommands.pop();
		}
	}
}