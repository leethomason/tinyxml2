#include "tinyxml2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined( WIN32 )
	#include <crtdbg.h>
	_CrtMemState startMemState;
	_CrtMemState endMemState;
#endif

using namespace tinyxml2;
int gPass = 0;
int gFail = 0;

//#define DREAM_ONLY

/*
int gNew = 0;
int gNewTotal = 0;

void* operator new( size_t size ) 
{
	++gNew;
	return malloc( size );
}

void* operator new[]( size_t size ) 
{
	++gNew;
	return malloc( size );
}

void operator delete[]( void* mem ) 
{
	free( mem );
}

void operator delete( void* mem ) 
{
	free( mem );
}
*/


bool XMLTest (const char* testString, const char* expected, const char* found, bool echo=true )
{
	bool pass = !strcmp( expected, found );
	if ( pass )
		printf ("[pass]");
	else
		printf ("[fail]");

	if ( !echo )
		printf (" %s\n", testString);
	else
		printf (" %s [%s][%s]\n", testString, expected, found);

	if ( pass )
		++gPass;
	else
		++gFail;
	return pass;
}


bool XMLTest( const char* testString, int expected, int found, bool echo=true )
{
	bool pass = ( expected == found );
	if ( pass )
		printf ("[pass]");
	else
		printf ("[fail]");

	if ( !echo )
		printf (" %s\n", testString);
	else
		printf (" %s [%d][%d]\n", testString, expected, found);

	if ( pass )
		++gPass;
	else
		++gFail;
	return pass;
}


void NullLineEndings( char* p )
{
	while( p && *p ) {
		if ( *p == '\n' || *p == '\r' ) {
			*p = 0;
			return;
		}
		++p;
	}
}


int main( int argc, const char* argv )
{
	#if defined( WIN32 )
		_CrtMemCheckpoint( &startMemState );
	#endif	
#ifndef DREAM_ONLY
#if 0 
	{
		static const char* test = "<!--hello world\n"
			                      "          line 2\r"
			                      "          line 3\r\n"
			                      "          line 4\n\r"
			                      "          line 5\r-->";

		XMLDocument doc;
		doc.Parse( test );
		doc.Print();
	}
#endif
#if 0
	{
		static const char* test[] = {	"<element />",
									    "<element></element>",
										"<element><subelement/></element>",
									    "<element><subelement></subelement></element>",
									    "<element><subelement><subsub/></subelement></element>",
									    "<!--comment beside elements--><element><subelement></subelement></element>",
									    "<!--comment beside elements, this time with spaces-->  \n <element>  <subelement> \n </subelement> </element>",
									    "<element attrib1='foo' attrib2=\"bar\" ></element>",
									    "<element attrib1='foo' attrib2=\"bar\" ><subelement attrib3='yeehaa' /></element>",
										"<element>Text inside element.</element>",
										"<element><b></b></element>",
										"<element>Text inside and <b>bolded</b> in the element.</element>",
										"<outer><element>Text inside and <b>bolded</b> in the element.</element></outer>",
										"<element>This &amp; That.</element>",
										"<element attrib='This&lt;That' />",
										0
		};
		for( int i=0; test[i]; ++i ) {
			XMLDocument doc;
			doc.Parse( test[i] );
			doc.Print();
			printf( "----------------------------------------------\n" );
		}
	}
#endif
#if 0
	{
		static const char* test = "<element>Text before.</element>";
		XMLDocument doc;
		doc.Parse( test );
		XMLElement* root = doc.FirstChildElement();
		XMLElement* newElement = doc.NewElement( "Subelement" );
		root->InsertEndChild( newElement );
		doc.Print();
	}
	{
		XMLDocument* doc = new XMLDocument();
		static const char* test = "<element><sub/></element>";
		doc->Parse( test );
		delete doc;
	}
#endif
	{
#if 0
		// Test: Programmatic DOM
		// Build:
		//		<element>
		//			<!--comment-->
		//			<sub attrib="1" />
		//			<sub attrib="2" />
		//			<sub attrib="3" >& Text!</sub>
		//		<element>

		XMLDocument* doc = new XMLDocument();
		XMLNode* element = doc->InsertEndChild( doc->NewElement( "element" ) );

		XMLElement* sub[3] = { doc->NewElement( "sub" ), doc->NewElement( "sub" ), doc->NewElement( "sub" ) };
		for( int i=0; i<3; ++i ) {
			sub[i]->SetAttribute( "attrib", i );
		}
		element->InsertEndChild( sub[2] );
		XMLNode* comment = element->InsertFirstChild( doc->NewComment( "comment" ) );
		element->InsertAfterChild( comment, sub[0] );
		element->InsertAfterChild( sub[0], sub[1] );
		sub[2]->InsertFirstChild( doc->NewText( "& Text!" ));
		doc->Print();
		XMLTest( "Programmatic DOM", "comment", doc->FirstChildElement( "element" )->FirstChild()->Value() );
		XMLTest( "Programmatic DOM", "0", doc->FirstChildElement( "element" )->FirstChildElement()->Attribute( "attrib" ) );
		XMLTest( "Programmatic DOM", 2, doc->FirstChildElement()->LastChildElement( "sub" )->IntAttribute( "attrib" ) );
		XMLTest( "Programmatic DOM", "& Text!", 
				 doc->FirstChildElement()->LastChildElement( "sub" )->FirstChild()->ToText()->Value() );

		// And now deletion:
		element->DeleteChild( sub[2] );
		doc->DeleteNode( comment );

		element->FirstChildElement()->SetAttribute( "attrib", true );
		element->LastChildElement()->DeleteAttribute( "attrib" );

		XMLTest( "Programmatic DOM", true, doc->FirstChildElement()->FirstChildElement()->BoolAttribute( "attrib" ) );
		int value = 10;
		int result = doc->FirstChildElement()->LastChildElement()->QueryIntAttribute( "attrib", &value );
		XMLTest( "Programmatic DOM", result, NO_ATTRIBUTE );
		XMLTest( "Programmatic DOM", value, 10 );

		doc->Print();

		XMLStreamer streamer;
		doc->Print( &streamer );
		printf( "%s", streamer.CStr() );

		delete doc;
#endif
	}
#endif
	{
#if 0 
		// Test: Dream
		// XML1 : 1,187,569 bytes	in 31,209 allocations
		// XML2 :   469,073	bytes	in    323 allocations
		//int newStart = gNew;
		XMLDocument doc;
		doc.LoadFile( "dream.xml" );

		doc.SaveFile( "dreamout.xml" );
		doc.PrintError();

		XMLTest( "Dream", "xml version=\"1.0\"",
			              doc.FirstChild()->ToDeclaration()->Value() );
		XMLTest( "Dream", true, doc.FirstChild()->NextSibling()->ToUnknown() ? true : false );
		XMLTest( "Dream", "DOCTYPE PLAY SYSTEM \"play.dtd\"",
						  doc.FirstChild()->NextSibling()->ToUnknown()->Value() );
		XMLTest( "Dream", "And Robin shall restore amends.",
			              doc.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );
		XMLTest( "Dream", "And Robin shall restore amends.",
			              doc.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );

		XMLDocument doc2;
		doc2.LoadFile( "dreamout.xml" );
		XMLTest( "Dream-out", "xml version=\"1.0\"",
			              doc2.FirstChild()->ToDeclaration()->Value() );
		XMLTest( "Dream-out", true, doc2.FirstChild()->NextSibling()->ToUnknown() ? true : false );
		XMLTest( "Dream-out", "DOCTYPE PLAY SYSTEM \"play.dtd\"",
						  doc2.FirstChild()->NextSibling()->ToUnknown()->Value() );
		XMLTest( "Dream-out", "And Robin shall restore amends.",
			              doc2.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );

#endif
		//gNewTotal = gNew - newStart;
	}

#if 0
	{
		const char* error =	"<?xml version=\"1.0\" standalone=\"no\" ?>\n"
							"<passages count=\"006\" formatversion=\"20020620\">\n"
							"    <wrong error>\n"
							"</passages>";

		XMLDocument doc;
		doc.Parse( error );
		XMLTest( "Bad XML", doc.ErrorID(), ERROR_PARSING_ATTRIBUTE );
	}

	{
		const char* str = "<doc attr0='1' attr1='2.0' attr2='foo' />";

		XMLDocument doc;
		doc.Parse( str );

		XMLElement* ele = doc.FirstChildElement();

		int iVal, result;
		double dVal;

		result = ele->QueryDoubleAttribute( "attr0", &dVal );
		XMLTest( "Query attribute: int as double", result, XML_NO_ERROR );
		XMLTest( "Query attribute: int as double", (int)dVal, 1 );
		result = ele->QueryDoubleAttribute( "attr1", &dVal );
		XMLTest( "Query attribute: double as double", (int)dVal, 2 );
		result = ele->QueryIntAttribute( "attr1", &iVal );
		XMLTest( "Query attribute: double as int", result, XML_NO_ERROR );
		XMLTest( "Query attribute: double as int", iVal, 2 );
		result = ele->QueryIntAttribute( "attr2", &iVal );
		XMLTest( "Query attribute: not a number", result, WRONG_ATTRIBUTE_TYPE );
		result = ele->QueryIntAttribute( "bar", &iVal );
		XMLTest( "Query attribute: does not exist", result, NO_ATTRIBUTE );
	}

	{
		const char* str = "<doc/>";

		XMLDocument doc;
		doc.Parse( str );

		XMLElement* ele = doc.FirstChildElement();

		int iVal;
		double dVal;

		ele->SetAttribute( "str", "strValue" );
		ele->SetAttribute( "int", 1 );
		ele->SetAttribute( "double", -1.0 );

		const char* cStr = ele->Attribute( "str" );
		ele->QueryIntAttribute( "int", &iVal );
		ele->QueryDoubleAttribute( "double", &dVal );

		XMLTest( "Attribute round trip. c-string.", "strValue", cStr );
		XMLTest( "Attribute round trip. int.", 1, iVal );
		XMLTest( "Attribute round trip. double.", -1, (int)dVal );
	}

#endif
	{
		XMLDocument doc;
		doc.LoadFile( "utf8test.xml" );

		// Get the attribute "value" from the "Russian" element and check it.
		XMLElement* element = doc.FirstChildElement( "document" )->FirstChildElement( "Russian" );
		const unsigned char correctValue[] = {	0xd1U, 0x86U, 0xd0U, 0xb5U, 0xd0U, 0xbdU, 0xd0U, 0xbdU, 
												0xd0U, 0xbeU, 0xd1U, 0x81U, 0xd1U, 0x82U, 0xd1U, 0x8cU, 0 };

		XMLTest( "UTF-8: Russian value.", (const char*)correctValue, element->Attribute( "value" ) );

		const unsigned char russianElementName[] = {	0xd0U, 0xa0U, 0xd1U, 0x83U,
														0xd1U, 0x81U, 0xd1U, 0x81U,
														0xd0U, 0xbaU, 0xd0U, 0xb8U,
														0xd0U, 0xb9U, 0 };
		const char russianText[] = "<\xD0\xB8\xD0\xBC\xD0\xB5\xD0\xB5\xD1\x82>";

		XMLText* text = doc.FirstChildElement( "document" )->FirstChildElement( (const char*) russianElementName )->FirstChild()->ToText();
		XMLTest( "UTF-8: Browsing russian element name.",
				 russianText,
				 text->Value() );

		// Now try for a round trip.
		doc.SaveFile( "utf8testout.xml" );

		// Check the round trip.
		char savedBuf[256];
		char verifyBuf[256];
		int okay = 0;

		FILE* saved  = fopen( "utf8testout.xml", "r" );
		FILE* verify = fopen( "utf8testverify.xml", "r" );

		if ( saved && verify )
		{
			okay = 1;
			while ( fgets( verifyBuf, 256, verify ) )
			{
				fgets( savedBuf, 256, saved );
				NullLineEndings( verifyBuf );
				NullLineEndings( savedBuf );

				if ( strcmp( verifyBuf, savedBuf ) )
				{
					printf( "verify:%s<\n", verifyBuf );
					printf( "saved :%s<\n", savedBuf );
					okay = 0;
					break;
				}
			}
		}
		if ( saved )
			fclose( saved );
		if ( verify )
			fclose( verify );
		XMLTest( "UTF-8: Verified multi-language round trip.", 1, okay );
	}


#if defined( WIN32 )
		_CrtMemCheckpoint( &endMemState );  
		//_CrtMemDumpStatistics( &endMemState );

		_CrtMemState diffMemState;
		_CrtMemDifference( &diffMemState, &startMemState, &endMemState );
		_CrtMemDumpStatistics( &diffMemState );
		//printf( "new total=%d\n", gNewTotal );
	#endif

	printf ("\nPass %d, Fail %d\n", gPass, gFail);
	return 0;
}