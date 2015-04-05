// MLtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;

typedef std::pair<String, String> IdClass;

class Symbol {
public:
	String str;
	Symbol *last = NULL, *next = NULL;
	virtual bool canBeConvertedFrom(Symbol* symbol) {
		return false;
	}
};

// a symbol that is raw text from a file
class Token : public Symbol
{
public:
	int line;
	int pos;
	String after;
	IdClass *idClass=NULL;
	Token(String s)
	{
		line = -1;
		pos = -1;
		str = s;
	}
};
vector<IdClass* > idClasses = {
	new IdClass({ ";{}", "" }),
	new IdClass({ "abcdefghijklmnopqrstuvwxzyABCDEFGHIJKLMNOPQRSTUVWXYZ_", "abcdefghijklmnopqrstuvwxzyABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_" }),
	new IdClass({ "0123456789", "0123456789" }),
	new IdClass({ "+-/*|&!`~#$%^()[]?\\,.<>=:\'\"", "" }),
};


class Match {
public:
	/*
		return the last token successfully matched, or NULL
	*/
	virtual Symbol* check(Symbol *symbol) {
		return symbol;
	}
};
class MString :public Match {
public:
	String str;
	MString(String _str) {
		str = _str;
	}
	Symbol *check(Symbol *token) {
		if (!token)
			return NULL;
		return token->str == str? token:NULL;
	}
};
#define str_(str) ((new MString(((str)))))
class MIdClass : public Match {
public:
	IdClass *idClass;
	MIdClass(IdClass *_idClass) {
		idClass = _idClass;
	}
	Symbol* check(Symbol *s) {
		if (!s)
			return NULL;
		Token *token = dynamic_cast<Token*>(s);
		return token->idClass == idClass? token:NULL;
	}
};
#define id_(n) ((new MIdClass(idClasses[n])))


class MSeries :public Match {
public:
	vector<Match*> matches;
	MSeries(vector<Match*> _matches) {
		matches = _matches;
	}
	Symbol* check(Symbol *token) {
		if (!token)
			return NULL;
		Symbol *lastSymbol = NULL;
		auto it = matches.begin();
		for (; it != matches.end(); it++) {
			Symbol *next = (*it)->check(token);
			if (next) {
				lastSymbol = next;
				token = next->next;
			}
			else
				break;
		}
		if (it == matches.end())
			return lastSymbol;
		return NULL;
	}
};
#define series_(matches) ((new MSeries(((matches)))))

/*
	match \min\ or more of the pattern \matches\, followed by a single instance of \terminal\
	*/
class MMultiple :public Match {
public:
	Match *match;
	Match *terminal = NULL;
	int min = 1;
	int max = -1;
	MMultiple(Match* _match, Match *_terminal=NULL,int _min = 1,int _max=-1) {
		match = _match;
		min = _min;
		terminal = _terminal;
		max = _max;
	}
	Symbol* check(Symbol *token) {
		if (!token && min>0)
			return NULL;
		int nMatch = 0;
		Symbol *lastSymbol = NULL;
		while (token) {
			if (terminal && terminal->check(token))
				break;
			Symbol *next = match->check(token);
			if (next) {
				lastSymbol = next;
				token = next->next;
				nMatch++;
				if (nMatch == max)
					break;
			}
			else
				break;
		}
		if (nMatch < min)
			return NULL;
		if (terminal)
			return terminal->check(token);
		else
			return lastSymbol;
	}
};
#define many_(matches) ((new MMultiple(((matches)))))
#define many_t(matches,term) ((new MMultiple{((matches)),((term)))))
#define many_tn(matches,term,min) ((new MMultiple(((matches)),((term)),min)))
#define many_n(matches,min) ((new MMultiple(((matches)),NULL,min)))
#define opt_(matches) ((new MMultiple(((matches)),NULL,0,1)))

class MAnyOf : public Match {
public:
	vector<Match*> possibilities;
	MAnyOf(vector<Match*> _p) {
		possibilities = _p;
	}
	Symbol* check(Symbol* token) {
		auto it = possibilities.begin();
		Symbol *found = NULL;
		for (; it != possibilities.end(); it++) {
			if ((found = (*it)->check(token)) != NULL)
				break;
		}
		return found;
	}
};
#define anyof_(matches) ((new MAny(((matches)))))

class MNotPrecededBy : public Match {
public:
	Match *not;
	Match *match;
	MNotPrecededBy(Match *_match, Match *_not) {
		not = _not;
		match = _match;
	}
	Symbol* check(Symbol *symbol) {
		if (!symbol)
			return NULL; 
		if (symbol->last) {
			if ((not->check(symbol->last)) != NULL)
				return NULL;
		}
		Symbol *end;
		if ((end = match->check(symbol)) == NULL)
			return NULL;
		return end;
	}
};


class Macro
{
public:
	Match *pattern;
	double priority;
	virtual vector<Symbol*> onMatch(Symbol *begin, Symbol *end) = 0;
	Macro(Match *_pattern, double _priority) {
		pattern = _pattern;
		priority = _priority;
	}
};

template<class T>
class SingleResultMacro : public Macro {
public:
	SingleResultMacro(Match *_pattern, double _priority) : Macro(_pattern, _priority){}
	virtual vector<Symbol*> onMatch(Symbol *begin, Symbol *end)
	{
		vector<Symbol*> v;
		v.push_back(new T(begin, end));
			return v;
	}
};

class MultilineComment : public Symbol {
public:
	String contents;
	MultilineComment(Symbol *begin, Symbol *end) {
		Symbol *sym = begin->next;
		while (sym && sym != end) {
			contents += sym->str;
			Token *token = dynamic_cast<Token*>(sym);
			if (token)
				contents += token->after;
			sym = sym->next;
		}
	}
};

class StringLiteral :public Symbol {
public:
	String contents;
	StringLiteral(Symbol *begin, Symbol *end) {
		Symbol *sym = begin->next;
		while (sym && sym != end) {
			contents += sym->str;
			Token *token = dynamic_cast<Token*>(sym);
			if (token)
				contents += token->after;
			sym = sym->next;
		}
		str = "\"" + contents + "\"";
	}
};

vector<Macro*> macros = {
	new SingleResultMacro<MultilineComment>(
	series_(vector<Match*>({ str_("/"), str_("*"), many_tn(new Match(), series_(vector<Match*>({ str_("*"), str_("/") })), 0) })),
		2000
	),
	new SingleResultMacro<StringLiteral>(
		series_(vector<Match*>({ str_("\""), many_tn(new Match(), new MNotPrecededBy(str_("\""), str_("\\")), 0) })),
		1000
	),
};

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *fp;
	//fopen_s(&fp, "file.q", "r");
	String file;
	String whitespace = " \n\r\t";
	while (0)
	{
		file += fgetc(fp);
		if (feof(fp))
			break;
	}
	//file = "hello 123 goodbye123 1+2*chao3";
	file = "\"the quick \\\" /* commented out */ brown \\\" fox\" ERROR";
	vector<Token*> tokens;
	int at = 0;
	int line = 0;
	for (; at < file.length; at++)
	{
		Token *token=new Token("");
		if (tokens.size())
		{
			token->last = tokens.back();
			tokens.back()->next = token;
		}
		while (whitespace.find(file[at]) != String::npos) 
		{ 
			if (file[at] == '\n') 
				line++; 
			if (token->last)
				((Token*)token->last)->after += file[at];
			at++;	
		}
		for (auto match : idClasses)
		{
			if (match->first.find(file[at]) != String::npos)
			{
				token->pos = at;
				token->line = line;
				token->idClass = match;
				token->str += file[at];
				while (match->second.find(file[at+1]) != String::npos)
				{
					token->str += file[++at];
				}
				break;
			}
		}
		if (!token->str.empty())
			tokens.push_back(token);
		else
		{
			printf("%c doesn't match! on line %i", file[at++],line);
		}//uhh
	}

	{
		vector<Symbol*> program;
		for (auto token : tokens) {
			program.push_back(token);
		}

		/*Symbol* match = macros[1]->pattern->check(program[0]);
		printf("%i\n", match);*/

		for (auto macro : macros) {
			Symbol *sym = program[0];
			while (sym) {
				Symbol *end = macro->pattern->check(sym);
				if (end) {
					vector<Symbol*> replacement = macro->onMatch(sym, end);
					if (sym->last) {
						sym->last->next = replacement[0];
						replacement[0]->last = sym->last;
					}
					else
						program[0] = replacement[0];
					if (end->next) {
						end->next->last = replacement.back();
						replacement.back()->next = end->next;
					}
					sym = replacement.back();
					for (int i = 0; i<replacement.size(); i++) {
						if (i>0)
							replacement[i]->last = replacement[i - 1];
						if (i + 1 < replacement.size())
							replacement[i]->next = replacement[i + 1];
					}
				}
				sym = sym->next;
			}
		}

		printf("");
	}
	return 0;
}

