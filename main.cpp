#include <cassert>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <any>
#include <string_view>

using namespace std;

enum TokenType { 
	NUMBER, IDENT, QUOTE, CHAR, SEMICOLON,
	PLUS, MINUS, ASTERISK, SLASH, EQUALS,
	PRINT, RETURN, FUNC,
	EOF_, 
	// OPENING_COMMENT, CLOSING_COMMENT
};

// enum TokenResult { VALID, NEW_LINE, SYNTAX_ERROR };

std::ostream& operator<<(std::ostream& out, const TokenType value){
	switch (value) {
		case NUMBER: return out << "NUMBER";
		case IDENT: return out << "IDENT";
		case QUOTE: return out << "QUOTE";
		case CHAR: return out << "CHAR";
		case SEMICOLON: return out << "SEMICOLON";
		case PLUS: return out << "PLUS";
		case MINUS: return out << "MINUS";
		case ASTERISK: return out << "ASTERISK";
		case SLASH: return out << "SLASH";
		case EQUALS: return out << "EQUALS";
		case PRINT: return out << "PRINT";
		case RETURN: return out << "RETURN";
		case FUNC: return out << "FUNC";
		case EOF_: return out << "EOF";
		// case OPENING_COMMENT: return out << "OPENING_COMMENT";
		// case CLOSING_COMMENT: return out << "CLOSING_COMMENT";
	}
	assert(false);
	return out;
}

struct Token {
	TokenType Type;
	any Value;
	int Line;
	int Col;

	friend std::ostream& operator<< (std::ostream& stream, const Token& tok) {
		stream << tok.Type
			<< ", line: " << tok.Line
			<< ", col: " << tok.Col
			<< ", val: ";
		if (const auto* value = any_cast<int>(&tok.Value)) {
			stream << *value;
		} else if (const auto* value = any_cast<string>(&tok.Value)) {
			stream << *value;
		} else if (!tok.Value.has_value()) {
			stream << "<empty>";
		} else {
			stream << "<unsupported value>";
		}
		return stream;
	}
};

// bool isInteger(std::string_view str) {
//     int value;
//     auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    
//     return ec == std::errc{} && ptr == str.data() + str.size();
// }

bool tryParseInt(std::string_view str, int& out) {
	auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
	return ec == std::errc{} && ptr == str.data() + str.size();
}

class Lexer {
	private:
		int intResult;
		string input;
		regex del{R"([^\s;]+|\s|;)"}; // split on space and ;, add them back when split
		regex space{R"( )"}; // split on just space
		regex newLine{R"(\n|\r\n)"}; // LF | CRLF
		regex tab{R"(\t)"};
		vector<string> split;
		vector<Token> tokens = {};
		int lc = 1;
		int col = 1;

		bool parseToken(string token, sregex_token_iterator* it, sregex_token_iterator* end) { //keywords must be before identifiers
			Token t = { };
			if (tryParseInt(token, intResult)) {
				t.Type = NUMBER;
				t.Value = intResult;
			} else if (token == "print") {
				t.Type = PRINT;
			} else if (token == "return") {
				t.Type = RETURN;
			} else if (token == "func") {
				t.Type = FUNC;
			} else if (token == "+") {
				t.Type = PLUS;
			} else if (token == "-") {
				t.Type = MINUS;
			} else if (token == "/") {
				t.Type = SLASH;
			} else if (token == "*") {
				t.Type = ASTERISK;
			} else if (token == "=") {
				t.Type = EQUALS;
			} else if (token == ";") {
				t.Type = SEMICOLON;
			} else if (token == "/*") { //if comment, iterate until closing comment block, if we do // comments then iterate till new line
				//if we do quotes then iterate till next quote
				bool endMet = false;
				while (true) {
					++(*it);
					endMet = (*it)->str() == "*/";
					if (endMet || *it == *end) break;
				}
				if (!endMet) return false;
				return true;
			} else if (regex_match(token, newLine)) { //return on new line
				lc++;
				col = 1;
				return true;
			} else if (regex_match(token, space)) { //return on space
				col += token.length();
				return true;
			} else if (tokens.size() == 0 || tokens.back().Type != IDENT) {
				t.Type = IDENT;
			} else { //return on error
				return false;
			}

			//otherwise tally up the column and add the token
			t.Line = lc;
			t.Col = col;
			if (t.Type != NUMBER) {
				t.Value = token;
			}
			tokens.push_back(t);
			col += token.length();
			return true;
		}
		
	public:
		//split input by space and semicolon -> does it match a token? add to stack. if it matches none, call exception
		Lexer(std::string i) {
			input = i;
		}

		tuple<bool, vector<Token>> tokenize() {
			sregex_token_iterator it(input.begin(), input.end(), del), end;

			while (it != end) {
				if (!parseToken(*it, &it, &end)) {
					return { false, {} };
					cout << format("Syntax error at line {} column {}", lc, col) << endl;
					cout << format("Offending token {}", it->str()) << endl;
				}
				++it;
			}
			return { true, tokens};
		}
};

int main() {
	// int* a = new int(1);
	// delete a;
	ifstream file("./in.p");
	string str;
	ostringstream oss;
	while (std::getline(file, str)) {
		oss << str << endl;
	}
	Lexer* l = new Lexer(oss.str());
	auto [success, tokens] = l->tokenize();
	delete l;
	if (!success) {
		return 1;
	}
	for (Token tok: tokens) { cout << tok << endl; }
	return 0;
};
