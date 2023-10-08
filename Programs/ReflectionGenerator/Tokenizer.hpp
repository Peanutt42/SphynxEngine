#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

namespace Sphynx::ReflectionGenerator {
	enum class TokenType {
		COMPONENT_PROP,
		//SYSTEM_PROP,
		//CONFIG_PROP,
		STRUCT_KW,
		CLASS_KW,
		PUBLIC_KW,
		PROTECTED_KW,
		PRIVATE_KW,
		NAMESPACE_KW,
		REF,
		STAR,
		COLON,
		LEFT_PAREN,
		RIGHT_PAREN,
		HASHTAG,
		LEFT_BRACKET,
		RIGHT_BRACKET,
		SEMICOLON,
		EQUAL,
		IDENTIFIER,
		STRING_LITERAL, 
		NUMBER,
		GREATER,
		SMALLER,
		TRUE_KW, 
		FALSE_KW,
		END_OF_FILE,
		IGNORE_TYPE,
		ERROR_TYPE
	};
	constexpr const char* TokenTypeToString(TokenType type) {
		switch (type) {
		default:
		case TokenType::ERROR_TYPE: return "ERROR";
		case TokenType::COMPONENT_PROP: return "COMPONENT_PROP";
		//case TokenType::SYSTEM_PROP: return "SYSTEM_PROP";
		//case TokenType::CONFIG_PROP: return "CONFIG_PROP";
		case TokenType::STRUCT_KW: return "STRUCT_KW";
		case TokenType::CLASS_KW: return "CLASS_KW";
		case TokenType::PUBLIC_KW: return "PUBLIC_KW";
		case TokenType::PROTECTED_KW: return "PROTECTED_KW";
		case TokenType::PRIVATE_KW: return "PRIVATE_KW";
		case TokenType::NAMESPACE_KW: return "NAMESPACE_KW";
		case TokenType::REF: return "REF";
		case TokenType::STAR: return "STAR";
		case TokenType::COLON: return "COLON";
		case TokenType::LEFT_PAREN: return "LEFT_PAREN";
		case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
		case TokenType::HASHTAG: return "HASHTAG";
		case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
		case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
		case TokenType::SEMICOLON: return "SEMICOLON";
		case TokenType::EQUAL: return "EQUAL";
		case TokenType::IDENTIFIER: return "IDENTIFIER";
		case TokenType::STRING_LITERAL: return "STRING_LITERAL";
		case TokenType::NUMBER: return "NUMBER";
		case TokenType::GREATER: return "GREATER";
		case TokenType::SMALLER: return "SMALLER";
		case TokenType::TRUE_KW: return "TRUE_KW";
		case TokenType::FALSE_KW: return "FALSE_KW";
		case TokenType::END_OF_FILE: return "END_OF_FILE";
		}
	}

	struct Token {
		size_t Line = 0;
		size_t Column = 0;
		TokenType Type = TokenType::ERROR_TYPE;
		std::string Code;
	};

	class Tokenizer {
	public:
		Tokenizer(const std::string& sourceCode) : m_Code(sourceCode) {
			while (!AtEnd()) {
				m_Start = m_Cursor;
				char c = Advance();
				TokenType type = TokenType::ERROR_TYPE;
				std::string code = std::string(1, c);
				switch (c) {
					// Single character tokens
				case '(': type = TokenType::LEFT_PAREN; break;
				case ')': type = TokenType::RIGHT_PAREN; break;
				case '#': type = TokenType::HASHTAG; break;
				case '{': type = TokenType::LEFT_BRACKET; break;
				case '}': type = TokenType::RIGHT_BRACKET; break;
				case ';': type = TokenType::SEMICOLON; break;
				case '=': type = TokenType::EQUAL; break;
				case ':': type = TokenType::COLON; break;
				case '&': type = TokenType::REF; break;
				case '*': type = TokenType::STAR; break;
				case '>': type = TokenType::GREATER; break;
				case '<': type = TokenType::SMALLER; break;
				case '"': {
					while (Peek() != '"' && !AtEnd())
					{
						if (Peek() == '\n')
						{
							m_Line++;
							m_Column = 0;
						}
						Advance();
					}

					if (AtEnd())
					{
						m_ErrorMessage = "Unexpected string literal at " + std::to_string(m_Line) + ":" + std::to_string(m_Column);
						type = TokenType::ERROR_TYPE;
						break;
					}

					Advance();

					code = m_Code.substr(m_Start, m_Cursor - m_Start);
					type = TokenType::STRING_LITERAL;
				} break;

					// Whitespace
				case '/':
				{
					if (Match('/'))
					{
						while (Peek() != '\n' && !AtEnd())
							Advance();
						type = TokenType::IGNORE_TYPE;
						break;
					}
					else if (Match('*'))
					{
						while (!AtEnd() && Peek() != '*' && PeekNext() != '/')
						{
							c = Advance();
							if (c == '\n')
							{
								m_Column = 0;
								m_Line++;
							}
						}

						// Consume */
						if (!AtEnd()) Match('*');
						if (!AtEnd()) Match('/');
						type = TokenType::IGNORE_TYPE;
						break;
					}
					break;
				}
				case ' ':
				case '\r':
				case '\t':
					// Ignore whitespace
					type = TokenType::IGNORE_TYPE;
					break;
				case '\n':
					m_Column = 0;
					m_Line++;
					type = TokenType::IGNORE_TYPE;
					break;

				default: {
					if (IsAlpha(c)) {
						while (IsAlphaNumeric(Peek()) || Peek() == '_')
							Advance();

						code = m_Code.substr(m_Start, m_Cursor - m_Start);
						type = TokenType::IDENTIFIER;
						auto iter = s_Keywords.find(code);
						if (iter != s_Keywords.end())
							type = iter->second;
					}
					else if (IsDigit(c)) {
						while (IsDigit(Peek())) Advance();

						if (Peek() == '.' && (IsDigit(PeekNext()) || (PeekNext() == 'e' && IsDigit(PeekNextNext()))
							|| (PeekNext() == 'E' && IsDigit(PeekNextNext()))))
						{
							Advance();

							while (IsDigit(Peek()))
							{
								Advance();
							}

							if ((Peek() == 'e' || Peek() == 'E') && (IsDigit(PeekNext()) ||
								((PeekNext() == '-' && IsDigit(PeekNextNext())) || (PeekNext() == '+' && IsDigit(PeekNextNext())))))
							{
								Advance();
								while (IsDigit(Peek())) Advance();

								if ((Peek() == '-' || Peek() == '+') && IsDigit(PeekNext()))
								{
									Advance();
									while (IsDigit(Peek())) Advance();
								}

								if (Peek() == '.')
								{
									m_ErrorMessage = "Unexpected number literal at " + std::to_string(m_Line) + ":" + std::to_string(m_Column);
									type = TokenType::ERROR_TYPE;
									break;
								}
							}
						}

						if ((Peek() == 'e' || Peek() == 'E') && (IsDigit(PeekNext()) ||
							((PeekNext() == '-' && IsDigit(PeekNextNext())) || (PeekNext() == '+' && IsDigit(PeekNextNext())))))
						{
							Advance();
							while (IsDigit(Peek())) Advance();

							if ((Peek() == '-' || Peek() == '+') && IsDigit(PeekNext()))
							{
								Advance();
								while (IsDigit(Peek())) Advance();
							}

							if (Peek() == '.')
							{
								m_ErrorMessage = "Unexpected number literal at " + std::to_string(m_Line) + ":" + std::to_string(m_Column);
								type = TokenType::ERROR_TYPE;
								break;
							}
						}
						type = TokenType::NUMBER;
						code = m_Code.substr(m_Start, m_Cursor - m_Start);
					}
				} break;
				}

				//if (type == TokenType::ERROR_TYPE)
					//std::cout << "ERROR: " << code << '\n';
				if (type != TokenType::ERROR_TYPE && type != TokenType::IGNORE_TYPE)
					m_Tokens.emplace_back(m_Line, m_Column, type, code);
			}
			m_Tokens.emplace_back(m_Line, m_Column, TokenType::END_OF_FILE, "EOF");
		}

		bool IsSuccessful() const { return m_ErrorMessage.empty(); }
		const std::string& GetErrorMessage() const { return m_ErrorMessage; }

		const std::vector<Token>& GetTokens() const { return m_Tokens; }

	private:
		char Advance() {
			char c = m_Code[m_Cursor];
			m_Cursor++;
			m_Column++;
			return c;
		}

		char Peek() {
			if (AtEnd()) return '\0';
			return m_Code[m_Cursor];
		}

		char PeekNext() {
			if (AtEnd() || m_Cursor == m_Code.size() - 1) return '\0';
			return m_Code[m_Cursor + 1];
		}

		char PeekNextNext() {
			if (AtEnd() || m_Cursor == m_Code.size() - 1 || m_Cursor == m_Code.size() - 2) return '\0';
			return m_Code[m_Cursor + 1];
		}

		bool Match(char expected) {
			if (AtEnd()) return false;
			if (m_Code[m_Cursor] != expected) return false;

			m_Cursor++;
			m_Column++;
			return true;
		}

		constexpr bool IsDigit(char c) const { return c >= '0' && c <= '9'; }

		constexpr bool IsAlphaNumeric(char c) const { return IsAlpha(c) || IsDigit(c); }

		constexpr bool IsAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

		bool AtEnd() const { return m_Cursor == m_Code.size(); }

	private:
		const std::string& m_Code;
		std::string m_ErrorMessage;
		std::vector<Token> m_Tokens;
		size_t m_Cursor = 0;
		size_t m_Line = 0;
		size_t m_Column = 0;
		size_t m_Start = 0;

		const inline static std::unordered_map<std::string, TokenType> s_Keywords = {
			{ "Component",			TokenType::COMPONENT_PROP },
			//{ "System",				TokenType::SYSTEM_PROP },
			//{ "Config",				TokenType::CONFIG_PROP },
			{ "public",				TokenType::PUBLIC_KW },
			{ "protected",			TokenType::PROTECTED_KW },
			{ "private",			TokenType::PRIVATE_KW },
			{ "namespace",			TokenType::NAMESPACE_KW },
			{ "class",				TokenType::CLASS_KW },
			{ "struct",				TokenType::STRUCT_KW }
		};
	};
}