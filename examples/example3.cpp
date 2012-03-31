#include "tinyxml2.h"

using namespace tinyxml2;

int main (int argc, char * const argv[]) 
{
   XMLDocument doc;
   if (doc.LoadFile("input_example03.xml") == XML_SUCCESS)
   {
      XMLNode *new_node = doc.NewElement("foo");
      XMLNode *node = doc.FirstChildElement("Hello");
 
		node->InsertEndChild(new_node);
      new_node->InsertFirstChild(doc.NewText("bar"));
		doc.SaveFile("demo2.xml");

		XMLText* textNode = node->FirstChild()->ToText();
		const char* title = textNode->Value();
		printf( "Contents of Hello: %s\n", title );
		

}
	return 0;
}
