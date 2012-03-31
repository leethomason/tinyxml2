#include "tinyxml2.h"

using namespace tinyxml2;

int main (int argc, char * const argv[]) 
{
   XMLDocument doc;
   if (doc.LoadFile("demo.xml") == XML_SUCCESS)
   {
      XMLNode *node = doc.NewElement("foo");
      doc.InsertEndChild(node);
      doc.SaveFile("demo2.xml");
   }
	return 0;
}
