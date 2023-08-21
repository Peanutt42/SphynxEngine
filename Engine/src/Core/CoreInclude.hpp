#pragma once

#include "std.hpp"

#include <glm/glm.hpp>

#include "Time.hpp"
#include "Version.hpp"
#include "EngineApi.hpp"
#include "IntTypes.hpp"
#include "UUID.hpp"

#include "Logging/Logging.hpp"
#include "Platform/Platform.hpp"
#include "Profiling/Profiling.hpp"
#include "Input/Input.hpp"
#include "Memory/BufferView.hpp"
#include "Math/Transform.hpp"
#include "Serialization/StreamWriter.hpp"
#include "Serialization/StreamReader.hpp"
#include "Serialization/MemoryStream.hpp"
#include "Serialization/FileStream.hpp"

#ifdef SE_GAME_MODULE
#include "Scripting/ScriptingModuleIncludes.hpp"
#endif