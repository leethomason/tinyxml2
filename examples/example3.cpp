#include "tinyxml2.h"
#include <cstdio>

using namespace tinyxml2;

int main (int argc, char * const argv[]) 
{
   XMLDocument doc;
   
	if (doc.LoadFile("input_example_03.xml") == XML_SUCCESS)
   {
      //search for a node named Hello
		XMLNode *node = doc.FirstChildElement("Hello");
		//read the text content of it and print it to stdout
		XMLText* textNode = node->FirstChild()->ToText();
		const char* title = textNode->Value();
		printf( "Contents of Hello: %s\n", title );
 
		//create a new node named foo
      XMLNode *new_node = doc.NewElement("foo");
		//add foo as child of Hello
		node->InsertEndChild(new_node);
		//add some text content to the "foo" node
      new_node->InsertFirstChild(doc.NewText("bar"));

		//save the output document
		doc.SaveFile("output_example_03.xml");

	}

	return 0;
}
