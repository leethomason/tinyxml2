#include "tinyxml2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <new.h>
#include <stdarg.h>

//#pragma warning ( disable : 4291 )

using namespace tinyxml2;

static const char LINE_FEED				= (char)0x0a;			// all line endings are normalized to LF
static const char LF = LINE_FEED;
static const char CARRIAGE_RETURN		= (char)0x0d;			// CR gets filtered out
static const char CR = CARRIAGE_RETURN;
static const char SINGLE_QUOTE			= '\'';
static const char DOUBLE_QUOTE			= '\"';

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
//	ef bb bf (Microsoft "lead bytes") - designates UTF-8

static const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
static const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
static const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;


#define DELETE_NODE( node ) { MemPool* pool = node->memPool; node->~XMLNode(); pool->Free( node ); }
#define DELETE_ATTRIBUTE( attrib ) { MemPool* pool = attrib->memPool; attrib->~XMLAttribute(); pool->Free( attrib ); }

struct Entity {
	const char* pattern;
	int length;
	char value;
};

static const int NUM_ENTITIES = 5;
static const Entity entities[NUM_ENTITIES] = 
{
	{ "quot", 4,	DOUBLE_QUOTE },
	{ "amp", 3,		'&'  },
	{ "apos", 4,	SINGLE_QUOTE },
	{ "lt",	2, 		'<'	 },
	{ "gt",	2,		'>'	 }
};


StrPair::~StrPair()
{
	Reset();
}


void StrPair::Reset()
{
	if ( flags & NEEDS_DELETE ) {
		delete [] start;
	}
	flags = 0;
	start = 0;
	end = 0;
}


void StrPair::SetStr( const char* str, int flags )
{
	Reset();
	size_t len = strlen( str );
	start = new char[ len+1 ];
	memcpy( start, str, len+1 );
	end = start + len;
	this->flags = flags | NEEDS_DELETE;
}


char* StrPair::ParseText( char* p, const char* endTag, int strFlags )
{
	TIXMLASSERT( endTag && *endTag );

	char* start = p;	// fixme: hides a member
	char  endChar = *endTag;
	int   length = strlen( endTag );	

	// Inner loop of text parsing.
	while ( *p ) {
		if ( *p == endChar && strncmp( p, endTag, length ) == 0 ) {
			Set( start, p, strFlags );
			return p + length;
		}
		++p;
	}	
	return 0;
}


char* StrPair::ParseName( char* p )
{
	char* start = p;

	start = p;
	if ( !start || !(*start) ) {
		return 0;
	}

	if ( !XMLUtil::IsAlpha( *p ) ) {
		return 0;
	}

	while( *p && (
			   XMLUtil::IsAlphaNum( (unsigned char) *p ) 
			|| *p == '_'
			|| *p == '-'
			|| *p == '.'
			|| *p == ':' ))
	{
		++p;
	}

	if ( p > start ) {
		Set( start, p, 0 );
		return p;
	}
	return 0;
}



const char* StrPair::GetStr()
{
	if ( flags & NEEDS_FLUSH ) {
		*end = 0;
		flags ^= NEEDS_FLUSH;

		if ( flags ) {
			char* p = start;	// the read pointer
			char* q = start;	// the write pointer

			while( p < end ) {
				if ( (flags & NEEDS_NEWLINE_NORMALIZATION) && *p == CR ) {
					// CR-LF pair becomes LF
					// CR alone becomes LF
					// LF-CR becomes LF
					if ( *(p+1) == LF ) {
						p += 2;
					}
					else {
						++p;
					}
					*q++ = LF;
				}
				else if ( (flags & NEEDS_NEWLINE_NORMALIZATION) && *p == LF ) {
					if ( *(p+1) == CR ) {
						p += 2;
					}
					else {
						++p;
					}
					*q++ = LF;
				}
				else if ( (flags & NEEDS_ENTITY_PROCESSING) && *p == '&' ) {
					int i=0;

					// Entities handled by tinyXML2:
					// - special entities in the entity table [in/out]
					// - numeric character reference [in]
					//   &#20013; or &#x4e2d;

					if ( *(p+1) == '#' ) {
						char buf[10] = { 0 };
						int len;
						p = const_cast<char*>( XMLUtil::GetCharacterRef( p, buf, &len ) );
						for( int i=0; i<len; ++i ) {
							*q++ = buf[i];
						}
						TIXMLASSERT( q <= p );
					}
					else {
						for( i=0; i<NUM_ENTITIES; ++i ) {
							if (    strncmp( p+1, entities[i].pattern, entities[i].length ) == 0
								 && *(p+entities[i].length+1) == ';' ) 
							{
								// Found an entity convert;
								*q = entities[i].value;
								++q;
								p += entities[i].length + 2;
								break;
							}
						}
						if ( i == NUM_ENTITIES ) {
							// fixme: treat as error?
							++p;
							++q;
						}
					}
				}
				else {
					*q = *p;
					++p;
					++q;
				}
			}
			*q = 0;
		}
		flags = (flags & NEEDS_DELETE);
	}
	return start;
}




// --------- XMLUtil ----------- //

const char* XMLUtil::ReadBOM( const char* p, bool* bom )
{
	*bom = false;
	const unsigned char* pu = reinterpret_cast<const unsigned char*>(p);
	// Check for BOM:
	if (    *(pu+0) == TIXML_UTF_LEAD_0 
		 && *(pu+1) == TIXML_UTF_LEAD_1
		 && *(pu+2) == TIXML_UTF_LEAD_2 ) 
	{
		*bom = true;
		p += 3;
	}
	return p;
}


void XMLUtil::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
{
	const unsigned long BYTE_MASK = 0xBF;
	const unsigned long BYTE_MARK = 0x80;
	const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

	if (input < 0x80) 
		*length = 1;
	else if ( input < 0x800 )
		*length = 2;
	else if ( input < 0x10000 )
		*length = 3;
	else if ( input < 0x200000 )
		*length = 4;
	else
		{ *length = 0; return; }	// This code won't covert this correctly anyway.

	output += *length;

	// Scary scary fall throughs.
	switch (*length) 
	{
		case 4:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 3:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 2:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 1:
			--output; 
			*output = (char)(input | FIRST_BYTE_MARK[*length]);
	}
}


const char* XMLUtil::GetCharacterRef( const char* p, char* value, int* length )
{
	// Presume an entity, and pull it out.
	*length = 0;

	if ( *(p+1) == '#' && *(p+2) )
	{
		unsigned long ucs = 0;
		ptrdiff_t delta = 0;
		unsigned mult = 1;

		if ( *(p+2) == 'x' )
		{
			// Hexadecimal.
			if ( !*(p+3) ) return 0;

			const char* q = p+3;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != 'x' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else if ( *q >= 'a' && *q <= 'f' )
					ucs += mult * (*q - 'a' + 10);
				else if ( *q >= 'A' && *q <= 'F' )
					ucs += mult * (*q - 'A' + 10 );
				else 
					return 0;
				mult *= 16;
				--q;
			}
		}
		else
		{
			// Decimal.
			if ( !*(p+2) ) return 0;

			const char* q = p+2;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != '#' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else 
					return 0;
				mult *= 10;
				--q;
			}
		}
		// convert the UCS to UTF-8
		ConvertUTF32ToUTF8( ucs, value, length );
		return p + delta + 1;
	}
	return p+1;
}


char* XMLDocument::Identify( char* p, XMLNode** node ) 
{
	XMLNode* returnNode = 0;
	char* start = p;
	p = XMLUtil::SkipWhiteSpace( p );
	if( !p || !*p )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?
	// - Everthing else is unknown to tinyxml.
	//

	static const char* xmlHeader		= { "<?" };
	static const char* commentHeader	= { "<!--" };
	static const char* dtdHeader		= { "<!" };
	static const char* cdataHeader		= { "<![CDATA[" };
	static const char* elementHeader	= { "<" };	// and a header for everything else; check last.

	static const int xmlHeaderLen		= 2;
	static const int commentHeaderLen	= 4;
	static const int dtdHeaderLen		= 2;
	static const int cdataHeaderLen		= 9;
	static const int elementHeaderLen	= 1;

	TIXMLASSERT( sizeof( XMLComment ) == sizeof( XMLUnknown ) );		// use same memory pool
	TIXMLASSERT( sizeof( XMLComment ) == sizeof( XMLDeclaration ) );	// use same memory pool

	if ( XMLUtil::StringEqual( p, xmlHeader, xmlHeaderLen ) ) {
		returnNode = new (commentPool.Alloc()) XMLDeclaration( this );
		returnNode->memPool = &commentPool;
		p += xmlHeaderLen;
	}
	else if ( XMLUtil::StringEqual( p, commentHeader, commentHeaderLen ) ) {
		returnNode = new (commentPool.Alloc()) XMLComment( this );
		returnNode->memPool = &commentPool;
		p += commentHeaderLen;
	}
	else if ( XMLUtil::StringEqual( p, cdataHeader, cdataHeaderLen ) ) {
		XMLText* text = new (textPool.Alloc()) XMLText( this );
		returnNode = text;
		returnNode->memPool = &textPool;
		p += cdataHeaderLen;
		text->SetCData( true );
	}
	else if ( XMLUtil::StringEqual( p, dtdHeader, dtdHeaderLen ) ) {
		returnNode = new (commentPool.Alloc()) XMLUnknown( this );
		returnNode->memPool = &commentPool;
		p += dtdHeaderLen;
	}
	else if ( XMLUtil::StringEqual( p, elementHeader, elementHeaderLen ) ) {
		returnNode = new (elementPool.Alloc()) XMLElement( this );
		returnNode->memPool = &elementPool;
		p += elementHeaderLen;
	}
	else {
		returnNode = new (textPool.Alloc()) XMLText( this );
		returnNode->memPool = &textPool;
		p = start;	// Back it up, all the text counts.
	}

	*node = returnNode;
	return p;
}


bool XMLDocument::Accept( XMLVisitor* visitor ) const
{
	if ( visitor->VisitEnter( *this ) )
	{
		for ( const XMLNode* node=FirstChild(); node; node=node->NextSibling() )
		{
			if ( !node->Accept( visitor ) )
				break;
		}
	}
	return visitor->VisitExit( *this );
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
	ClearChildren();
	if ( parent ) {
		parent->Unlink( this );
	}
}


void XMLNode::SetValue( const char* str, bool staticMem )
{
	if ( staticMem )
		value.SetInternedStr( str );
	else
		value.SetStr( str );
}


void XMLNode::ClearChildren()
{
	while( firstChild ) {
		XMLNode* node = firstChild;
		Unlink( node );
		
		DELETE_NODE( node );
	}
	firstChild = lastChild = 0;
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


void XMLNode::DeleteChild( XMLNode* node )
{
	TIXMLASSERT( node->parent == this );
	DELETE_NODE( node );
}


XMLNode* XMLNode::InsertEndChild( XMLNode* addThis )
{
	if ( lastChild ) {
		TIXMLASSERT( firstChild );
		TIXMLASSERT( lastChild->next == 0 );
		lastChild->next = addThis;
		addThis->prev = lastChild;
		lastChild = addThis;

		addThis->next = 0;
	}
	else {
		TIXMLASSERT( firstChild == 0 );
		firstChild = lastChild = addThis;

		addThis->prev = 0;
		addThis->next = 0;
	}
	addThis->parent = this;
	return addThis;
}


XMLNode* XMLNode::InsertFirstChild( XMLNode* addThis )
{
	if ( firstChild ) {
		TIXMLASSERT( lastChild );
		TIXMLASSERT( firstChild->prev == 0 );

		firstChild->prev = addThis;
		addThis->next = firstChild;
		firstChild = addThis;

		addThis->prev = 0;
	}
	else {
		TIXMLASSERT( lastChild == 0 );
		firstChild = lastChild = addThis;

		addThis->prev = 0;
		addThis->next = 0;
	}
	addThis->parent = this;
	return addThis;
}


XMLNode* XMLNode::InsertAfterChild( XMLNode* afterThis, XMLNode* addThis )
{
	TIXMLASSERT( afterThis->parent == this );
	if ( afterThis->parent != this )
		return 0;

	if ( afterThis->next == 0 ) {
		// The last node or the only node.
		return InsertEndChild( addThis );
	}
	addThis->prev = afterThis;
	addThis->next = afterThis->next;
	afterThis->next->prev = addThis;
	afterThis->next = addThis;
	addThis->parent = this;
	return addThis;
}




const XMLElement* XMLNode::FirstChildElement( const char* value ) const
{
	for( XMLNode* node=firstChild; node; node=node->next ) {
		XMLElement* element = node->ToElement();
		if ( element ) {
			if ( !value || XMLUtil::StringEqual( element->Name(), value ) ) {
				return element;
			}
		}
	}
	return 0;
}


const XMLElement* XMLNode::LastChildElement( const char* value ) const
{
	for( XMLNode* node=lastChild; node; node=node->prev ) {
		XMLElement* element = node->ToElement();
		if ( element ) {
			if ( !value || XMLUtil::StringEqual( element->Name(), value ) ) {
				return element;
			}
		}
	}
	return 0;
}


char* XMLNode::ParseDeep( char* p )
{
	while( p && *p ) {
		XMLNode* node = 0;
		p = document->Identify( p, &node );
		if ( p && node ) {
			p = node->ParseDeep( p );

			if ( node->IsClosingElement() ) {
				if ( !XMLUtil::StringEqual( Value(), node->Value() )) {
					document->SetError( ERROR_MISMATCHED_ELEMENT, Value(), 0 );
				}
				DELETE_NODE( node );
				return p;
			}
			this->InsertEndChild( node );
		}
	}
	return 0;
}

// --------- XMLText ---------- //
char* XMLText::ParseDeep( char* p )
{
	const char* start = p;
	if ( this->CData() ) {
		p = value.ParseText( p, "]]>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
		if ( !p ) {
			document->SetError( ERROR_PARSING_CDATA, start, 0 );
		}
		return p;
	}
	else {
		p = value.ParseText( p, "<", StrPair::TEXT_ELEMENT );
		if ( !p ) {
			document->SetError( ERROR_PARSING_TEXT, start, 0 );
		}
		if ( p && *p ) {
			return p-1;
		}
	}
	return 0;
}


bool XMLText::Accept( XMLVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


// --------- XMLComment ---------- //

XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLComment::~XMLComment()
{
	//printf( "~XMLComment\n" );
}


char* XMLComment::ParseDeep( char* p )
{
	// Comment parses as text.
	const char* start = p;
	p = value.ParseText( p, "-->", StrPair::COMMENT );
	if ( p == 0 ) {
		document->SetError( ERROR_PARSING_COMMENT, start, 0 );
	}
	return p;
}


bool XMLComment::Accept( XMLVisitor* visitor ) const
{
	return visitor->Visit( *this );
}


// --------- XMLDeclaration ---------- //

XMLDeclaration::XMLDeclaration( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLDeclaration::~XMLDeclaration()
{
	//printf( "~XMLDeclaration\n" );
}


char* XMLDeclaration::ParseDeep( char* p )
{
	// Declaration parses as text.
	const char* start = p;
	p = value.ParseText( p, "?>", StrPair::NEEDS_NEWLINE_NORMALIZATION );
	if ( p == 0 ) {
		document->SetError( ERROR_PARSING_DECLARATION, start, 0 );
	}
	return p;
}


bool XMLDeclaration::Accept( XMLVisitor* visitor ) const
{
	return visitor->Visit( *this );
}

// --------- XMLUnknown ---------- //

XMLUnknown::XMLUnknown( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLUnknown::~XMLUnknown()
{
}


char* XMLUnknown::ParseDeep( char* p )
{
	// Unknown parses as text.
	const char* start = p;

	p = value.ParseText( p, ">", StrPair::NEEDS_NEWLINE_NORMALIZATION );
	if ( !p ) {
		document->SetError( ERROR_PARSING_UNKNOWN, start, 0 );
	}
	return p;
}


bool XMLUnknown::Accept( XMLVisitor* visitor ) const
{
	return visitor->Visit( *this );
}

// --------- XMLAttribute ---------- //
char* XMLAttribute::ParseDeep( char* p )
{
	p = name.ParseText( p, "=", StrPair::ATTRIBUTE_NAME );
	if ( !p || !*p ) return 0;

	char endTag[2] = { *p, 0 };
	++p;
	p = value.ParseText( p, endTag, StrPair::ATTRIBUTE_VALUE );
	if ( value.Empty() ) return 0;
	return p;
}


void XMLAttribute::SetName( const char* n )
{
	name.SetStr( n );
}


int XMLAttribute::QueryIntAttribute( int* value ) const
{
	if ( TIXML_SSCANF( Value(), "%d", value ) == 1 )
		return XML_NO_ERROR;
	return WRONG_ATTRIBUTE_TYPE;
}


int XMLAttribute::QueryUnsignedAttribute( unsigned int* value ) const
{
	if ( TIXML_SSCANF( Value(), "%u", value ) == 1 )
		return XML_NO_ERROR;
	return WRONG_ATTRIBUTE_TYPE;
}


int XMLAttribute::QueryBoolAttribute( bool* value ) const
{
	int ival = -1;
	QueryIntAttribute( &ival );

	if ( ival > 0 || XMLUtil::StringEqual( Value(), "true" ) ) {
		*value = true;
		return XML_NO_ERROR;
	}
	else if ( ival == 0 || XMLUtil::StringEqual( Value(), "false" ) ) {
		*value = false;
		return XML_NO_ERROR;
	}
	return WRONG_ATTRIBUTE_TYPE;
}


int XMLAttribute::QueryDoubleAttribute( double* value ) const
{
	if ( TIXML_SSCANF( Value(), "%lf", value ) == 1 )
		return XML_NO_ERROR;
	return WRONG_ATTRIBUTE_TYPE;
}


int XMLAttribute::QueryFloatAttribute( float* value ) const
{
	if ( TIXML_SSCANF( Value(), "%f", value ) == 1 )
		return XML_NO_ERROR;
	return WRONG_ATTRIBUTE_TYPE;
}


void XMLAttribute::SetAttribute( const char* v )
{
	value.SetStr( v );
}


void XMLAttribute::SetAttribute( int v )
{
	char buf[BUF_SIZE];
	TIXML_SNPRINTF( buf, BUF_SIZE-1, "%d", v );	
	value.SetStr( buf );
}


void XMLAttribute::SetAttribute( unsigned v )
{
	char buf[BUF_SIZE];
	TIXML_SNPRINTF( buf, BUF_SIZE-1, "%u", v );	
	value.SetStr( buf );
}


void XMLAttribute::SetAttribute( bool v )
{
	char buf[BUF_SIZE];
	TIXML_SNPRINTF( buf, BUF_SIZE-1, "%d", v ? 1 : 0 );	
	value.SetStr( buf );
}

void XMLAttribute::SetAttribute( double v )
{
	char buf[BUF_SIZE];
	TIXML_SNPRINTF( buf, BUF_SIZE-1, "%f", v );	
	value.SetStr( buf );
}

void XMLAttribute::SetAttribute( float v )
{
	char buf[BUF_SIZE];
	TIXML_SNPRINTF( buf, BUF_SIZE-1, "%f", v );	
	value.SetStr( buf );
}


// --------- XMLElement ---------- //
XMLElement::XMLElement( XMLDocument* doc ) : XMLNode( doc ),
	closing( false ),
	rootAttribute( 0 )
	//lastAttribute( 0 )
{
}


XMLElement::~XMLElement()
{
	while( rootAttribute ) {
		XMLAttribute* next = rootAttribute->next;
		DELETE_ATTRIBUTE( rootAttribute );
		rootAttribute = next;
	}
}


XMLAttribute* XMLElement::FindAttribute( const char* name )
{
	XMLAttribute* a = 0;
	for( a=rootAttribute; a; a = a->next ) {
		if ( XMLUtil::StringEqual( a->Name(), name ) )
			return a;
	}
	return 0;
}


const XMLAttribute* XMLElement::FindAttribute( const char* name ) const
{
	XMLAttribute* a = 0;
	for( a=rootAttribute; a; a = a->next ) {
		if ( XMLUtil::StringEqual( a->Name(), name ) )
			return a;
	}
	return 0;
}


const char* XMLElement::GetText() const
{
	if ( FirstChild() && FirstChild()->ToText() ) {
		return FirstChild()->ToText()->Value();
	}
	return 0;
}



XMLAttribute* XMLElement::FindOrCreateAttribute( const char* name )
{
	XMLAttribute* attrib = FindAttribute( name );
	if ( !attrib ) {
		attrib = new (document->attributePool.Alloc() ) XMLAttribute( this );
		attrib->memPool = &document->attributePool;
		LinkAttribute( attrib );
		attrib->SetName( name );
	}
	return attrib;
}


void XMLElement::LinkAttribute( XMLAttribute* attrib )
{
	if ( rootAttribute ) {
		XMLAttribute* end = rootAttribute;
		while ( end->next )
			end = end->next;
		end->next = attrib;
	}
	else {
		rootAttribute = attrib;
	}
}


void XMLElement::DeleteAttribute( const char* name )
{
	XMLAttribute* prev = 0;
	for( XMLAttribute* a=rootAttribute; a; a=a->next ) {
		if ( XMLUtil::StringEqual( name, a->Name() ) ) {
			if ( prev ) {
				prev->next = a->next;
			}
			else {
				rootAttribute = a->next;
			}
			DELETE_ATTRIBUTE( a );
			break;
		}
		prev = a;
	}
}


char* XMLElement::ParseAttributes( char* p, bool* closedElement )
{
	const char* start = p;
	*closedElement = false;

	// Read the attributes.
	while( p ) {
		p = XMLUtil::SkipWhiteSpace( p );
		if ( !p || !(*p) ) {
			document->SetError( ERROR_PARSING_ELEMENT, start, Name() );
			return 0;
		}

		// attribute.
		if ( XMLUtil::IsAlpha( *p ) ) {
			XMLAttribute* attrib = new (document->attributePool.Alloc() ) XMLAttribute( this );
			attrib->memPool = &document->attributePool;

			p = attrib->ParseDeep( p );
			if ( !p || Attribute( attrib->Name() ) ) {
				DELETE_ATTRIBUTE( attrib );
				document->SetError( ERROR_PARSING_ATTRIBUTE, start, p );
				return 0;
			}
			LinkAttribute( attrib );
		}
		// end of the tag
		else if ( *p == '/' && *(p+1) == '>' ) {
			if ( closing ) {
				document->SetError( ERROR_PARSING_ELEMENT, start, p );
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
			document->SetError( ERROR_PARSING_ELEMENT, start, p );
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
	p = XMLUtil::SkipWhiteSpace( p );
	if ( !p ) return 0;
	const char* start = p;

	// The closing element is the </element> form. It is
	// parsed just like a regular element then deleted from
	// the DOM.
	if ( *p == '/' ) {
		closing = true;
		++p;
	}

	p = value.ParseName( p );
	if ( value.Empty() ) return 0;

	bool elementClosed=false;
	p = ParseAttributes( p, &elementClosed );
	if ( !p || !*p || elementClosed || closing ) 
		return p;

	p = XMLNode::ParseDeep( p );
	return p;
}


bool XMLElement::Accept( XMLVisitor* visitor ) const
{
	if ( visitor->VisitEnter( *this, rootAttribute ) ) 
	{
		for ( const XMLNode* node=FirstChild(); node; node=node->NextSibling() )
		{
			if ( !node->Accept( visitor ) )
				break;
		}
	}
	return visitor->VisitExit( *this );

}

// --------- XMLDocument ----------- //
XMLDocument::XMLDocument() :
	XMLNode( 0 ),
	writeBOM( false ),
	charBuffer( 0 )
{
	document = this;	// avoid warning about 'this' in initializer list
}


XMLDocument::~XMLDocument()
{
	ClearChildren();
	delete [] charBuffer;

#if 0
	textPool.Trace( "text" );
	elementPool.Trace( "element" );
	commentPool.Trace( "comment" );
	attributePool.Trace( "attribute" );
#endif

	TIXMLASSERT( textPool.CurrentAllocs() == 0 );
	TIXMLASSERT( elementPool.CurrentAllocs() == 0 );
	TIXMLASSERT( commentPool.CurrentAllocs() == 0 );
	TIXMLASSERT( attributePool.CurrentAllocs() == 0 );
}


void XMLDocument::InitDocument()
{
	errorID = XML_NO_ERROR;
	errorStr1 = 0;
	errorStr2 = 0;

	delete [] charBuffer;
	charBuffer = 0;

}


XMLElement* XMLDocument::NewElement( const char* name )
{
	XMLElement* ele = new (elementPool.Alloc()) XMLElement( this );
	ele->memPool = &elementPool;
	ele->SetName( name );
	return ele;
}


XMLComment* XMLDocument::NewComment( const char* str )
{
	XMLComment* comment = new (commentPool.Alloc()) XMLComment( this );
	comment->memPool = &commentPool;
	comment->SetValue( str );
	return comment;
}


XMLText* XMLDocument::NewText( const char* str )
{
	XMLText* text = new (textPool.Alloc()) XMLText( this );
	text->memPool = &textPool;
	text->SetValue( str );
	return text;
}


int XMLDocument::LoadFile( const char* filename )
{
	ClearChildren();
	InitDocument();

	FILE* fp = fopen( filename, "rb" );
	if ( !fp ) {
		SetError( ERROR_FILE_NOT_FOUND, filename, 0 );
		return errorID;
	}
	LoadFile( fp );
	fclose( fp );
	return errorID;
}


int XMLDocument::LoadFile( FILE* fp ) 
{
	ClearChildren();
	InitDocument();

	fseek( fp, 0, SEEK_END );
	unsigned size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	if ( size == 0 ) {
		return errorID;
	}

	charBuffer = new char[size+1];
	fread( charBuffer, size, 1, fp );
	charBuffer[size] = 0;

	const char* p = charBuffer;
	p = XMLUtil::SkipWhiteSpace( p );
	p = XMLUtil::ReadBOM( p, &writeBOM );
	if ( !p || !*p ) {
		SetError( ERROR_EMPTY_DOCUMENT, 0, 0 );
		return errorID;
	}

	ParseDeep( charBuffer + (p-charBuffer) );
	return errorID;
}


void XMLDocument::SaveFile( const char* filename )
{
	FILE* fp = fopen( filename, "w" );
	XMLStreamer stream( fp );
	Print( &stream );
	fclose( fp );
}


int XMLDocument::Parse( const char* p )
{
	ClearChildren();
	InitDocument();

	if ( !p || !*p ) {
		SetError( ERROR_EMPTY_DOCUMENT, 0, 0 );
		return errorID;
	}
	p = XMLUtil::SkipWhiteSpace( p );
	p = XMLUtil::ReadBOM( p, &writeBOM );
	if ( !p || !*p ) {
		SetError( ERROR_EMPTY_DOCUMENT, 0, 0 );
		return errorID;
	}

	size_t len = strlen( p );
	charBuffer = new char[ len+1 ];
	memcpy( charBuffer, p, len+1 );

	
	ParseDeep( charBuffer );
	return errorID;
}


void XMLDocument::Print( XMLStreamer* streamer ) 
{
	XMLStreamer stdStreamer( stdout );
	if ( !streamer )
		streamer = &stdStreamer;
	Accept( streamer );
}


void XMLDocument::SetError( int error, const char* str1, const char* str2 )
{
	errorID = error;
	errorStr1 = str1;
	errorStr2 = str2;
}


void XMLDocument::PrintError() const 
{
	if ( errorID ) {
		char buf1[20] = { 0 };
		char buf2[20] = { 0 };
		
		if ( errorStr1 ) {
			strncpy( buf1, errorStr1, 20 );
			buf1[19] = 0;
		}
		if ( errorStr2 ) {
			strncpy( buf2, errorStr2, 20 );
			buf2[19] = 0;
		}

		printf( "XMLDocument error id=%d str1=%s str2=%s\n",
			    errorID, buf1, buf2 );
	}
}


XMLStreamer::XMLStreamer( FILE* file ) : 
	elementJustOpened( false ), 
	firstElement( true ),
	fp( file ), 
	depth( 0 ), 
	textDepth( -1 )
{
	for( int i=0; i<ENTITY_RANGE; ++i ) {
		entityFlag[i] = false;
		restrictedEntityFlag[i] = false;
	}
	for( int i=0; i<NUM_ENTITIES; ++i ) {
		TIXMLASSERT( entities[i].value < ENTITY_RANGE );
		if ( entities[i].value < ENTITY_RANGE ) {
			entityFlag[ entities[i].value ] = true;
		}
	}
	restrictedEntityFlag['&'] = true;
	restrictedEntityFlag['<'] = true;
	restrictedEntityFlag['>'] = true;	// not required, but consistency is nice
	buffer.Push( 0 );
}


void XMLStreamer::Print( const char* format, ... )
{
    va_list     va;
    va_start( va, format );

	if ( fp ) {
		vfprintf( fp, format, va );
	}
	else {
		// This seems brutally complex. Haven't figured out a better
		// way on windows.
		#ifdef _MSC_VER
			int len = -1;
			int expand = 1000;
			while ( len < 0 ) {
				len = vsnprintf_s( accumulator.Mem(), accumulator.Capacity(), accumulator.Capacity()-1, format, va );
				if ( len < 0 ) {
					accumulator.PushArr( expand );
					expand *= 3/2;
				}
			}
			char* p = buffer.PushArr( len ) - 1;
			memcpy( p, accumulator.Mem(), len+1 );
		#else
			int len = vsnprintf( 0, 0, format, va );
			char* p = buffer.PushArr( len ) - 1;
			vsprintf_s( p, len+1, format, va );
		#endif
	}
    va_end( va );
}


void XMLStreamer::PrintSpace( int depth )
{
	for( int i=0; i<depth; ++i ) {
		Print( "    " );
	}
}


void XMLStreamer::PrintString( const char* p, bool restricted )
{
	// Look for runs of bytes between entities to print.
	const char* q = p;
	const bool* flag = restricted ? restrictedEntityFlag : entityFlag;

	while ( *q ) {
		// Remember, char is sometimes signed. (How many times has that bitten me?)
		if ( *q > 0 && *q < ENTITY_RANGE ) {
			// Check for entities. If one is found, flush
			// the stream up until the entity, write the 
			// entity, and keep looking.
			if ( flag[*q] ) {
				while ( p < q ) {
					Print( "%c", *p );
					++p;
				}
				for( int i=0; i<NUM_ENTITIES; ++i ) {
					if ( entities[i].value == *q ) {
						Print( "&%s;", entities[i].pattern );
						break;
					}
				}
				++p;
			}
		}
		++q;
	}
	// Flush the remaining string. This will be the entire
	// string if an entity wasn't found.
	if ( q-p > 0 ) {
		Print( "%s", p );
	}
}


void XMLStreamer::PushHeader( bool writeBOM, bool writeDec )
{
	static const unsigned char bom[] = { TIXML_UTF_LEAD_0, TIXML_UTF_LEAD_1, TIXML_UTF_LEAD_2, 0 };
	if ( writeBOM ) {
		Print( "%s", bom );
	}
	if ( writeDec ) {
		PushDeclaration( "xml version=\"1.0\"" );
	}
}


void XMLStreamer::OpenElement( const char* name )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	stack.Push( name );

	if ( textDepth < 0 && !firstElement ) {
		Print( "\n" );
		PrintSpace( depth );
	}

	Print( "<%s", name );
	elementJustOpened = true;
	firstElement = false;
	++depth;
}


void XMLStreamer::PushAttribute( const char* name, const char* value )
{
	TIXMLASSERT( elementJustOpened );
	Print( " %s=\"", name );
	PrintString( value, false );
	Print( "\"" );
}


void XMLStreamer::CloseElement()
{
	--depth;
	const char* name = stack.Pop();

	if ( elementJustOpened ) {
		Print( "/>" );
	}
	else {
		if ( textDepth < 0 ) {
			Print( "\n" );
			PrintSpace( depth );
		}
		Print( "</%s>", name );
	}

	if ( textDepth == depth )
		textDepth = -1;
	if ( depth == 0 )
		Print( "\n" );
	elementJustOpened = false;
}


void XMLStreamer::SealElement()
{
	elementJustOpened = false;
	Print( ">" );
}


void XMLStreamer::PushText( const char* text, bool cdata )
{
	textDepth = depth-1;

	if ( elementJustOpened ) {
		SealElement();
	}
	if ( cdata ) {
		Print( "<![CDATA[" );
		Print( "%s", text );
		Print( "]]>" );
	}
	else {
		PrintString( text, true );
	}
}


void XMLStreamer::PushComment( const char* comment )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	if ( textDepth < 0 && !firstElement ) {
		Print( "\n" );
		PrintSpace( depth );
	}
	firstElement = false;
	Print( "<!--%s-->", comment );
}


void XMLStreamer::PushDeclaration( const char* value )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	if ( textDepth < 0 && !firstElement) {
		Print( "\n" );
		PrintSpace( depth );
	}
	firstElement = false;
	Print( "<?%s?>", value );
}


void XMLStreamer::PushUnknown( const char* value )
{
	if ( elementJustOpened ) {
		SealElement();
	}
	if ( textDepth < 0 && !firstElement ) {
		Print( "\n" );
		PrintSpace( depth );
	}
	firstElement = false;
	Print( "<!%s>", value );
}


bool XMLStreamer::VisitEnter( const XMLDocument& doc )
{
	if ( doc.HasBOM() ) {
		PushHeader( true, false );
	}
	return true;
}


bool XMLStreamer::VisitEnter( const XMLElement& element, const XMLAttribute* attribute )
{
	OpenElement( element.Name() );
	while ( attribute ) {
		PushAttribute( attribute->Name(), attribute->Value() );
		attribute = attribute->Next();
	}
	return true;
}


bool XMLStreamer::VisitExit( const XMLElement& element )
{
	CloseElement();
	return true;
}


bool XMLStreamer::Visit( const XMLText& text )
{
	PushText( text.Value(), text.CData() );
	return true;
}


bool XMLStreamer::Visit( const XMLComment& comment )
{
	PushComment( comment.Value() );
	return true;
}

bool XMLStreamer::Visit( const XMLDeclaration& declaration )
{
	PushDeclaration( declaration.Value() );
	return true;
}


bool XMLStreamer::Visit( const XMLUnknown& unknown )
{
	PushUnknown( unknown.Value() );
	return true;
}


