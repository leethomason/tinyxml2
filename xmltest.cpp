#include "tinyxml2.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined( _MSC_VER )
	#include <crtdbg.h>
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	_CrtMemState startMemState;
	_CrtMemState endMemState;
#endif

using namespace tinyxml2;
int gPass = 0;
int gFail = 0;


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


// Comments in the header. (Don't know how to get Doxygen to read comments in this file.)
int example_1()
{
	XMLDocument doc;
	doc.LoadFile( "resources/dream.xml" );

	return doc.ErrorID();
}


// Comments in the header. (Don't know how to get Doxygen to read comments in this file.)
int example_2()
{
	static const char* xml = "<element/>";
	XMLDocument doc;
	doc.Parse( xml );

	return doc.ErrorID();
}


int example_3()
{
	static const char* xml = 
		"<?xml version=\"1.0\"?>"
		"<!DOCTYPE PLAY SYSTEM \"play.dtd\">"
		"<PLAY>"
		"<TITLE>A Midsummer Night's Dream</TITLE>"
		"</PLAY>";

	XMLDocument doc;
	doc.Parse( xml );

	XMLElement* titleElement = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" );
	const char* title = titleElement->GetText();
	printf( "Name of play (1): %s\n", title );
		
	XMLText* textNode = titleElement->FirstChild()->ToText();
	title = textNode->Value();
	printf( "Name of play (2): %s\n", title );

	return doc.ErrorID();
}


int main( int /*argc*/, const char ** /*argv*/ )
{
	#if defined( _MSC_VER ) && defined( DEBUG )
		_CrtMemCheckpoint( &startMemState );
	#endif	

	#if defined(_MSC_VER)
	#pragma warning ( push )
	#pragma warning ( disable : 4996 )		// Fail to see a compelling reason why this should be deprecated.
	#endif

	FILE* fp = fopen( "resources/dream.xml", "r" );
	if ( !fp ) {
		printf( "Error opening test file 'dream.xml'.\n"
				"Is your working directory the same as where \n"
				"the xmltest.cpp and dream.xml file are?\n\n"
	#if defined( _MSC_VER )
				"In windows Visual Studio you may need to set\n"
				"Properties->Debugging->Working Directory to '..'\n"
	#endif
			  );
		exit( 1 );
	}
	fclose( fp );

	#if defined(_MSC_VER)
	#pragma warning ( pop )
	#endif

	XMLTest( "Example-1", 0, example_1() );
	XMLTest( "Example-2", 0, example_2() );
	XMLTest( "Example-3", 0, example_3() );

	/* ------ Example 2: Lookup information. ---- */	

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
#if 1
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
	{
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
		XMLTest( "Programmatic DOM", result, XML_NO_ATTRIBUTE );
		XMLTest( "Programmatic DOM", value, 10 );

		doc->Print();

		{
			XMLPrinter streamer;
			doc->Print( &streamer );
			printf( "%s", streamer.CStr() );
		}
		{
			XMLPrinter streamer( 0, true );
			doc->Print( &streamer );
			XMLTest( "Compact mode", "<element><sub attrib=\"1\"/><sub/></element>", streamer.CStr(), false );
		}
		delete doc;
	}
	{
		// Test: Dream
		// XML1 : 1,187,569 bytes	in 31,209 allocations
		// XML2 :   469,073	bytes	in    323 allocations
		//int newStart = gNew;
		XMLDocument doc;
		doc.LoadFile( "resources/dream.xml" );

		doc.SaveFile( "resources/dreamout.xml" );
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
		doc2.LoadFile( "resources/dreamout.xml" );
		XMLTest( "Dream-out", "xml version=\"1.0\"",
			              doc2.FirstChild()->ToDeclaration()->Value() );
		XMLTest( "Dream-out", true, doc2.FirstChild()->NextSibling()->ToUnknown() ? true : false );
		XMLTest( "Dream-out", "DOCTYPE PLAY SYSTEM \"play.dtd\"",
						  doc2.FirstChild()->NextSibling()->ToUnknown()->Value() );
		XMLTest( "Dream-out", "And Robin shall restore amends.",
			              doc2.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );

		//gNewTotal = gNew - newStart;
	}


	{
		const char* error =	"<?xml version=\"1.0\" standalone=\"no\" ?>\n"
							"<passages count=\"006\" formatversion=\"20020620\">\n"
							"    <wrong error>\n"
							"</passages>";

		XMLDocument doc;
		doc.Parse( error );
		XMLTest( "Bad XML", doc.ErrorID(), XML_ERROR_PARSING_ATTRIBUTE );
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
		XMLTest( "Query attribute: not a number", result, XML_WRONG_ATTRIBUTE_TYPE );
		result = ele->QueryIntAttribute( "bar", &iVal );
		XMLTest( "Query attribute: does not exist", result, XML_NO_ATTRIBUTE );
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

		XMLTest( "Attribute match test", ele->Attribute( "str", "strValue" ), "strValue" );
		XMLTest( "Attribute round trip. c-string.", "strValue", cStr );
		XMLTest( "Attribute round trip. int.", 1, iVal );
		XMLTest( "Attribute round trip. double.", -1, (int)dVal );
	}

	{
		XMLDocument doc;
		doc.LoadFile( "resources/utf8test.xml" );

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
		doc.SaveFile( "resources/utf8testout.xml" );

		// Check the round trip.
		char savedBuf[256];
		char verifyBuf[256];
		int okay = 0;


#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4996 )		// Fail to see a compelling reason why this should be deprecated.
#endif
		FILE* saved  = fopen( "resources/utf8testout.xml", "r" );
		FILE* verify = fopen( "resources/utf8testverify.xml", "r" );
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

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

	// --------GetText()-----------
	{
		const char* str = "<foo>This is  text</foo>";
		XMLDocument doc;
		doc.Parse( str );
		const XMLElement* element = doc.RootElement();

		XMLTest( "GetText() normal use.", "This is  text", element->GetText() );

		str = "<foo><b>This is text</b></foo>";
		doc.Parse( str );
		element = doc.RootElement();

		XMLTest( "GetText() contained element.", element->GetText() == 0, true );
	}


	// ---------- CDATA ---------------
	{
		const char* str =	"<xmlElement>"
								"<![CDATA["
									"I am > the rules!\n"
									"...since I make symbolic puns"
								"]]>"
							"</xmlElement>";
		XMLDocument doc;
		doc.Parse( str );
		doc.Print();

		XMLTest( "CDATA parse.", doc.FirstChildElement()->FirstChild()->Value(), 
								 "I am > the rules!\n...since I make symbolic puns",
								 false );
	}

	// ----------- CDATA -------------
	{
		const char* str =	"<xmlElement>"
								"<![CDATA["
									"<b>I am > the rules!</b>\n"
									"...since I make symbolic puns"
								"]]>"
							"</xmlElement>";
		XMLDocument doc;
		doc.Parse( str );
		doc.Print();

		XMLTest( "CDATA parse. [ tixml1:1480107 ]", doc.FirstChildElement()->FirstChild()->Value(), 
								 "<b>I am > the rules!</b>\n...since I make symbolic puns",
								 false );
	}

	// InsertAfterChild causes crash.
	{
		// InsertBeforeChild and InsertAfterChild causes crash.
		XMLDocument doc;
		XMLElement* parent = doc.NewElement( "Parent" );
		doc.InsertFirstChild( parent );

		XMLElement* childText0 = doc.NewElement( "childText0" );
		XMLElement* childText1 = doc.NewElement( "childText1" );

		XMLNode* childNode0 = parent->InsertEndChild( childText0 );
		XMLNode* childNode1 = parent->InsertAfterChild( childNode0, childText1 );

		XMLTest( "Test InsertAfterChild on empty node. ", ( childNode1 == parent->LastChild() ), true );
	}

	{
		// Entities not being written correctly.
		// From Lynn Allen

		const char* passages =
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<passages count=\"006\" formatversion=\"20020620\">"
				"<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;."
				" It also has &lt;, &gt;, and &amp;, as well as a fake copyright &#xA9;.\"> </psg>"
			"</passages>";

		XMLDocument doc;
		doc.Parse( passages );
		XMLElement* psg = doc.RootElement()->FirstChildElement();
		const char* context = psg->Attribute( "context" );
		const char* expected = "Line 5 has \"quotation marks\" and 'apostrophe marks'. It also has <, >, and &, as well as a fake copyright \xC2\xA9.";

		XMLTest( "Entity transformation: read. ", expected, context, true );

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4996 )		// Fail to see a compelling reason why this should be deprecated.
#endif
		FILE* textfile = fopen( "resources/textfile.txt", "w" );
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif
		if ( textfile )
		{
			XMLPrinter streamer( textfile );
			psg->Accept( &streamer );
			fclose( textfile );
		}
#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4996 )		// Fail to see a compelling reason why this should be deprecated.
#endif
		textfile = fopen( "resources/textfile.txt", "r" );
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif
		TIXMLASSERT( textfile );
		if ( textfile )
		{
			char buf[ 1024 ];
			fgets( buf, 1024, textfile );
			XMLTest( "Entity transformation: write. ",
					 "<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;."
					 " It also has &lt;, &gt;, and &amp;, as well as a fake copyright \xC2\xA9.\"/>\n",
					 buf, false );
		}
		fclose( textfile );
	}

	{
		// Suppress entities.
		const char* passages =
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<passages count=\"006\" formatversion=\"20020620\">"
				"<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;.\">Crazy &ttk;</psg>"
			"</passages>";
		
		XMLDocument doc( false );
		doc.Parse( passages );

		XMLTest( "No entity parsing.", doc.FirstChildElement()->FirstChildElement()->Attribute( "context" ), 
				 "Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;." );
		XMLTest( "No entity parsing.", doc.FirstChildElement()->FirstChildElement()->FirstChild()->Value(),
				 "Crazy &ttk;" );
		doc.Print();
	}

	{
        const char* test = "<?xml version='1.0'?><a.elem xmi.version='2.0'/>";

		XMLDocument doc;
        doc.Parse( test );
        XMLTest( "dot in names", doc.Error(), 0);
        XMLTest( "dot in names", doc.FirstChildElement()->Name(), "a.elem" );
        XMLTest( "dot in names", doc.FirstChildElement()->Attribute( "xmi.version" ), "2.0" );
	}

	{
        const char* test = "<element><Name>1.1 Start easy ignore fin thickness&#xA;</Name></element>";

        XMLDocument doc;
		doc.Parse( test );

		XMLText* text = doc.FirstChildElement()->FirstChildElement()->FirstChild()->ToText();
		XMLTest( "Entity with one digit.",
				 text->Value(), "1.1 Start easy ignore fin thickness\n",
				 false );
    }

	{
		// DOCTYPE not preserved (950171)
		// 
		const char* doctype =
			"<?xml version=\"1.0\" ?>"
			"<!DOCTYPE PLAY SYSTEM 'play.dtd'>"
			"<!ELEMENT title (#PCDATA)>"
			"<!ELEMENT books (title,authors)>"
			"<element />";

		XMLDocument doc;
		doc.Parse( doctype );
		doc.SaveFile( "resources/test7.xml" );
		doc.DeleteChild( doc.RootElement() );
		doc.LoadFile( "resources/test7.xml" );
		doc.Print();
		
		const XMLUnknown* decl = doc.FirstChild()->NextSibling()->ToUnknown();
		XMLTest( "Correct value of unknown.", "DOCTYPE PLAY SYSTEM 'play.dtd'", decl->Value() );

	}

	{
		// Comments do not stream out correctly.
		const char* doctype = 
			"<!-- Somewhat<evil> -->";
		XMLDocument doc;
		doc.Parse( doctype );

		XMLComment* comment = doc.FirstChild()->ToComment();

		XMLTest( "Comment formatting.", " Somewhat<evil> ", comment->Value() );
	}
	{
		// Double attributes
		const char* doctype = "<element attr='red' attr='blue' />";

		XMLDocument doc;
		doc.Parse( doctype );
		
		XMLTest( "Parsing repeated attributes.", XML_ERROR_PARSING_ATTRIBUTE, doc.ErrorID() );	// is an  error to tinyxml (didn't use to be, but caused issues)
		doc.PrintError();
	}

	{
		// Embedded null in stream.
		const char* doctype = "<element att\0r='red' attr='blue' />";

		XMLDocument doc;
		doc.Parse( doctype );
		XMLTest( "Embedded null throws error.", true, doc.Error() );
	}

	{
		// Empty documents should return TIXML_XML_ERROR_PARSING_EMPTY, bug 1070717
		const char* str = "    ";
		XMLDocument doc;
		doc.Parse( str );
		XMLTest( "Empty document error", XML_ERROR_EMPTY_DOCUMENT, doc.ErrorID() );
	}

	{
		// Low entities
		XMLDocument doc;
		doc.Parse( "<test>&#x0e;</test>" );
		const char result[] = { 0x0e, 0 };
		XMLTest( "Low entities.", doc.FirstChildElement()->GetText(), result );
		doc.Print();
	}

	{
		// Attribute values with trailing quotes not handled correctly
		XMLDocument doc;
		doc.Parse( "<foo attribute=bar\" />" );
		XMLTest( "Throw error with bad end quotes.", doc.Error(), true );
	}

	{
		// [ 1663758 ] Failure to report error on bad XML
		XMLDocument xml;
		xml.Parse("<x>");
		XMLTest("Missing end tag at end of input", xml.Error(), true);
		xml.Parse("<x> ");
		XMLTest("Missing end tag with trailing whitespace", xml.Error(), true);
		xml.Parse("<x></y>");
		XMLTest("Mismatched tags", xml.ErrorID(), XML_ERROR_MISMATCHED_ELEMENT);
	} 


	{
		// [ 1475201 ] TinyXML parses entities in comments
		XMLDocument xml;
		xml.Parse("<!-- declarations for <head> & <body> -->"
				  "<!-- far &amp; away -->" );

		XMLNode* e0 = xml.FirstChild();
		XMLNode* e1 = e0->NextSibling();
		XMLComment* c0 = e0->ToComment();
		XMLComment* c1 = e1->ToComment();

		XMLTest( "Comments ignore entities.", " declarations for <head> & <body> ", c0->Value(), true );
		XMLTest( "Comments ignore entities.", " far &amp; away ", c1->Value(), true );
	}

	{
		XMLDocument xml;
		xml.Parse( "<Parent>"
						"<child1 att=''/>"
						"<!-- With this comment, child2 will not be parsed! -->"
						"<child2 att=''/>"
					"</Parent>" );
		xml.Print();

		int count = 0;

		for( XMLNode* ele = xml.FirstChildElement( "Parent" )->FirstChild();
			 ele;
			 ele = ele->NextSibling() )
		{
			++count;
		}

		XMLTest( "Comments iterate correctly.", 3, count );
	}

	{
		// trying to repro ]1874301]. If it doesn't go into an infinite loop, all is well.
		unsigned char buf[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?><feed><![CDATA[Test XMLblablablalblbl";
		buf[60] = 239;
		buf[61] = 0;

		XMLDocument doc;
		doc.Parse( (const char*)buf);
	} 


	{
		// bug 1827248 Error while parsing a little bit malformed file
		// Actually not malformed - should work.
		XMLDocument xml;
		xml.Parse( "<attributelist> </attributelist >" );
		XMLTest( "Handle end tag whitespace", false, xml.Error() );
	}

	{
		// This one must not result in an infinite loop
		XMLDocument xml;
		xml.Parse( "<infinite>loop" );
		XMLTest( "Infinite loop test.", true, true );
	}
#endif
	{
		const char* pub = "<?xml version='1.0'?> <element><sub/></element> <!--comment--> <!DOCTYPE>";
		XMLDocument doc;
		doc.Parse( pub );

		XMLDocument clone;
		for( const XMLNode* node=doc.FirstChild(); node; node=node->NextSibling() ) {
			XMLNode* copy = node->ShallowClone( &clone );
			clone.InsertEndChild( copy );
		}

		clone.Print();

		int count=0;
		const XMLNode* a=clone.FirstChild();
		const XMLNode* b=doc.FirstChild();
		for( ; a && b; a=a->NextSibling(), b=b->NextSibling() ) {
			++count;
			XMLTest( "Clone and Equal", true, a->ShallowEqual( b ));
		}
		XMLTest( "Clone and Equal", 4, count );
	}

	{
		// This shouldn't crash.
		XMLDocument doc;
		if(XML_NO_ERROR != doc.LoadFile( "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ))
		{
			doc.PrintError();
		}
		XMLTest( "Error in snprinf handling.", true, doc.Error() );
	}
	
	{
		// Attribute ordering.
		static const char* xml = "<element attrib1=\"1\" attrib2=\"2\" attrib3=\"3\" />";
		XMLDocument doc;
		doc.Parse( xml );
		XMLElement* ele = doc.FirstChildElement();
		
		const XMLAttribute* a = ele->FirstAttribute();
		XMLTest( "Attribute order", "1", a->Value() );
		a = a->Next();
		XMLTest( "Attribute order", "2", a->Value() );
		a = a->Next();
		XMLTest( "Attribute order", "3", a->Value() );
		XMLTest( "Attribute order", "attrib3", a->Name() );
		
		ele->DeleteAttribute( "attrib2" );
		a = ele->FirstAttribute();
		XMLTest( "Attribute order", "1", a->Value() );
		a = a->Next();
		XMLTest( "Attribute order", "3", a->Value() );
		
		ele->DeleteAttribute( "attrib1" );
		ele->DeleteAttribute( "attrib3" );
		XMLTest( "Attribute order (empty)", false, ele->FirstAttribute() ? true : false );
	}

	{
		// Make sure an attribute with a space in it succeeds.
		static const char* xml0 = "<element attribute1= \"Test Attribute\"/>";
		static const char* xml1 = "<element attribute1 =\"Test Attribute\"/>";
		static const char* xml2 = "<element attribute1 = \"Test Attribute\"/>";
		XMLDocument doc0;
		doc0.Parse( xml0 );
		XMLDocument doc1;
		doc1.Parse( xml1 );
		XMLDocument doc2;
		doc2.Parse( xml2 );

		XMLElement* ele = 0;
		ele = doc0.FirstChildElement();
		XMLTest( "Attribute with space #1", "Test Attribute", ele->Attribute( "attribute1" ) );
		ele = doc1.FirstChildElement();
		XMLTest( "Attribute with space #2", "Test Attribute", ele->Attribute( "attribute1" ) );
		ele = doc2.FirstChildElement();
		XMLTest( "Attribute with space #3", "Test Attribute", ele->Attribute( "attribute1" ) );
	}

	{
		// Make sure we don't go into an infinite loop.
		static const char* xml = "<doc><element attribute='attribute'/><element attribute='attribute'/></doc>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLElement* ele0 = doc.FirstChildElement()->FirstChildElement();
		XMLElement* ele1 = ele0->NextSiblingElement();
		bool equal = ele0->ShallowEqual( ele1 );

		XMLTest( "Infinite loop in shallow equal.", true, equal );
	}

	// -------- Handles ------------
	{
		static const char* xml = "<element attrib='bar'><sub>Text</sub></element>";
		XMLDocument doc;
		doc.Parse( xml );

		XMLElement* ele = XMLHandle( doc ).FirstChildElement( "element" ).FirstChild().ToElement();
		XMLTest( "Handle, success, mutable", ele->Value(), "sub" );

		XMLHandle docH( doc );
		ele = docH.FirstChildElement( "none" ).FirstChildElement( "element" ).ToElement();
		XMLTest( "Handle, dne, mutable", false, ele != 0 );
	}
	
	{
		static const char* xml = "<element attrib='bar'><sub>Text</sub></element>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLConstHandle docH( doc );

		const XMLElement* ele = docH.FirstChildElement( "element" ).FirstChild().ToElement();
		XMLTest( "Handle, success, const", ele->Value(), "sub" );

		ele = docH.FirstChildElement( "none" ).FirstChildElement( "element" ).ToElement();
		XMLTest( "Handle, dne, const", false, ele != 0 );
	}
	{
		// Default Declaration & BOM
		XMLDocument doc;
		doc.InsertEndChild( doc.NewDeclaration() );
		doc.SetBOM( true );
		
		XMLPrinter printer;
		doc.Print( &printer );

		static const char* result  = "\xef\xbb\xbf<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		XMLTest( "BOM and default declaration", printer.CStr(), result, false );
		XMLTest( "CStrSize", printer.CStrSize(), 42, false );
	}

	
	// ----------- Performance tracking --------------
	{
#if defined( _MSC_VER )
		__int64 start, end, freq;
		QueryPerformanceFrequency( (LARGE_INTEGER*) &freq );
#endif

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4996 )		// Fail to see a compelling reason why this should be deprecated.
#endif
		FILE* fp  = fopen( "resources/dream.xml", "r" );
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif
		fseek( fp, 0, SEEK_END );
		long size = ftell( fp );
		fseek( fp, 0, SEEK_SET );

		char* mem = new char[size+1];
		fread( mem, size, 1, fp );
		fclose( fp );
		mem[size] = 0;

#if defined( _MSC_VER )
		QueryPerformanceCounter( (LARGE_INTEGER*) &start );
#else
		clock_t cstart = clock();
#endif
		static const int COUNT = 10;
		for( int i=0; i<COUNT; ++i ) {
			XMLDocument doc;
			doc.Parse( mem );
		}
#if defined( _MSC_VER )
		QueryPerformanceCounter( (LARGE_INTEGER*) &end );
#else
		clock_t cend = clock();
#endif

		delete [] mem;

		static const char* note = 
#ifdef DEBUG
			"DEBUG";
#else
			"Release";
#endif

#if defined( _MSC_VER )
		printf( "\nParsing %s of dream.xml: %.3f milli-seconds\n", note, 1000.0 * (double)(end-start) / ( (double)freq * (double)COUNT) );
#else
		printf( "\nParsing %s of dream.xml: %.3f milli-seconds\n", note, (double)(cend - cstart)/(double)COUNT );
#endif
	}

	#if defined( _MSC_VER ) &&  defined( DEBUG )
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
