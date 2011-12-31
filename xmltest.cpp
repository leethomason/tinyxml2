#include "tinyxml2.h"

#include <stdio.h>
#include <stdlib.h>

using namespace tinyxml2;

int main( int argc, const char* argv )
{
	{
		static const char* test = "<!--hello world-->";

		XMLDocument doc;
		doc.Parse( test );
		doc.Print( stdout );
	}
	/*
	{
		static const char* test = "<hello></hello>";

		XMLDocument doc;
		doc.Parse( test );
		doc.Print( stdout );
	}
	*/
	return 0;
}