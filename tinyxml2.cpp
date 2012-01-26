#include "tinyxml2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

using namespace tinyxml2;

static const char LINE_FEED				= (char)0x0a;			// all line endings are normalized to LF
static const char LF = LINE_FEED;
static const char CARRIAGE_RETURN		= (char)0x0d;			// CR gets filtered out
static const char CR = CARRIAGE_RETURN;
static const char SINGLE_QUOTE			= '\'';
static const char DOUBLE_QUOTE			= '\"';


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


const char* StrPair::GetStr()
{
	if ( flags & NEEDS_FLUSH ) {
		*end = 0;

		if ( flags & ( NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION ) ) {
			char* p = start;
			char* q = start;

			while( p < end ) {
				if ( *p == CR ) {
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
					++q;
				}
			}
		}
		flags = 0;
	}
	return start;
}


// --------- XMLBase ----------- //
char* XMLBase::ParseText( char* p, StrPair* pair, const char* endTag )
{
	TIXMLASSERT( endTag && *endTag );

	char* start = p;
	char  endChar = *endTag;
	int   length = strlen( endTag );	

	// Inner loop of text parsing.
	while ( *p ) {
		if ( *p == endChar && strncmp( p, endTag, length ) == 0 ) {
			pair->Set( start, p, StrPair::NEEDS_ENTITY_PROCESSING | StrPair::NEEDS_NEWLINE_NORMALIZATION );
			return p + length;
		}
		++p;
	}	
	return p;
}


char* XMLBase::ParseName( char* p, StrPair* pair )
{
	char* start = p;

	start = p;
	if ( !start || !(*start) ) {
		return 0;
	}

	if ( !IsAlpha( *p ) ) {
		return 0;
	}

	while( *p && (
			   IsAlphaNum( (unsigned char) *p ) 
			|| *p == '_'
			|| *p == '-'
			|| *p == '.'
			|| *p == ':' ))
	{
		++p;
	}

	if ( p > start ) {
		pair->Set( start, p, 0 );
		return p;
	}
	return 0;
}


char* XMLBase::Identify( XMLDocument* document, char* p, XMLNode** node ) 
{
	XMLNode* returnNode = 0;
	char* start = p;
	p = XMLNode::SkipWhiteSpace( p );
	if( !p || !*p )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	static const char* xmlHeader		= { "<?xml" };
	static const char* commentHeader	= { "<!--" };
	static const char* dtdHeader		= { "<!" };
	static const char* cdataHeader		= { "<![CDATA[" };
	static const char* elementHeader	= { "<" };	// and a header for everything else; check last.

	static const int xmlHeaderLen		= 5;
	static const int commentHeaderLen	= 4;
	static const int dtdHeaderLen		= 2;
	static const int cdataHeaderLen		= 9;
	static const int elementHeaderLen	= 1;

	if ( StringEqual( p, commentHeader, commentHeaderLen ) ) {
		returnNode = new XMLComment( document );
		p += commentHeaderLen;
	}
	else if ( StringEqual( p, elementHeader, elementHeaderLen ) ) {
		returnNode = new XMLElement( document );
		p += elementHeaderLen;
	}
	// fixme: better text detection
	else if ( (*p != '<') && IsAlphaNum( *p ) ) {
		// fixme: this is filtering out empty text...should it?
		returnNode = new XMLText( document );
		p = start;	// Back it up, all the text counts.
	}
	else {
		TIXMLASSERT( 0 );
	}

	*node = returnNode;
	return p;
}


// --------- XMLNode ----------- //

XMLNode::XMLNode( XMLDocument* doc ) :
	document( doc ),
	parent( 0 ),
	isTextParent( false ),
	firstChild( 0 ), lastChild( 0 ),
	prev( 0 ), next( 0 )
{

}


XMLNode::~XMLNode()
{
	//printf( "~XMLNode %x\n", this );
	while( firstChild ) {
		XMLNode* node = firstChild;
		Unlink( node );
		delete node;
	}
}


void XMLNode::Unlink( XMLNode* child )
{
	TIXMLASSERT( child->parent == this );
	if ( child == firstChild ) 
		firstChild = firstChild->next;
	if ( child == lastChild ) 
		lastChild = lastChild->prev;

	if ( child->prev ) {
		child->prev->next = child->next;
	}
	if ( child->next ) {
		child->next->prev = child->prev;
	}
	child->parent = 0;
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
	if ( addThis->ToText() ) {
		SetTextParent();
	}
	return addThis;
}


void XMLNode::Print( XMLStreamer* streamer )
{
	for( XMLNode* node = firstChild; node; node=node->next ) {
		node->Print( streamer );
	}
}


char* XMLNode::ParseDeep( char* p )
{
	while( p && *p ) {
		XMLNode* node = 0;
		p = Identify( document, p, &node );
		if ( p && node ) {
			p = node->ParseDeep( p );
			// FIXME: is it the correct closing element?
			if ( node->IsClosingElement() ) {
				delete node;
				return p;
			}
			this->InsertEndChild( node );
		}
	}
	return 0;
}

/*
void XMLNode::PrintSpace( FILE* fp, int depth ) 
{
	for( int i=0; i<depth; ++i ) {
		fprintf( fp, "    " );
	}
}
*/

// --------- XMLText ---------- //
char* XMLText::ParseDeep( char* p )
{
	p = ParseText( p, &value, "<" );
	// consumes the end tag.
	if ( p && *p ) {
		return p-1;
	}
	return 0;
}


void XMLText::Print( XMLStreamer* streamer )
{
	const char* v = value.GetStr();
	streamer->PushText( v );
}


// --------- XMLComment ---------- //

XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLComment::~XMLComment()
{
	//printf( "~XMLComment\n" );
}


void XMLComment::Print( XMLStreamer* streamer )
{
//	XMLNode::Print( fp, depth );
//	fprintf( fp, "<!--%s-->\n", value.GetStr() );
	streamer->PushComment( value.GetStr() );
}


char* XMLComment::ParseDeep( char* p )
{
	// Comment parses as text.
	return ParseText( p, &value, "-->" );
}


// --------- XMLAttribute ---------- //
char* XMLAttribute::ParseDeep( char* p )
{
	p = ParseText( p, &name, "=" );
	if ( !p || !*p ) return 0;

	char endTag[2] = { *p, 0 };
	++p;
	p = ParseText( p, &value, endTag );
	if ( value.Empty() ) return 0;
	return p;
}


void XMLAttribute::Print( XMLStreamer* streamer )
{
	// fixme: sort out single vs. double quote
	//fprintf( cfile, "%s=\"%s\"", name.GetStr(), value.GetStr() );
	streamer->PushAttribute( name.GetStr(), value.GetStr() );
}


// --------- XMLElement ---------- //
XMLElement::XMLElement( XMLDocument* doc ) : XMLNode( doc ),
	closing( false ),
	rootAttribute( 0 ),
	lastAttribute( 0 )
{
}


XMLElement::~XMLElement()
{
	//printf( "~XMLElemen %x\n",this );

	XMLAttribute* attribute = rootAttribute;
	while( attribute ) {
		XMLAttribute* next = attribute->next;
		delete attribute;
		attribute = next;
	}
}


char* XMLElement::ParseAttributes( char* p, bool* closedElement )
{
	const char* start = p;
	*closedElement = false;

	// Read the attributes.
	while( p ) {
		p = SkipWhiteSpace( p );
		if ( !p || !(*p) ) {
			document->SetError( XMLDocument::ERROR_PARSING_ELEMENT, start, name.GetStr() );
			return 0;
		}

		// attribute.
		if ( IsAlpha( *p ) ) {
			XMLAttribute* attrib = new XMLAttribute( this );
			p = attrib->ParseDeep( p );
			if ( !p ) {
				delete attrib;
				document->SetError( XMLDocument::ERROR_PARSING_ATTRIBUTE, start, p );
				return 0;
			}
			if ( rootAttribute ) {
				TIXMLASSERT( lastAttribute );
				lastAttribute->next = attrib;
				lastAttribute = attrib;
			}
			else {
				rootAttribute = lastAttribute = attrib;
			}
		}
		// end of the tag
		else if ( *p == '/' && *(p+1) == '>' ) {
			if ( closing ) {
				document->SetError( XMLDocument::ERROR_PARSING_ELEMENT, start, p );
				return 0;
			}
			*closedElement = true;
			return p+2;	// done; sealed element.
		}
		// end of the tag
		else if ( *p == '>' ) {
			++p;
			break;
		}
		else {
			document->SetError( XMLDocument::ERROR_PARSING_ELEMENT, start, p );
			return 0;
		}
	}
	return p;
}


//
//	<ele></ele>
//	<ele>foo<b>bar</b></ele>
//
char* XMLElement::ParseDeep( char* p )
{
	// Read the element name.
	p = SkipWhiteSpace( p );
	if ( !p ) return 0;
	const char* start = p;

	// The closing element is the </element> form. It is
	// parsed just like a regular element then deleted from
	// the DOM.
	if ( *p == '/' ) {
		closing = true;
		++p;
	}

	p = ParseName( p, &name );
	if ( name.Empty() ) return 0;

	bool elementClosed=false;
	p = ParseAttributes( p, &elementClosed );
	if ( !p || !*p || elementClosed || closing ) 
		return p;

	p = XMLNode::ParseDeep( p );
	return p;
}


void XMLElement::Print( XMLStreamer* streamer )
{
	//if ( !parent || !parent->IsTextParent() ) {
	//	PrintSpace( cfile, depth );
	//}
	//fprintf( cfile, "<%s", Name() );
	streamer->OpenElement( Name(), IsTextParent() );

	for( XMLAttribute* attrib=rootAttribute; attrib; attrib=attrib->next ) {
		//fprintf( cfile, " " );
		attrib->Print( streamer );

	}

	for( XMLNode* node=firstChild; node; node=node->next ) {
		node->Print( streamer );
	}
	streamer->CloseElement();

/*	if ( firstChild ) {
		fprintf( cfile, ">", Name() );
		if ( !IsTextParent() ) {
			fprintf( cfile, "\n" );
		}

		for( XMLNode* node=firstChild; node; node=node->next ) {
			node->Print( cfile, depth+1 );
		}

		fprintf( cfile, "</%s>", Name() );
		if ( !IsTextParent() ) {
			fprintf( cfile, "\n" );
		}
	}
	else {
		fprintf( cfile, "/>" );
		if ( !IsTextParent() ) {
			fprintf( cfile, "\n" );
		}
	}*/
}


// --------- XMLDocument ----------- //
XMLDocument::XMLDocument() :
	XMLNode( this ),
	charBuffer( 0 )
{
}


XMLDocument::~XMLDocument()
{
}



bool XMLDocument::Parse( const char* p )
{
	charBuffer = CharBuffer::Construct( p );
	XMLNode* node = 0;
	
	char* q = ParseDeep( charBuffer->mem );
	return true;
}


void XMLDocument::Print( XMLStreamer* streamer ) 
{
	XMLStreamer stdStreamer( stdout );
	if ( !streamer )
		streamer = &stdStreamer;
	for( XMLNode* node = firstChild; node; node=node->next ) {
		node->Print( streamer );
	}
}


void XMLDocument::SetError( int error, const char* str1, const char* str2 )
{
	printf( "ERROR: id=%d '%s' '%s'\n", error, str1, str2 );
}


StringStack::StringStack()
{
	*pool = 0;
	mem = pool;
	inUse = 1;	// always has a null
	allocated = INIT;
	nPositive = 0;
}


StringStack::~StringStack()
{
	if ( mem != pool ) {
		delete [] mem;
	}
}


void StringStack::Push( const char* str ) {
	int needed = strlen( str ) + 1;
	if ( needed > 1 )
		nPositive++;
	if ( inUse+needed > allocated ) {
		// fixme: power of 2
		// less stupid allocation
		int more = inUse+needed + 1000;

		char* newMem = new char[more];
		memcpy( newMem, mem, inUse );
		if ( mem != pool ) {
			delete [] mem;
		}
		mem = newMem;
	}
	strcpy( mem+inUse, str );
	inUse += needed;
}


const char* StringStack::Pop() {
	TIXMLASSERT( inUse > 1 );
	const char* p = mem+inUse-2;
	if ( *p ) {
		nPositive--;
	}
	while( *p ) {					// stack starts with a null, don't need to check for 'mem'
		TIXMLASSERT( p > mem );
		--p;
	}
	inUse = p-mem+1;
	return p+1;
}


XMLStreamer::XMLStreamer( FILE* file ) : fp( file ), depth( 0 ), elementJustOpened( false )
{
}


void XMLStreamer::PrintSpace( int depth )
{
	for( int i=0; i<depth; ++i ) {
		fprintf( fp, "    " );
	}
}


void XMLStreamer::OpenElement( const char* name, bool textParent )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	if ( text.NumPositive() == 0 ) {
		PrintSpace( depth );
	}
	stack.Push( name );
	text.Push( textParent ? "T" : "" );

	fprintf( fp, "<%s", name );
	elementJustOpened = true;
	++depth;
}


void XMLStreamer::PushAttribute( const char* name, const char* value )
{
	TIXMLASSERT( elementJustOpened );
	fprintf( fp, " %s=\"%s\"", name, value );
}


void XMLStreamer::CloseElement()
{
	--depth;
	const char* name = stack.Pop();
	int wasPositive = text.NumPositive();
	text.Pop();

	if ( elementJustOpened ) {
		fprintf( fp, "/>" );
		if ( text.NumPositive() == 0 ) {
			fprintf( fp, "\n" );
		}
	}
	else {
		if ( wasPositive == 0 ) {
			PrintSpace( depth );
		}
		fprintf( fp, "</%s>", name );
		if ( text.NumPositive() == 0 ) {
			fprintf( fp, "\n" );
		}
	}
	elementJustOpened = false;
}


void XMLStreamer::SealElement()
{
	elementJustOpened = false;
	fprintf( fp, ">" );
	if ( text.NumPositive() == 0 ) {
		fprintf( fp, "\n" );
	}
}


void XMLStreamer::PushText( const char* text )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	fprintf( fp, "%s", text );
}


void XMLStreamer::PushComment( const char* comment )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	PrintSpace( depth );
	fprintf( fp, "<!--%s-->\n", comment );
}
