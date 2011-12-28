#ifndef TINYXML2_INCLUDED
#define TINYXML2_INCLUDED


namespace tinyxml2
{


class Element
{
public:
	Element
};


class XMLDocument
{
public:
	XMLDocument();

	bool Parse( const char* );

private:
	XMLDocument( const XMLDocument& );	// not implemented
};





};	// tinyxml2

#endif // TINYXML2_INCLUDED