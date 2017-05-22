#include "tinyxml2.h"

using namespace tinyxml2;

int main (int argc, char * const argv[]) 
{
   XMLDocument doc;
   if (doc.LoadFile("input_example_02.xml") == XML_SUCCESS)
   {
      XMLNode *node = doc.NewElement("foo");
      doc.InsertEndChild(node);
      doc.SaveFile("output_example_02.xml");
   }
	return 0;
}
