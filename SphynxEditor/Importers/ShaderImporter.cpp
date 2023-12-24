#include "pch.hpp"
#include "ShaderImporter.hpp"
#include "Core/Time.hpp"
#include "Serialization/FileStream.hpp"

#include <shaderc/shaderc.hpp>

namespace Sphynx::Editor {
    Result<bool, std::string> CompileShader(const std::string& shaderCode, shaderc_shader_kind type, const std::string& shaderName, std::vector<uint32>& outSpirvCode) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
        options.SetGenerateDebugInfo();

        // Preprocessing
        shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(shaderCode, type, shaderName.c_str(), options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
			return Error<bool>("Precompilation error: {}", result.GetErrorMessage());

        std::string preprocessedShaderCode = { result.cbegin(), result.cend() };

        // Compiling
        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(preprocessedShaderCode, type, shaderName.c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            const char* shaderTypeStr = "";
            switch (type) {
            default: break;
            case shaderc_vertex_shader: shaderTypeStr = "vertex"; break;
            case shaderc_fragment_shader: shaderTypeStr = "fragment"; break;
            }
            return Error<bool>("Failed to compile {} shader: {}", shaderTypeStr, module.GetErrorMessage());
        }
        outSpirvCode.assign(module.cbegin(), module.cend());
        if (outSpirvCode.empty())
            return Error<bool>("Failed to compile shader");

        return true;
    }

    bool IsCachedShaderUsable(const std::filesystem::path& shaderFilepath, const std::filesystem::path& cachedShaderFilepath) {
        if (!std::filesystem::exists(cachedShaderFilepath))
            return false;

        if (std::filesystem::last_write_time(shaderFilepath) > std::filesystem::last_write_time(cachedShaderFilepath))
            return false;

        return true;
    }

    Result<bool, std::string> SeperateVertexAndFragmentShaderCode(const std::string& fileContents, std::string& vertexCode, std::string& fragmentCode) {
        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = fileContents.find(typeToken, 0); //Start of shader type declaration line
        while (pos != std::string::npos) {
            size_t eol = fileContents.find_first_of('\n', pos); //End of shader type declaration line
			if (eol == std::string::npos)
				return Error<bool>("Syntax error");

            size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
            std::string type = fileContents.substr(begin, eol - begin);

            size_t nextLinePos = fileContents.find_first_not_of('\n', eol); //Start of shader code after shader type declaration line
            if (nextLinePos == std::string::npos)
				return Error<bool>("Syntax error");

            pos = fileContents.find(typeToken, nextLinePos); //Start of next shader type declaration line

            if (type == "vertex") vertexCode = (pos == std::string::npos) ? fileContents.substr(nextLinePos) : fileContents.substr(nextLinePos, pos - nextLinePos);
            else if (type == "fragment") fragmentCode = (pos == std::string::npos) ? fileContents.substr(nextLinePos) : fileContents.substr(nextLinePos, pos - nextLinePos);
            else
				return Error<bool>("Unknown shader type: {}", type);
        }

        return true;
    }

	Result<bool, std::string> ShaderImporter::Import(const std::filesystem::path& filepath, std::vector<uint32>& outVertexCode, std::vector<uint32>& outFragmentCode) {
        std::filesystem::path cachedShaderFilepath = filepath;
        cachedShaderFilepath.replace_extension(".cached_shader");

        if (IsCachedShaderUsable(filepath, cachedShaderFilepath)) {
            FileStreamReader shaderFile(cachedShaderFilepath);
            shaderFile.ReadArray(outVertexCode);
            shaderFile.ReadArray(outFragmentCode);
        }
        else {
            ScopedTimer t("Compiling shader " + filepath.string());


            std::string shaderCode;
            FileStreamReader::ReadTextFile(filepath, shaderCode);
            std::string shaderName = filepath.filename().string();

            std::string vertexCode, fragmentCode;
            auto seperateResult = SeperateVertexAndFragmentShaderCode(shaderCode, vertexCode, fragmentCode);
            if (seperateResult.is_error())
				return seperateResult;
                
            CompileShader(vertexCode, shaderc_vertex_shader, shaderName, outVertexCode);
            CompileShader(fragmentCode, shaderc_fragment_shader, shaderName, outFragmentCode);

            FileStreamWriter cachedShaderFile(cachedShaderFilepath);
            cachedShaderFile.WriteArray(outVertexCode);
            cachedShaderFile.WriteArray(outFragmentCode);
        }

		return true;
	}
}