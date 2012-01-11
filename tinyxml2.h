#ifndef TINYXML2_INCLUDED
#define TINYXML2_INCLUDED

#include <limits.h>

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
class XMLDocument*;

// internal - move to separate namespace
struct CharBuffer
{
	size_t  length;
	char	mem[1];

	static CharBuffer* Construct( const char* in );
	static void Free( CharBuffer* );
};


class XMLNode
{
	friend class XMLDocument;
public:

	static XMLNode* Identify( const char* p );

protected:
	XMLNode( XMLDocument* );
	virtual ~XMLNode();

	// Utility
	static const char* SkipWhiteSpace( const char* p )	{ while( isspace( *p ) ) { ++p; } return p; }
	static char* SkipWhiteSpace( char* p )				{ while( isspace( *p ) ) { ++p; } return p; }

	inline static bool StringEqual( const char* p, const char* q, int nChar=INT_MAX )  {
		int n = 0;
		while( *p && *q && *p == *q && n<nChar ) {
			++p; ++q; ++n;
		}
		if ( (n == nChar) || ( *p == 0 && *q == 0 ) ) {
			return true;
		}
		return false;
	}

	/* Parses text. (Not a text node.)
	   - [ ] EOL normalization.
	   - [x] Trim leading whitespace
	   - [ ] Trim trailing whitespace.
	   - [ ] Leaves inner whitespace
	   - [ ] Inserts one space between lines.
	*/
	const char* ParseText( char* in, const char* endTag, char** next );

	virtual char* ParseDeep( char* )	{ TIXMLASSERT( 0 ); }

	XMLDocument*	document;
	XMLNode*		parent;

	XMLNode*		firstChild;
	XMLNode*		lastChild;

	XMLNode*		prev;
	XMLNode*		next;

private:

};


class XMLComment : public XMLNode
{
public:
	XMLComment( XMLDocument* doc );
	virtual ~XMLComment();

private:
	char* value;
};


class XMLDocument
{
public:
	XMLDocument();
	~XMLDocument();

	bool Parse( const char* );

	XMLNode* Root()				{ return root; }
	XMLNode* RootElement();

	XMLNode* InsertEndChild( XMLNode* addThis );

private:
	XMLDocument( const XMLDocument& );	// intentionally not implemented

	virtual char* ParseDeep( char* );

	XMLNode*	root;
	CharBuffer* charBuffer;
};






};	// tinyxml2



#endif // TINYXML2_INCLUDED