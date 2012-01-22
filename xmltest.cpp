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
									   // "<element></element>",
									    "<element><subelement/></element>",
										0
		};
		for( int i=0; test[i]; ++i ) {
			XMLDocument doc;
			doc.Parse( test[i] );
			doc.Print( stdout );
		}
	}
	return 0;
}