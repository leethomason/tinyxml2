#ifndef TINYXML_INCLUDED
#define TINYXML2_INCLUDED

/*
	TODO
	X const and non-const versions of API
	X memory pool the class construction
	X attribute accessors
	X node navigation
	- handles
	X visit pattern - change streamer?
	X make constructors protected
	X hide copy constructor
	X hide = operator
	X UTF8 support: isAlpha, etc.
	- string buffer for sets. (Grr.)
	- MS BOM
	- print to memory buffer
	- tests from xml1
	- xml1 tests especially UTF-8
	- perf test: xml1
	- perf test: xenowar
*/

#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <memory.h>

#if defined( _DEBUG ) || defined( DEBUG ) || defined (__DEBUG__)
	#ifndef DEBUG
		#define DEBUG
	#endif
#endif


#if defined(DEBUG)
        #if defined(_MSC_VER)
                #define TIXMLASSERT( x )           if ( !(x)) { _asm { int 3 } } //if ( !(x)) WinDebugBreak()
        #elif defined (ANDROID_NDK)
                #include <android/log.h>
                #define TIXMLASSERT( x )           if ( !(x)) { __android_log_assert( "assert", "grinliz", "ASSERT in '%s' at %d.", __FILE__, __LINE__ ); }
        #else
                #include <assert.h>
                #define TIXMLASSERT                assert
        #endif
#else
        #define TIXMLASSERT( x )           {}
#endif


// Deprecated library function hell. Compilers want to use the
// new safe versions. This probably doesn't fully address the problem,
// but it gets closer. There are too many compilers for me to fully
// test. If you get compilation troubles, undefine TIXML_SAFE

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
	// Microsoft visual studio, version 2005 and higher.
	#define TIXML_SNPRINTF _snprintf_s
	#define TIXML_SSCANF   sscanf_s
#elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
	// Microsoft visual studio, version 6 and higher.
	//#pragma message( "Using _sn* functions." )
	#define TIXML_SNPRINTF _snprintf
	#define TIXML_SSCANF   sscanf
#elif defined(__GNUC__) && (__GNUC__ >= 3 )
	// GCC version 3 and higher.s
	//#warning( "Using sn* functions." )
	#define TIXML_SNPRINTF snprintf
	#define TIXML_SSCANF   sscanf
#else
	#define TIXML_SNPRINTF snprintf
	#define TIXML_SSCANF   sscanf
#endif


namespace tinyxml2
{
class XMLDocument;
class XMLElement;
class XMLAttribute;
class XMLComment;
class XMLNode;
class XMLText;
class XMLDeclaration;
class XMLUnknown;

class XMLStreamer;

class StrPair
{
public:
	enum {
		NEEDS_ENTITY_PROCESSING			= 0x01,
		NEEDS_NEWLINE_NORMALIZATION		= 0x02,

		TEXT_ELEMENT		= NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION,
		ATTRIBUTE_NAME		= 0,
		ATTRIBUTE_VALUE		= NEEDS_ENTITY_PROCESSING | NEEDS_NEWLINE_NORMALIZATION,
		COMMENT				= NEEDS_NEWLINE_NORMALIZATION,
	};

	StrPair() : flags( 0 ), start( 0 ), end( 0 ) {}
	void Set( char* start, char* end, int flags ) {
		this->start = start; this->end = end; this->flags = flags | NEEDS_FLUSH;
	}
	const char* GetStr();
	bool Empty() const { return start == end; }

	void SetInternedStr( const char* str ) { this->start = (char*) str; this->end = 0; this->flags = 0; }
	char* ParseText( char* in, const char* endTag, int strFlags );
	char* ParseName( char* in );


private:
	enum {
		NEEDS_FLUSH = 0x100
	};

	// After parsing, if *end != 0, it can be set to zero.
	int flags;
	char* start;	
	char* end;
};


template <class T, int INIT>
class DynArray
{
public:
	DynArray< T, INIT >() 
	{
		mem = pool;
		allocated = INIT;
		size = 0;
	}
	~DynArray()
	{
		if ( mem != pool ) {
			delete mem;
		}
	}
	void Push( T t )
	{
		EnsureCapacity( size+1 );
		mem[size++] = t;
	}

	T* PushArr( int count )
	{
		EnsureCapacity( size+count );
		T* ret = &mem[size];
		size += count;
		return ret;
	}
	T Pop() {
		return mem[--size];
	}
	void PopArr( int count ) 
	{
		TIXMLASSERT( size >= count );
		size -= count;
	}

	bool Empty() const { return size == 0; }
	T& operator[](int i) { TIXMLASSERT( i>= 0 && i < size ); return mem[i]; }
	const T& operator[](int i) const { TIXMLASSERT( i>= 0 && i < size ); return mem[i]; }
	int Size() const { return size; }
	const T* Mem() const { return mem; }
	T* Mem() { return mem; }


private:
	void EnsureCapacity( int cap ) {
		if ( cap > allocated ) {
			int newAllocated = cap * 2;
			T* newMem = new T[newAllocated];
			memcpy( newMem, mem, sizeof(T)*size );	// warning: not using constructors, only works for PODs
			if ( mem != pool ) delete [] mem;
			mem = newMem;
			allocated = newAllocated;
		}
	}

	T* mem;
	T pool[INIT];
	int allocated;		// objects allocated
	int size;			// number objects in use
};


class MemPool
{
public:
	MemPool() {}
	virtual ~MemPool() {}

	virtual int ItemSize() const = 0;
	virtual void* Alloc() = 0;
	virtual void Free( void* ) = 0; 
};


template< int SIZE >
class MemPoolT : public MemPool
{
public:
	MemPoolT() : root(0), currentAllocs(0), nAllocs(0), maxAllocs(0)	{}
	~MemPoolT() {
		// Delete the blocks.
		for( int i=0; i<blockPtrs.Size(); ++i ) {
			delete blockPtrs[i];
		}
	}

	virtual int ItemSize() const	{ return SIZE; }
	int CurrentAllocs() const		{ return currentAllocs; }

	virtual void* Alloc() {
		if ( !root ) {
			// Need a new block.
			Block* block = new Block();
			blockPtrs.Push( block );

			for( int i=0; i<COUNT-1; ++i ) {
				block->chunk[i].next = &block->chunk[i+1];
			}
			block->chunk[COUNT-1].next = 0;
			root = block->chunk;
		}
		void* result = root;
		root = root->next;

		++currentAllocs;
		if ( currentAllocs > maxAllocs ) maxAllocs = currentAllocs;
		nAllocs++;
		return result;
	}
	virtual void Free( void* mem ) {
		if ( !mem ) return;
		--currentAllocs;
		Chunk* chunk = (Chunk*)mem;
		memset( chunk, 0xfe, sizeof(Chunk) );
		chunk->next = root;
		root = chunk;
	}
	void Trace( const char* name ) {
		printf( "Mempool %s watermark=%d [%dk] current=%d size=%d nAlloc=%d blocks=%d\n",
				 name, maxAllocs, maxAllocs*SIZE/1024, currentAllocs, SIZE, nAllocs, blockPtrs.Size() );
	}

private:
	enum { COUNT = 1024/SIZE };
	union Chunk {
		Chunk* next;
		char mem[SIZE];
	};
	struct Block {
		Chunk chunk[COUNT];
	};
	DynArray< Block*, 10 > blockPtrs;
	Chunk* root;

	int currentAllocs;
	int nAllocs;
	int maxAllocs;
};



/**
	Implements the interface to the "Visitor pattern" (see the Accept() method.)
	If you call the Accept() method, it requires being passed a XMLVisitor
	class to handle callbacks. For nodes that contain other nodes (Document, Element)
	you will get called with a VisitEnter/VisitExit pair. Nodes that are always leaves
	are simply called with Visit().

	If you return 'true' from a Visit method, recursive parsing will continue. If you return
	false, <b>no children of this node or its sibilings</b> will be Visited.

	All flavors of Visit methods have a default implementation that returns 'true' (continue 
	visiting). You need to only override methods that are interesting to you.

	Generally Accept() is called on the TiXmlDocument, although all nodes suppert Visiting.

	You should never change the document from a callback.

	@sa XMLNode::Accept()
*/
class XMLVisitor
{
public:
	virtual ~XMLVisitor() {}

	/// Visit a document.
	virtual bool VisitEnter( const XMLDocument& /*doc*/ )			{ return true; }
	/// Visit a document.
	virtual bool VisitExit( const XMLDocument& /*doc*/ )			{ return true; }

	/// Visit an element.
	virtual bool VisitEnter( const XMLElement& /*element*/, const XMLAttribute* /*firstAttribute*/ )	{ return true; }
	/// Visit an element.
	virtual bool VisitExit( const XMLElement& /*element*/ )			{ return true; }

	/// Visit a declaration
	virtual bool Visit( const XMLDeclaration& /*declaration*/ )		{ return true; }
	/// Visit a text node
	virtual bool Visit( const XMLText& /*text*/ )					{ return true; }
	/// Visit a comment node
	virtual bool Visit( const XMLComment& /*comment*/ )				{ return true; }
	/// Visit an unknown node
	virtual bool Visit( const XMLUnknown& /*unknown*/ )				{ return true; }
};


class XMLUtil
{
public:
	// Anything in the high order range of UTF-8 is assumed to not be whitespace. This isn't 
	// correct, but simple, and usually works.
	static const char* SkipWhiteSpace( const char* p )	{ while( IsUTF8Continuation(*p) || isspace( *p ) ) { ++p; } return p; }
	static char* SkipWhiteSpace( char* p )				{ while( IsUTF8Continuation(*p) || isspace( *p ) ) { ++p; } return p; }

	inline static bool StringEqual( const char* p, const char* q, int nChar=INT_MAX )  {
		int n = 0;
		if ( p == q ) {
			return true;
		}
		while( *p && *q && *p == *q && n<nChar ) {
			++p; ++q; ++n;
		}
		if ( (n == nChar) || ( *p == 0 && *q == 0 ) ) {
			return true;
		}
		return false;
	}
	inline static int IsUTF8Continuation( unsigned char p ) { return p & 0x80; }
	inline static int IsAlphaNum( unsigned char anyByte )	{ return ( anyByte < 128 ) ? isalnum( anyByte ) : 1; }
	inline static int IsAlpha( unsigned char anyByte )		{ return ( anyByte < 128 ) ? isalpha( anyByte ) : 1; }
};


class XMLNode
{
	friend class XMLDocument;
	friend class XMLElement;
public:
	const XMLDocument* GetDocument() const	{ return document; }
	XMLDocument* GetDocument()				{ return document; }

	virtual XMLElement*		ToElement()		{ return 0; }
	virtual XMLText*		ToText()		{ return 0; }
	virtual XMLComment*		ToComment()		{ return 0; }
	virtual XMLDocument*	ToDocument()	{ return 0; }
	virtual XMLDeclaration*	ToDeclaration()	{ return 0; }
	virtual XMLUnknown*		ToUnknown()		{ return 0; }

	virtual const XMLElement*		ToElement() const		{ return 0; }
	virtual const XMLText*			ToText() const			{ return 0; }
	virtual const XMLComment*		ToComment() const		{ return 0; }
	virtual const XMLDocument*		ToDocument() const		{ return 0; }
	virtual const XMLDeclaration*	ToDeclaration() const	{ return 0; }
	virtual const XMLUnknown*		ToUnknown() const		{ return 0; }

	const char* Value() const			{ return value.GetStr(); }
	void SetValue( const char* val )	{ value.SetInternedStr( val ); }

	const XMLNode*	Parent() const			{ return parent; }
	XMLNode* Parent()						{ return parent; }

	/// Returns true if this node has no children.
	bool NoChildren() const					{ return !firstChild; }

	const XMLNode*  FirstChild() const		{ return firstChild; }
	XMLNode*		FirstChild()			{ return firstChild; }
	const XMLElement* FirstChildElement( const char* value=0 ) const;
	XMLElement* FirstChildElement( const char* value=0 )	{ return const_cast<XMLElement*>(const_cast<const XMLNode*>(this)->FirstChildElement( value )); }

	const XMLNode*	LastChild() const						{ return lastChild; }
	XMLNode*		LastChild()								{ return const_cast<XMLNode*>(const_cast<const XMLNode*>(this)->LastChild() ); }

	const XMLElement* LastChildElement( const char* value=0 ) const;
	XMLElement* LastChildElement( const char* value=0 )	{ return const_cast<XMLElement*>(const_cast<const XMLNode*>(this)->LastChildElement(value) ); }
	
	const XMLNode*	PreviousSibling() const					{ return prev; }
	XMLNode*	PreviousSibling()							{ return prev; }

	const XMLNode*	PreviousSiblingElement( const char* value=0 ) const ;
	XMLNode*	PreviousSiblingElement( const char* value=0 ) { return const_cast<XMLNode*>(const_cast<const XMLNode*>(this)->PreviousSiblingElement( value ) ); }
	
	const XMLNode*	NextSibling() const						{ return next; }
	XMLNode*	NextSibling()								{ return next; }
		
	const XMLNode*	NextSiblingElement( const char* value=0 ) const;
 	XMLNode*	NextSiblingElement( const char* value=0 )	{ return const_cast<XMLNode*>(const_cast<const XMLNode*>(this)->NextSiblingElement( value ) ); }

	/**

		Tests: Programmatic DOM
	*/
	XMLNode* InsertEndChild( XMLNode* addThis );
	/**

		Tests: Programmatic DOM
	*/
	XMLNode* InsertFirstChild( XMLNode* addThis );
	/**

		Tests: Programmatic DOM
	*/
	XMLNode* InsertAfterChild( XMLNode* afterThis, XMLNode* addThis );
	
	void ClearChildren();
	void DeleteChild( XMLNode* node );

	virtual bool Accept( XMLVisitor* visitor ) const = 0;
	//virtual void Print( XMLStreamer* streamer );

	virtual char* ParseDeep( char* );
	virtual bool IsClosingElement() const { return false; }

protected:
	XMLNode( XMLDocument* );
	virtual ~XMLNode();
	XMLNode( const XMLNode& );	// not supported
	void operator=( const XMLNode& );	// not supported
	
	XMLDocument*	document;
	XMLNode*		parent;
	mutable StrPair	value;

	XMLNode*		firstChild;
	XMLNode*		lastChild;

	XMLNode*		prev;
	XMLNode*		next;

private:
	MemPool*		memPool;
	void Unlink( XMLNode* child );
};


class XMLText : public XMLNode
{
	friend class XMLBase;
	friend class XMLDocument;
public:
	virtual bool Accept( XMLVisitor* visitor ) const;

	virtual XMLText*	ToText()			{ return this; }
	virtual const XMLText*	ToText() const	{ return this; }

	void SetCData( bool value )				{ isCData = true; }
	bool CData() const						{ return isCData; }

	char* ParseDeep( char* );

protected:
	XMLText( XMLDocument* doc )	: XMLNode( doc ), isCData( false )	{}
	virtual ~XMLText()												{}
	XMLText( const XMLText& );	// not supported
	void operator=( const XMLText& );	// not supported

private:
	bool isCData;
};


class XMLComment : public XMLNode
{
	friend class XMLDocument;
public:
	virtual XMLComment*	ToComment()					{ return this; }
	virtual const XMLComment* ToComment() const		{ return this; }

	virtual bool Accept( XMLVisitor* visitor ) const;

	char* ParseDeep( char* );

protected:
	XMLComment( XMLDocument* doc );
	virtual ~XMLComment();
	XMLComment( const XMLComment& );	// not supported
	void operator=( const XMLComment& );	// not supported

private:
};


class XMLDeclaration : public XMLNode
{
	friend class XMLDocument;
public:
	virtual XMLDeclaration*	ToDeclaration()					{ return this; }
	virtual const XMLDeclaration* ToDeclaration() const		{ return this; }

	virtual bool Accept( XMLVisitor* visitor ) const;

	char* ParseDeep( char* );

protected:
	XMLDeclaration( XMLDocument* doc );
	virtual ~XMLDeclaration();
	XMLDeclaration( const XMLDeclaration& );	// not supported
	void operator=( const XMLDeclaration& );	// not supported
};


class XMLUnknown : public XMLNode
{
	friend class XMLDocument;
public:
	virtual XMLUnknown*	ToUnknown()					{ return this; }
	virtual const XMLUnknown* ToUnknown() const		{ return this; }

	virtual bool Accept( XMLVisitor* visitor ) const;

	char* ParseDeep( char* );

protected:
	XMLUnknown( XMLDocument* doc );
	virtual ~XMLUnknown();
	XMLUnknown( const XMLUnknown& );	// not supported
	void operator=( const XMLUnknown& );	// not supported
};


enum {
	ATTRIBUTE_SUCCESS,
	NO_ATTRIBUTE,
	WRONG_ATTRIBUTE_TYPE
};


class XMLAttribute
{
	friend class XMLElement;
public:
	const char* Name() const { return name.GetStr(); }
	const char* Value() const { return value.GetStr(); }
	const XMLAttribute* Next() const { return next; }

	int QueryIntAttribute( int* value ) const;
	int QueryUnsignedAttribute( unsigned int* value ) const;
	int QueryBoolAttribute( bool* value ) const;
	int QueryDoubleAttribute( double* value ) const;
	int QueryFloatAttribute( float* value ) const;

	void SetAttribute( const char* value );
	
	// NOTE: other sets aren't supported...need to deal with memory model?	
	/*
	void SetAttribute( int value );
	void SetAttribute( unsigned value );
	void SetAttribute( bool value );
	void SetAttribute( double value );
	*/

private:
	XMLAttribute( XMLElement* element ) : next( 0 ) {}
	virtual ~XMLAttribute()	{}
	XMLAttribute( const XMLAttribute& );	// not supported
	void operator=( const XMLAttribute& );	// not supported

	char* ParseDeep( char* p );

	mutable StrPair name;
	mutable StrPair value;
	XMLAttribute* next;
	MemPool* memPool;
};


class XMLElement : public XMLNode
{
	friend class XMLBase;
	friend class XMLDocument;
public:
	const char* Name() const		{ return Value(); }
	void SetName( const char* str )	{ SetValue( str ); }

	virtual XMLElement* ToElement()				{ return this; }
	virtual const XMLElement* ToElement() const { return this; }
	virtual bool Accept( XMLVisitor* visitor ) const;

	const char* Attribute( const char* name ) const;

	int QueryIntAttribute( const char* name, int* value ) const;
	int QueryUnsignedAttribute( const char* name, unsigned int* value ) const;
	int QueryBoolAttribute( const char* name, bool* value ) const;
	int QueryDoubleAttribute( const char* name, double* _value ) const;
	int QueryFloatAttribute( const char* name, float* _value ) const;

	void SetAttribute( const char* name, const char* value );
	void SetAttribute( const char* name, int value );
	void SetAttribute( const char* name, unsigned value );
	void SetAttribute( const char* name, bool value );
	void SetAttribute( const char* name, double value );

	void RemoveAttribute( const char* name );

	const XMLAttribute* FirstAttribute() const { return rootAttribute; }

	const char* GetText() const;

	// internal:
	virtual bool IsClosingElement() const { return closing; }
	char* ParseDeep( char* p );

private:
	XMLElement( XMLDocument* doc );
	virtual ~XMLElement();
	XMLElement( const XMLElement& );	// not supported
	void operator=( const XMLElement& );	// not supported

	char* ParseAttributes( char* p, bool *closedElement );

	bool closing;
	XMLAttribute* rootAttribute;
	XMLAttribute* lastAttribute;	// fixme: remove
};


class XMLDocument : public XMLNode
{
	friend class XMLElement;
public:
	XMLDocument(); 
	~XMLDocument();

	virtual XMLDocument* ToDocument()				{ return this; }
	virtual const XMLDocument* ToDocument() const	{ return this; }

	int Parse( const char* );
	int Load( const char* );
	int Load( FILE* );

	void Print( XMLStreamer* streamer=0 );
	virtual bool Accept( XMLVisitor* visitor ) const;

	/**
		Testing: Programmatic DOM
	*/
	XMLElement* NewElement( const char* name );
	/**
		Testing: Programmatic DOM
	*/
	XMLComment* NewComment( const char* comment );
	/**
		Testing: Programmatic DOM
	*/
	XMLText* NewText( const char* text );

	enum {
		NO_ERROR = 0,
		ERROR_ELEMENT_MISMATCH,
		ERROR_PARSING_ELEMENT,
		ERROR_PARSING_ATTRIBUTE,
		ERROR_IDENTIFYING_TAG
	};
	void SetError( int error, const char* str1, const char* str2 );
	
	bool Error() const { return errorID != NO_ERROR; }
	int GetErrorID() const { return errorID; }
	const char* GetErrorStr1() const { return errorStr1; }
	const char* GetErrorStr2() const { return errorStr2; }

	char* Identify( char* p, XMLNode** node );

private:
	XMLDocument( const XMLDocument& );	// not supported
	void operator=( const XMLDocument& );	// not supported
	void InitDocument();

	int errorID;
	const char* errorStr1;
	const char* errorStr2;
	char* charBuffer;

	MemPoolT< sizeof(XMLElement) >	elementPool;
	MemPoolT< sizeof(XMLAttribute) > attributePool;
	MemPoolT< sizeof(XMLText) >		textPool;
	MemPoolT< sizeof(XMLComment) >	commentPool;
};


class XMLStreamer : public XMLVisitor
{
public:
	XMLStreamer( FILE* file );
	~XMLStreamer()	{}

	void OpenElement( const char* name );
	void PushAttribute( const char* name, const char* value );
	void CloseElement();

	void PushText( const char* text, bool cdata=false );
	void PushComment( const char* comment );

	virtual bool VisitEnter( const XMLDocument& /*doc*/ )			{ return true; }
	virtual bool VisitExit( const XMLDocument& /*doc*/ )			{ return true; }

	virtual bool VisitEnter( const XMLElement& element, const XMLAttribute* attribute );
	virtual bool VisitExit( const XMLElement& element );

	virtual bool Visit( const XMLText& text );
	virtual bool Visit( const XMLComment& comment );


private:
	void SealElement();
	void PrintSpace( int depth );
	void PrintString( const char* );	// prints out, after detecting entities.

	FILE* fp;
	int depth;
	bool elementJustOpened;
	int textDepth;

	enum {
		ENTITY_RANGE = 64
	};
	bool entityFlag[ENTITY_RANGE];

	DynArray< const char*, 10 > stack;
};


};	// tinyxml2



#endif // TINYXML2_INCLUDED