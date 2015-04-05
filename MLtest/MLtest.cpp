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

struct Macro
{
	String name;
	Match *match;
	double priority;
};

vector<Macro> macros = {
	{
		"multi-line comment",
		series_(vector<Match*>({ str_("/*"), many_tn(new Match(), str_("*/"), 0) })),
		2000
	},
	{
		"string literal",
		series_(vector<Match*>({ str_("\""), many_tn(new Match() , new MNotPrecededBy(str_("\""),str_("\\")), 0) })),
		1000
	}
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
	file = "\"aaa \\\" gsdgsdg\"dfgd";
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

		Symbol* match = macros[1].match->check(program[0]);
		printf("%i\n", match);
	}
	return 0;
}

