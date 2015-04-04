// MLtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;

typedef std::pair<String, String> IdClass;
struct Token
{
	int line;
	int pos;
	String str;
	Token *last=NULL, *next=NULL;
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
	virtual Token* check(Token *token) = 0;
};
class MString :public Match {
public:
	String str;
	MString(String _str) {
		str = _str;
	}
	Token *check(Token *token) {
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
	Token* check(Token *token) {
		if (!token)
			return NULL;
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
	Token* check(Token *token) {
		if (!token)
			return NULL;
		Token *lastToken = NULL;
		auto it = matches.begin();
		for (; it != matches.end(); it++) {
			Token *next = (*it)->check(token);
			if (next) {
				lastToken = next;
				token = next->next;
			}
			else
				break;
		}
		if (it == matches.end())
			return lastToken;
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
	Token* check(Token *token) {
		if (!token && min>0)
			return NULL;
		int nMatch = 0;
		Token *lastToken = NULL;
		while (true) {
			if (terminal && terminal->check(token))
				break;
			Token *next = match->check(token);
			if (next) {
				lastToken = next;
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
			return lastToken;
	}
};
#define many_(matches) ((new MMultiple(((matches)))))
#define many_t(matches,term) ((new MMultiple{((matches)),((term)))))
#define many_tn(matches,term,min) ((new MMultiple(((matches)),((term)),min)))
#define many_n(matches,min) ((new MMultiple(((matches)),NULL,min)))
#define opt_(matches) ((new MMultiple(((matches)),NULL,0,1)))


struct Macro
{
	String name;
	Match *match;
};

vector<Macro> macros = {
	{
		"identifierTest",
		str_("void")
	},
	{
		"stringTest",
		id_(1)
	},
	{
		"operatorTest",
		id_(3)
	},
	{
		"sentenceTest",
		series_(vector<Match*>({ str_("the"), str_("quick"), str_("brown"), str_("fox") }))
	},
	{
		"stringLiteralTest",
		series_(vector<Match*>({ str_("\""), many_tn(id_(1),str_("\""),0) }))
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
	file = "\"aaa bbbb\"dfgd";
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
				token->last->after += file[at];
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

	Token* match=macros[4].match->check(tokens[0]);
	return 0;
}

