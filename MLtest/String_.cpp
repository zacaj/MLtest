#include "stdafx.h"



String::String(void)
{
	reserved=8;
	str=new char[reserved];
	str[0]='\0';
	length=0;
}

String::String( const char *_str )
{
	length=strlen(_str);
	reserved=length+1;
	str = new char[reserved];
	strcpy(str,_str);
}

String::String( uint reserve )
{
	reserved=reserve;
	str = new char[reserved];
	str[0]='\0';
	length=0;
}

String::String( const String& s )
{
	reserved=s.reserved;
	str = new char[reserved];
	length=s.length;
	strcpy(str,s.str);
}

String::String( const char c )
{
	reserved=8;
	str = new char[reserved];
	str[0]=c;
	str[1]='\0';
	length=1;
}


String::~String(void)
{
	delete[] str;
}

uint String::size() const
{
	return length;
}

void String::reserve( uint n )
{
	if(n<=reserved)
		return;
	char *oldStr=str;
	str = new char[reserved=n];
	strcpy(str,oldStr);
	delete[] oldStr;
}

void String::push_back( char c )
{
	if(length+1>=reserved)
		reserve(length*2+1);
	str[length++]=c;
	str[length]='\0';
}

String& String::operator+=( const String& s )
{
	if(length+s.length>=reserved)
		reserve(length+s.length+2);
	strcat(str,s.str);
	length+=s.length;
	str[length]='\0';
	return *this;
}

String& String::operator+=( const char c )
{
	push_back(c);
	return *this;
}

String::operator const char*() const
{
	return str;
}

String String::operator+( const String& s ) const
{
	return String(*this)+=s;
}

String String::substr( uint pos/*=0*/,uint len/*=npos*/ ) const
{
	String r(len+1);
	r.length=len;
	strncpy(r.str,str+pos,len);
	r.str[r.length]='\0';
	return r;
}

uint String::rfind( const char c ) const
{
	char *s=strrchr(str,c);
	if(s==NULL)
		return npos;
	return s-str;
}

uint String::find( const char c,uint pos ) const
{
	char *s=strchr(str+pos,c);
	if(s==NULL)
		return npos;
	return s-str;
}

uint String::find( const char c ) const
{
	char *s=strchr(str,c);
	if(s==NULL)
		return npos;
	return s-str;
}

uint String::finds( const String &characters,const uint start/*=0 */ )
{
	uint first=size();
	for(int i=0;i<characters.size();i++)
	{
		uint pos=find(characters[i],start);
		if(pos<first)
			first=pos;
	}
	return first;
}

void String::insert( uint pos,char c )
{
	if(length+1>=reserved)
		reserve(length+8);
	for(int i=length;i>pos;i--)
		str[i]=str[i-1];
	str[pos]=c;
	length++;
	str[length]='\0';
}

void String::insert( uint pos,String& s )
{
	if(length+s.length>=reserved)
		reserve(length+s.length+2);
	for(int i=length;i>pos;i--)
		str[i]=str[i-s.length];
	strncpy(str+pos,s,s.length);
	length+=s.length;
	str[length]='\0';
}

void String::insert( uint pos,const char *s )
{
	String t(s);
	insert(pos,t);
}

bool String::empty() const
{
	return length==0;
}

String& String::erase( uint pos,uint len )
{
	for(int i=pos;i<length-len;i++)
		str[i]=str[i+len];
	length-=len;
	str[length]='\0';
	return *this;
}

unsigned char * String::data() const
{
	return (unsigned char*)str;
}

bool String::operator==( const char *s ) const
{
	return *this==String(s);
}

bool String::operator==( const String& s ) const
{
	return strcmp(str,s.str)==0;
}

String& String::operator=( const String& s )
{
	if(reserved<s.length+1)
		reserve(s.length+2);
	strcpy(str,s.str);
	length=s.length;
	return *this;
}

String& String::operator=( const char *_str )
{
	length=strlen(_str);
	reserved=length+1;
	delete[] str;
	str = new char[reserved];
	strcpy(str,_str);
	return *this;
}

bool String::operator<( const String& s ) const
{
	return strcmp(str,s.str)<0;
}

bool String::operator>(const String& s) const
{
	return strcmp(str, s.str) > 0;
}

String& String::removeLeadingTrailing( const String& s )
{
	if(empty())
		return *this;
	size_t notSpace=find_nots(s);
	if(notSpace!=String::npos && notSpace!=0)
		erase(0,notSpace);
	while(1) 
	{
		int i;
		for(i=0;i<s.length;i++)
			if(s[i]==back())
				break;
		if(i==s.length)
			break;
		pop_back();
	}
	return *this;
}

char String::back() const
{
	return str[length-1];
}

uint String::find_not( char c ) const
{
	uint i=0;
	while(i<length && str[i]==c) i++;
	return i;
}

uint String::find_nots( const String& characters,const uint start/*=0 */ )
{
	uint first=size();
	for(int i=start;i<size();i++)
	{
		int j=0;
		for(;j<characters.size();j++)
		{
			if(str[i]==characters[j])
				break;
		}
		if(j==characters.size())
			return i;
	}
	return first;
}

char String::pop_back()
{
	char ret=back();
	length--;
	str[length]='\0';
	return ret;
}

int String::asInt() const
{
	int i;
	sscanf(str,"%i",&i);
	return i;
}

bool String::operator!=( const char *s ) const
{
	return !(*this==String(s));
}

bool String::operator!=( const String& s ) const
{
	return !(*this==s);
}

void String::split( char c,std::vector<String> &v ) const
{
	uint last=0;
	while(1)
	{
		uint next=find(c,last);
		if(next==npos)
		{
			v.push_back(substr(last,length-last));
			break;
		}
		v.push_back(substr(last,next-last));
		last=next+1;
	}
}

String operator+( const char* a,const String& b )
{
	return String(a)+b;
}
