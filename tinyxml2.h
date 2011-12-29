#ifndef TINYXML2_INCLUDED
#define TINYXML2_INCLUDED


namespace tinyxml2
{



// internal - move to separate namespace
struct CharBuffer
{
	size_t  length;
	char	mem[1];

	static CharBuffer* Construct( const char* in );
	static void Free( CharBuffer* );
};


/*
class Element
{
public:
	Element
};
*/



class XMLDocument
{
public:
	XMLDocument();

	bool Parse( const char* );

private:
	XMLDocument( const XMLDocument& );	// not implemented
	CharBuffer* charBuffer;
};



};	// tinyxml2



#endif // TINYXML2_INCLUDED