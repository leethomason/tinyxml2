#include "tinyxml2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

using namespace tinyxml2;

static const char LINE_FEED				= (char)0x0a;				// all line endings are normalized to LF
static const char LF = LINE_FEED;
static const char CARRIAGE_RETURN		= (char)0x0d;			// CR gets filtered out
static const char CR = CARRIAGE_RETURN;


// --------- CharBuffer ----------- //
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


// --------- XMLNode ----------- //

XMLNode::XMLNode( XMLDocument* doc ) :
	document( doc ),
	parent( 0 ),
	firstChild( 0 ), lastChild( 0 ),
	prev( 0 ), next( 0 )
{

}


XMLNode::~XMLNode()
{
	XMLNode* node=firstChild;
	while( node ) {
		XMLNode* temp = node->next;
		delete node;
		node = temp;
	}
}


XMLNode* XMLNode::InsertEndChild( XMLNode* addThis )
{
	if ( lastChild ) {
		TIXMLASSERT( firstChild );
		TIXMLASSERT( lastChild->next == 0 );
		lastChild->next = addThis;
		addThis->prev = lastChild;
		lastChild = addThis;

		addThis->parent = this;
		addThis->next = 0;
	}
	else {
		TIXMLASSERT( firstChild == 0 );
		firstChild = lastChild = addThis;

		addThis->parent = this;
		addThis->prev = 0;
		addThis->next = 0;
	}
	return addThis;
}


void XMLNode::Print( FILE* fp, int depth )
{
	for( XMLNode* node = firstChild; node; node=node->next ) {
		node->Print( fp, depth );
	}
}

void XMLNode::PrintSpace( FILE* fp, int depth ) 
{
	for( int i=0; i<depth; ++i ) {
		fprintf( fp, "    " );
	}
}


const char* XMLNode::ParseText( char* p, const char* endTag, char** next )
{
	TIXMLASSERT( endTag && *endTag );

	char* start = p;
	char* q = p;		// q (target) <= p (src) in same buffer.
	char  endChar = *endTag;
	int   length = strlen( endTag );	
	char* nextTag = 0;

	// Inner loop of text parsing.
	while ( *p ) {
		if ( *p == endChar && strncmp( p, endTag, length ) == 0 ) {
			*q = 0;
			nextTag = p + length;
			break;
		}
		else if ( *p == CR ) {
			// CR-LF pair becomes LF
			// CR alone becomes LF
			// LF-CR becomes LF
			if ( *(p+1) == LF ) {
				p += 2;
			}
			else {
				++p;
			}
			*q = LF;
		}
		else if ( *p == LF ) {
			if ( *(p+1) == CR ) {
				p += 2;
			}
			else {
				++p;
			}
			*q = LF;
		}
		else {
			*q = *p;
			++p;
		}
		++q;
	}	

	// Error? If we don't have a text tag, something went wrong. (Although 
	// what the nextTag points at may be null.)
	if ( nextTag == 0 ) {
		return 0;
	}
	*next = nextTag;
	return start;
}


// --------- XMLComment ---------- //

XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLComment::~XMLComment()
{

}


void XMLComment::Print( FILE* fp, int depth )
{
	XMLNode::Print( fp, depth );
	fprintf( fp, "<!--%s-->\n", value );
}


char* XMLComment::ParseDeep( char* p )
{
	// Comment parses as text.
	value = ParseText( p, "-->", &p );
	return p;
}


// --------- XMLDocument ----------- //
XMLDocument::XMLDocument() : 
	charBuffer( 0 )
{
	root = new XMLNode( this );
}


XMLDocument::~XMLDocument()
{
	delete root;
	delete charBuffer;
}



bool XMLDocument::Parse( const char* p )
{
	charBuffer = CharBuffer::Construct( p );
	XMLNode* node = 0;
	
	char* q = Identify( charBuffer->mem, &node );
	root->InsertEndChild( node );
	node->ParseDeep( q );

	return true;
}


void XMLDocument::Print( FILE* fp, int depth ) 
{
	for( XMLNode* node = root->firstChild; node; node=node->next ) {
		node->Print( fp, depth );
	}
}


char* XMLDocument::Identify( char* p, XMLNode** node ) 
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

	static const char* xmlHeader = { "<?xml" };
	static const char* commentHeader = { "<!--" };
	static const char* dtdHeader = { "<!" };
	static const char* cdataHeader = { "<![CDATA[" };

	static const int xmlHeaderLen = 5;
	static const int commentHeaderLen = 4;
	static const int dtdHeaderLen = 2;
	static const int cdataHeaderLen = 9;

	if ( XMLNode::StringEqual( p, commentHeader, commentHeaderLen ) ) {
		returnNode = new XMLComment( this );
		p += commentHeaderLen;
	}
	else {
		TIXMLASSERT( 0 );
	}

	*node = returnNode;
	return p;
}
