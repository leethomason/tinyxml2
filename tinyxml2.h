#ifndef TINYXML2_INCLUDED
#define TINYXML2_INCLUDED

#include <limits.h>
#include <ctype.h>
#include <stdio.h>

#if defined( _DEBUG ) || defined( DEBUG ) || defined (__DEBUG__)
	#ifndef DEBUG
		#define DEBUG
	#endif
#endif


#if defined(DEBUG)
        #if defined(_MSC_VER)
                #define TIXMLASSERT( x )           if ( !(x)) { _asm { int 3 } } //if ( !(x)) WinDebugBreak()
        #elif defined (ANDROID_NDK)
                #include <android/log.h>
                #define TIXMLASSERT( x )           if ( !(x)) { __android_log_assert( "assert", "grinliz", "ASSERT in '%s' at %d.", __FILE__, __LINE__ ); }
        #else
                #include <assert.h>
                #define TIXMLASSERT                assert
        #endif
#else
        #define TIXMLASSERT( x )           {}
#endif


namespace tinyxml2
{
class XMLDocument;
class XMLElement;
class XMLAttribute;
class XMLComment;
class XMLNode;

// internal - move to separate namespace
struct CharBuffer
{
	size_t  length;
	char	mem[1];

	static CharBuffer* Construct( const char* in );
	static void Free( CharBuffer* );
};

// FIXME: refactor to be the basis for all string handling.
class StrPair
{
public:
	enum {
		NEEDS_ENTITY_PROCESSING			= 0x01,
		NEEDS_NEWLINE_NORMALIZATION		= 0x02
	};

	StrPair() : flags( 0 ), start( 0 ), end( 0 ) {}
	void Set( char* start, char* end, int flags ) {
		this->start = start; this->end = end; this->flags = flags | NEEDS_FLUSH;
	}
	const char* GetStr();
	bool Empty() const { return start == end; }

private:
	enum {
		NEEDS_FLUSH = 0x100
	};

	// After parsing, if *end != 0, it can be set to zero.
	int flags;
	char* start;	
	char* end;
};


class XMLBase
{
public:
	XMLBase() {}
	virtual ~XMLBase() {}

protected:
	static const char* SkipWhiteSpace( const char* p )	{ while( isspace( *p ) ) { ++p; } return p; }
	static char* SkipWhiteSpace( char* p )				{ while( isspace( *p ) ) { ++p; } return p; }

	inline static bool StringEqual( const char* p, const char* q, int nChar=INT_MAX )  {
		int n = 0;
		if ( p == q ) {
			return true;
		}
		while( *p && *q && *p == *q && n<nChar ) {
			++p; ++q; ++n;
		}
		if ( (n == nChar) || ( *p == 0 && *q == 0 ) ) {
			return true;
		}
		return false;
	}
	inline static int IsUTF8Continuation( unsigned char p ) { return p & 0x80; }
	inline static int IsAlphaNum( unsigned char anyByte )	{ return ( anyByte <= 127 ) ? isalnum( anyByte ) : 1; }
	inline static int IsAlpha( unsigned char anyByte )		{ return ( anyByte <= 127 ) ? isalpha( anyByte ) : 1; }

	char* ParseText( char* in, StrPair* pair, const char* endTag );
	char* ParseName( char* in, StrPair* pair );
	char* Identify( XMLDocument* document, char* p, XMLNode** node );
};


class XMLNode : public XMLBase
{
	friend class XMLDocument;
	friend class XMLElement;
public:
	virtual ~XMLNode();

	XMLNode* InsertEndChild( XMLNode* addThis );
	virtual void Print( FILE* cfile, int depth );

	virtual XMLElement* ToElement() { return 0; }

	virtual char* ParseDeep( char* )	{ TIXMLASSERT( 0 ); }

protected:
	XMLNode( XMLDocument* );

	XMLDocument*	document;
	XMLNode*		parent;

	XMLNode*		firstChild;
	XMLNode*		lastChild;

	XMLNode*		prev;
	XMLNode*		next;

private:
	void PrintSpace( FILE* cfile, int depth );			// prints leading spaces.

};


class XMLComment : public XMLNode
{
public:
	XMLComment( XMLDocument* doc );
	virtual ~XMLComment();

	virtual void Print( FILE* cfile, int depth );

	const char* Value() { return value.GetStr(); }

	char* ParseDeep( char* );

protected:

private:
	StrPair value;
};


class XMLAttribute : public XMLBase
{
	friend class XMLElement;
public:
	XMLAttribute( XMLElement* element ) : next( 0 ) {}
	virtual ~XMLAttribute()	{}
	virtual void Print( FILE* cfile );

private:
	char* ParseDeep( char* p );

	StrPair value;
	XMLAttribute* next;
};


class XMLElement : public XMLNode
{
public:
	XMLElement( XMLDocument* doc );
	virtual ~XMLElement();

	const char* Name() { return name.GetStr(); }
	virtual void Print( FILE* cfile, int depth );

	virtual XMLElement* ToElement() { return this; }
	bool Closing() const			{ return closing; }

	char* ParseDeep( char* p );

protected:

private:
	StrPair name;
	bool closing;
	XMLAttribute* rootAttribute;
	XMLAttribute* lastAttribute;
};


class XMLDocument : public XMLBase
{
public:
	XMLDocument();
	~XMLDocument();

	bool Parse( const char* );
	void Print( FILE* cfile=stdout, int depth=0 );

	XMLNode* Root()				{ return root; }
	XMLNode* RootElement();

	enum {
		ERROR_ELEMENT_MISMATCH,
		ERROR_PARSING_ELEMENT,
		ERROR_PARSING_ATTRIBUTE
	};
	void SetError( int error, const char* str1, const char* str2 )	{}

private:
	XMLDocument( const XMLDocument& );	// intentionally not implemented

	XMLNode*	root;
	CharBuffer* charBuffer;
};


};	// tinyxml2



#endif // TINYXML2_INCLUDED