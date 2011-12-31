#include "tinyxml2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

using namespace tinyxml2;

/*static*/ CharBuffer* CharBuffer::Construct( const char* in )
{
	size_t len = strlen( in );
	size_t size = len + sizeof( CharBuffer );
	CharBuffer* cb = (CharBuffer*) malloc( size );
	cb->length = len;
	strcpy( cb->mem, in );
	return cb;
}


/*static*/ void CharBuffer::Free( CharBuffer* cb )
{
	free( cb );
}


const char* XMLNode::SkipWhiteSpace( const char* p )
{
	while( isspace( *p ) ) {
		++p;
	}
	return p;
}


XMLDocument::XMLDocument() : 
	charBuffer( 0 )
{
}


bool XMLDocument::Parse( const char* p ) 
{
	XMLNode* returnNode = 0;

	p = XMLNode::SkipWhiteSpace( p );
	if( !p || !*p || *p != '<' )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };
	const char* cdataHeader = { "<![CDATA[" };

	if ( XMLNode::StringEqual( p, xmlHeader, 5 ) ) {

	}
	else {
		TIXMLASSERT( 0 );
	}

	return true;
}
