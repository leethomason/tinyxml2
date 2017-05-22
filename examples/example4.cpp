#include "tinyxml2.h"
#include <cstdio>

using namespace tinyxml2;

int main (int argc, char * const argv[]) 
{
   XMLDocument doc;
   
	if (doc.LoadFile(argv[1]) == XML_SUCCESS)
   {
      //search for a node named Hello
		XMLElement *node = doc.RootElement();
	 	

		for(XMLElement* current_node=node->FirstChildElement("type");current_node!=NULL;current_node=current_node->NextSiblingElement("type"))
		{
			const char* at = current_node->Attribute("id");
			printf("id: %s\n",at);
		}	


	}

	return 0;
}
