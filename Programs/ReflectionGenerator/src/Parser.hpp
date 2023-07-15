#pragma once

#include <filesystem>

#include "Tokenizer.hpp"
#include "ReflectionInfo.hpp"

namespace Sphynx::Scripting {
	class Parser {
		struct NamespaceEntry {
			size_t Level = 0;
			std::string Name;
		};

	public:
		Parser(const std::vector<Token>& tokens, const std::string& filename, std::vector<ComponentReflectionInfo>& outComponents, std::vector<ConfigReflectionInfo>& outConfigs, std::vector<SystemReflectionInfo>& outSystems)
			: m_Tokens(tokens), m_Components(outComponents), m_Configs(outConfigs), m_Systems(outSystems), m_Filename(filename)
		{
			m_TokenIter = m_Tokens.cbegin();
			
			size_t level = 0;
			while (!Finished()) {
				if (Match(TokenType::COMPONENT_PROP)) {
					Expect(TokenType::LEFT_PAREN);
					// Arguments...
					Expect(TokenType::RIGHT_PAREN);
					Match(TokenType::SEMICOLON);

					ParseDefinition(m_Components.emplace_back());
				}
				else if (Match(TokenType::CONFIG_PROP)) {
					Expect(TokenType::LEFT_PAREN);
					// Arguments...
					Expect(TokenType::RIGHT_PAREN);
					Match(TokenType::SEMICOLON);

					ParseDefinition(m_Configs.emplace_back());
				}
				else if (Match(TokenType::SYSTEM_PROP)) {
					Expect(TokenType::LEFT_PAREN);
					// Arguments... (like ExecBefore(OtherSystem), ...)
					Expect(TokenType::RIGHT_PAREN);
					Match(TokenType::SEMICOLON);

					bool isPublic = true;
					SystemReflectionInfo& systemInfo = m_Systems.emplace_back();
					ParseStructClassDefinition(isPublic, systemInfo.FullName);
					size_t systemLevel = 1;
					while (!Finished()) {
						if (Match(TokenType::LEFT_BRACKET))
							systemLevel++;
						else if (Match(TokenType::RIGHT_BRACKET)) {
							systemLevel--;
							if (systemLevel <= 0) {
								Match(TokenType::SEMICOLON);
								break;
							}
						}
						else if (Match(TokenType::PUBLIC_KW)) {
							isPublic = true;
							Match(TokenType::COLON);
						}
						else if (Match(TokenType::PRIVATE_KW) || Match(TokenType::PROTECTED_KW)) {
							isPublic = false;
							Match(TokenType::COLON);
						}
						//else if (Match(TokenType::HIDE_PROPERTY_PROP)) {
						//	hideNextProperty = true;
						//	Match(TokenType::SEMICOLON);
						//}
						else if (m_TokenIter->Type == TokenType::IDENTIFIER) {
							//if (hideNextProperty) {
							//	hideNextProperty = false;
							//	while (!Finished() && m_TokenIter->Type != TokenType::SEMICOLON) {
							//		Advance();
							//		if (m_TokenIter->Type == TokenType::LEFT_BRACKET) {
							//			while (!Finished() && m_TokenIter->Type != TokenType::RIGHT_BRACKET)
							//				Advance();
							//			Match(TokenType::SEMICOLON);
							//			Advance();
							//			break;
							//		}
							//	}
							//}
							//else if (isPublic) {
							//	std::vector<std::vector<Token>::const_iterator> variableIters;
							//	for (; m_TokenIter != m_Tokens.cend() && IsTokenInsideOfType(*m_TokenIter);
							//		++m_TokenIter)
							//	{
							//		if (m_TokenIter->Code == "static" ||
							//			m_TokenIter->Code == "constexpr")
							//			break;
							//		if (m_TokenIter->Code == "inline")
							//			continue;
							//		variableIters.push_back(m_TokenIter);
							//	}
							//
							//	if (variableIters.size() > 0 && m_TokenIter->Type != TokenType::LEFT_PAREN && variableIters.back()->Type == TokenType::IDENTIFIER) {
							//		VariableReflectionInfo& variable = component.Variables.emplace_back();
							//		for (size_t i = 0; i < variableIters.size() - 1; i++) {
							//			variable.Type += variableIters[i]->Code;
							//			if (IsVariableAttribute(variableIters[i]->Code))
							//				variable.Type += ' ';
							//		}
							//		variable.Name = variableIters.back()->Code;
							//	}
							//	while (!Finished() && m_TokenIter->Type != TokenType::SEMICOLON) {
							//		Advance();
							//		if (m_TokenIter->Type == TokenType::LEFT_BRACKET) {
							//			while (!Finished() && m_TokenIter->Type != TokenType::RIGHT_BRACKET)
							//				Advance();
							//			Match(TokenType::SEMICOLON);
							//			Advance();
							//			break;
							//		}
							//	}
							//}
							//else
								Advance();
						}
						else if (m_TokenIter->Type == TokenType::CLASS_KW || m_TokenIter->Type == TokenType::STRUCT_KW) {
							size_t beginSystemLevel = systemLevel;
							while (!Finished() && m_TokenIter->Type != TokenType::LEFT_BRACKET)
								Advance();
							Advance();

							while (!Finished()) {
								if (Match(TokenType::LEFT_BRACKET))
									systemLevel++;
								else if (Match(TokenType::RIGHT_BRACKET)) {
									systemLevel--;
									if (systemLevel <= beginSystemLevel) {
										Match(TokenType::SEMICOLON);
										break;
									}
								}
								Advance();
							}
						}
						else
							Advance();
					}
				}
				else if (Match(TokenType::NAMESPACE_KW)) {
					if (m_TokenIter->Type != TokenType::IDENTIFIER) {
						m_ErrorMessage = "Expected a identifier after 'namespace'";
						return;
					}
					
					std::string newNamespace;
					while (!Finished()) {
						newNamespace += m_TokenIter->Code;
						m_TokenIter++;
						Match(TokenType::COLON);
						Match(TokenType::COLON);
						if (m_TokenIter->Type == TokenType::IDENTIFIER)
							newNamespace += "::";
						else
							break;
					}
					m_CurrentNamespaceStack.emplace_back() = NamespaceEntry{ level, newNamespace };
				}
				else if (Match(TokenType::LEFT_BRACKET))
					level++;
				else if (Match(TokenType::RIGHT_BRACKET)) {
					level--;
					if (m_CurrentNamespaceStack.size() > 0 && m_CurrentNamespaceStack.back().Level == level)
						m_CurrentNamespaceStack.pop_back();
				}
				else
					Advance();
			}
		}

		const std::vector<ComponentReflectionInfo>& GetComponents() const { return m_Components; }

		bool IsSuccessful() const { return m_ErrorMessage.empty(); }
		std::string GetErrorMessagee() const { return m_ErrorMessage + " at " + m_Filename + " in line " + std::to_string(m_TokenIter->Line) + ":" + std::to_string(m_TokenIter->Column); }

	private:
		const inline static Token s_InvalidToken {};
		
		void ParseDefinition(ComponentReflectionInfo& component) {
			bool isPublic = true;
			ParseStructClassDefinition(isPublic, component.FullName);

			bool hideNextProperty = false;
			size_t componentLevel = 1;
			while (!Finished()) {
				if (Match(TokenType::LEFT_BRACKET))
					componentLevel++;
				else if (Match(TokenType::RIGHT_BRACKET)) {
					componentLevel--;
					if (componentLevel <= 0) {
						Match(TokenType::SEMICOLON);
						break;
					}
				}
				else if (Match(TokenType::PUBLIC_KW)) {
					isPublic = true;
					Match(TokenType::COLON);
				}
				else if (Match(TokenType::PRIVATE_KW) || Match(TokenType::PROTECTED_KW)) {
					isPublic = false;
					Match(TokenType::COLON);
				}
				else if (Match(TokenType::HIDE_PROPERTY_PROP)) {
					hideNextProperty = true;
					Match(TokenType::SEMICOLON);
				}
				else if (m_TokenIter->Type == TokenType::IDENTIFIER) {
					if (hideNextProperty) {
						hideNextProperty = false;
						while (!Finished() && m_TokenIter->Type != TokenType::SEMICOLON) {
							Advance();
							if (m_TokenIter->Type == TokenType::LEFT_BRACKET) {
								while (!Finished() && m_TokenIter->Type != TokenType::RIGHT_BRACKET)
									Advance();
								Match(TokenType::SEMICOLON);
								Advance();
								break;
							}
						}
					}
					else if (isPublic) {
						std::vector<std::vector<Token>::const_iterator> variableIters;
						for (; m_TokenIter != m_Tokens.cend() && IsTokenInsideOfType(*m_TokenIter);
							++m_TokenIter)
						{
							if (m_TokenIter->Code == "static" ||
								m_TokenIter->Code == "constexpr")
								break;
							if (m_TokenIter->Code == "inline")
								continue;
							variableIters.push_back(m_TokenIter);
						}

						if (variableIters.size() > 0 && m_TokenIter->Type != TokenType::LEFT_PAREN && variableIters.back()->Type == TokenType::IDENTIFIER) {
							VariableReflectionInfo& variable = component.Variables.emplace_back();
							for (size_t i = 0; i < variableIters.size() - 1; i++) {
								variable.Type += variableIters[i]->Code;
								if (IsVariableAttribute(variableIters[i]->Code))
									variable.Type += ' ';
							}
							variable.Name = variableIters.back()->Code;
						}
						while (!Finished() && m_TokenIter->Type != TokenType::SEMICOLON) {
							Advance();
							if (m_TokenIter->Type == TokenType::LEFT_BRACKET) {
								while (!Finished() && m_TokenIter->Type != TokenType::RIGHT_BRACKET)
									Advance();
								Match(TokenType::SEMICOLON);
								Advance();
								break;
							}
						}
					}
					else
						Advance();
				}
				else if (m_TokenIter->Type == TokenType::CLASS_KW || m_TokenIter->Type == TokenType::STRUCT_KW) {
					size_t beginComponentLevel = componentLevel;
					while (!Finished() && m_TokenIter->Type != TokenType::LEFT_BRACKET)
						Advance();
					Advance();

					while (!Finished()) {
						if (Match(TokenType::LEFT_BRACKET))
							componentLevel++;
						else if (Match(TokenType::RIGHT_BRACKET)) {
							componentLevel--;
							if (componentLevel <= beginComponentLevel) {
								Match(TokenType::SEMICOLON);
								break;
							}
						}
						Advance();
					}
				}
				else
					Advance();
			}
		}

		const Token& Expect(TokenType type) {
			if (m_TokenIter->Type != type)
			{
				m_ErrorMessage = "Expected " + std::string(TokenTypeToString(type)) + " but instead got " + std::string(TokenTypeToString(m_TokenIter->Type));
				
				return s_InvalidToken;
			}

			const auto tokenToReturn = m_TokenIter;
			Advance();
			return *tokenToReturn;
		}
		bool Match(TokenType type) {
			if (m_TokenIter->Type == type)
			{
				Advance();
				return true;
			}

			return false;
		}
		bool IsVariableAttribute(const std::string& str) {
			for (const std::string& keyword : m_VariableAttributes) {
				if (str == keyword)
					return true;
			}
			return false;
		}
		bool IsTokenInsideOfType(const Token& token) {
			return token.Type != TokenType::SEMICOLON &&
				token.Type != TokenType::EQUAL &&
				token.Type != TokenType::LEFT_BRACKET &&
				token.Type != TokenType::LEFT_PAREN;
		}
		bool Finished() const {
			return m_TokenIter == m_Tokens.cend() || m_TokenIter->Type == TokenType::END_OF_FILE;
		}
		void Advance() {
			++m_TokenIter;
		}

		void ParseStructClassDefinition(bool& outIsPublic, std::string& outFullname) {
			outIsPublic = true;
			if (Match(TokenType::CLASS_KW))
				outIsPublic = false;
			else
				Expect(TokenType::STRUCT_KW);

			// only take the last identifier as the class name ex: class PE_API EngineClass {}; -> "EngineClass"
			Token classType = s_InvalidToken;
			while (!Finished() && (m_TokenIter->Type != TokenType::COLON && m_TokenIter->Type != TokenType::LEFT_BRACKET)) {
				classType = *m_TokenIter;
				Advance();
			}

			if (Match(TokenType::COLON)) {
				while (!Finished() && !Match(TokenType::LEFT_BRACKET))
					Advance();
			}
			else
				Expect(TokenType::LEFT_BRACKET);

			std::string currentNamespace;
			for (const auto& [namespaceLevel, name] : m_CurrentNamespaceStack)
				currentNamespace += name + "::";
			outFullname = currentNamespace + classType.Code;
		}

	private:
		std::vector<Token>::const_iterator m_TokenIter;
		std::vector<NamespaceEntry> m_CurrentNamespaceStack;
		const std::vector<Token>& m_Tokens;
		std::vector<ComponentReflectionInfo>& m_Components;
		std::vector<ConfigReflectionInfo>& m_Configs;
		std::vector<SystemReflectionInfo>& m_Systems;
		std::vector<std::string> m_VariableAttributes = {
			"const",
			"mutable",
			"volatile"
		};
		std::string m_ErrorMessage;
		std::string m_Filename;
	};
}
