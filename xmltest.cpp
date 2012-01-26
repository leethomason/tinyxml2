#include "tinyxml2.h"

#include <stdio.h>
#include <stdlib.h>

using namespace tinyxml2;

int main( int argc, const char* argv )
{
#if 0
	{
		static const char* test = "<!--hello world-->";

		XMLDocument doc;
		doc.Parse( test );
		doc.Print( stdout );
	}
	{
		static const char* test = "<!--hello world\n"
			                      "          line 2\r"
			                      "          line 3\r\n"
			                      "          line 4\n\r"
			                      "          line 5\r-->";

		XMLDocument doc;
		doc.Parse( test );
		doc.Print( stdout );
	}
#endif
	{
		static const char* test[] = {	//"<element />",
									    //"<element></element>",
										//"<element><subelement/></element>",
									    //"<element><subelement></subelement></element>",
									    "<element><subelement><subsub/></subelement></element>",
									    "<!--comment beside elements--><element><subelement></subelement></element>",
									    //"<!--comment beside elements, this time with spaces-->  \n <element>  <subelement> \n </subelement> </element>",
									    "<element attrib1='foo' attrib2=\"bar\" ></element>",
									    "<element attrib1='foo' attrib2=\"bar\" ><subelement attrib3='yeehaa' /></element>",
										//"<element>Text inside element.</element>",
										//"<element><b></b></element>",
										"<element>Text inside and <b>bolded</b> in the element.</element>",
										"<outer><element>Text inside and <b>bolded</b> in the element.</element></outer>",
										0
		};
		for( int i=0; test[i]; ++i ) {
			XMLDocument doc;
			doc.Parse( test[i] );
			doc.Print();
			printf( "----------------------------------------------\n" );
		}
	}
	return 0;
}