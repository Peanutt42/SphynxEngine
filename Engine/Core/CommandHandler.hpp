#pragma once

#include "pch.hpp"
#include <queue>

namespace Sphynx {
	class CommandHandler {
	public:
		static void Update();

		static void QueueCommand(const std::string& command) {
			s_QueuedCommands.push(command);
		}

	private:
		inline static std::queue<std::string> s_QueuedCommands;
	};
}