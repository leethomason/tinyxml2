#include "tinyxml2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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


XMLDocument::XMLDocument() : 
	charBuffer( 0 )
{
}


bool XMLDocument::Parse( const char* str ) 
{

	return true;
}
