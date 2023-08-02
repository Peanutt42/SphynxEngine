#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <shaderc/shaderc.hpp>

namespace Sphynx::Editor {
    void CompileShader(const std::string& shaderCode, shaderc_shader_kind type, const std::string& shaderName, std::vector<uint32_t>& outSpirvCode) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
        options.SetGenerateDebugInfo();

        // Preprocessing
        shaderc::PreprocessedSourceCompilationResult result = compiler.PreprocessGlsl(shaderCode, type, shaderName.c_str(), options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            SE_FATAL(Logging::Rendering, "Precompilation error: {}", result.GetErrorMessage());
            return;
        }
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
            SE_FATAL(Logging::Rendering, "Failed to compile {} shader: {}", shaderTypeStr, module.GetErrorMessage());
            return;
        }
        outSpirvCode.assign(module.cbegin(), module.cend());
        if (outSpirvCode.empty())
            SE_FATAL(Logging::Rendering, "Failed to compile shader");
    }

    bool IsCachedShaderUsable(const std::filesystem::path& shaderFilepath, const std::filesystem::path& cachedShaderFilepath) {
        if (!std::filesystem::exists(cachedShaderFilepath))
            return false;

        if (std::filesystem::last_write_time(shaderFilepath) > std::filesystem::last_write_time(cachedShaderFilepath))
            return false;

        return true;
    }

    bool SeperateVertexAndFragmentShaderCode(const std::string& fileContents, std::string& vertexCode, std::string& fragmentCode, std::string& errorMsg) {
        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = fileContents.find(typeToken, 0); //Start of shader type declaration line
        while (pos != std::string::npos) {
            size_t eol = fileContents.find_first_of('\n', pos); //End of shader type declaration line
            if (eol == std::string::npos) {
                errorMsg = "Syntax error";
                return false;
            }
            size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
            std::string type = fileContents.substr(begin, eol - begin);

            size_t nextLinePos = fileContents.find_first_not_of('\n', eol); //Start of shader code after shader type declaration line
            if (nextLinePos == std::string::npos) {
                errorMsg = "Syntax error";
                return false;
            }
            pos = fileContents.find(typeToken, nextLinePos); //Start of next shader type declaration line

            if (type == "vertex") vertexCode = (pos == std::string::npos) ? fileContents.substr(nextLinePos) : fileContents.substr(nextLinePos, pos - nextLinePos);
            else if (type == "fragment") fragmentCode = (pos == std::string::npos) ? fileContents.substr(nextLinePos) : fileContents.substr(nextLinePos, pos - nextLinePos);
            else {
                errorMsg = "Unkown shader type " + type;
                return false;
            }
        }
        return true;
    }

	void ShaderImporter::Import(const std::filesystem::path& filepath, std::vector<uint32_t>& outVertexCode, std::vector<uint32_t>& outFragmentCode) {
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
            std::string errorMsg;
            if (!SeperateVertexAndFragmentShaderCode(shaderCode, vertexCode, fragmentCode, errorMsg)) {
                SE_ERR(Logging::Rendering, "Failed to seperate shader code: {}", errorMsg);
                return;
            }
                
            CompileShader(vertexCode, shaderc_vertex_shader, shaderName, outVertexCode);
            CompileShader(fragmentCode, shaderc_fragment_shader, shaderName, outFragmentCode);

            FileStreamWriter cachedShaderFile(cachedShaderFilepath);
            cachedShaderFile.WriteArray(outVertexCode);
            cachedShaderFile.WriteArray(outFragmentCode);
        }
	}
}