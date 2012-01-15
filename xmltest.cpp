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
#endif
	{
		static const char* test = "<!--hello world\n"
			                      "          line 2\r-->";

		XMLDocument doc;
		doc.Parse( test );
		doc.Print( stdout );
	}
	return 0;
}