#include "tinyxml2.h"
#include <cstdio>

using namespace tinyxml2;


int main(void)
{
	XMLDocument doc;
	doc.LoadFile( "../dream.xml" );
	const char* title = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" )->GetText();
		printf( "Name of play (1): %s\n", title );

		// Text is just another Node to TinyXML-2. The more
		// general way to get to the XMLText:
		XMLText* textNode = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" )->FirstChild()->ToText();
		title = textNode->Value();
		printf( "Name of play (2): %s\n", title );
	return 0;
}
