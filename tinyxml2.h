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
	static const char* SkipWhiteSpace( const char* p );
	static char* SkipWhiteSpace( char* p ) { return (char*) SkipWhiteSpace( (const char*)p ); }

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

private:

};


class XMLComment : public XMLNode
{

};


class XMLDocument
{
public:
	XMLDocument();

	bool Parse( const char* );

private:
	XMLDocument( const XMLDocument& );	// not implemented

	CharBuffer* charBuffer;
};






};	// tinyxml2



#endif // TINYXML2_INCLUDED