#pragma once
#include <xstddef>
#include <vector>
typedef unsigned int uint;
class String
{
public:
	const static uint npos=-1;

	char *str;
	uint length;
	uint reserved;

	String(void);
	String(const char *str);
	String(uint reserve);
	String(const String& s);
	String(const char c);
	~String(void);

	uint size() const;
	bool empty() const;
	unsigned char *data() const;
	char back() const;

	void reserve(uint n);
	void push_back(char c);
	char pop_back();

	String& operator += (const String& s);
	String& operator += (const char c);
	String operator + (const String& s) const;
	operator const char*() const;
	int asInt() const;
	bool operator == (const char *s) const;
	bool operator != (const char *s) const;
	bool operator == (const String& s) const;
	bool operator != (const String& s) const;
	String& operator = (const String& s);
	String& operator = (const char *s);
	bool operator<(const String& s) const;

	uint rfind(const char c) const;
	uint find(const char c) const;
	uint find(const char c,uint pos) const;
	uint find_not(const char c) const;
	uint finds(const String &characters,const uint start=0 );
	uint find_nots(const String &characters,const uint start=0 );

	String substr(uint pos=0,uint len=npos) const;
	void insert(uint pos,char c);
	void insert(uint pos,String& s);
	void insert(uint pos,const char *s);
	String& erase(uint pos,uint len);

	String& removeLeadingTrailing(const String& s);
	void split(char c,std::vector<String> &v ) const;
	bool String::operator>(const String& s) const;
};

String operator + (const char* a,const String& b);

namespace std {
	template <> struct hash<String>
	{
		size_t operator()(const String & s) const
		{
			size_t hash = 2166136261U;
			for(size_t i = 0; i < s.length; i++)
			{
				hash = hash ^ (s[i]);       /* xor  the low 8 bits */
				hash = hash * 16777619;  /* multiply by the magic number */
			}
			return hash;
		}
	};
}