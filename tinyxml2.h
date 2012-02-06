#ifndef TINYXML2_INCLUDED
#define TINYXML2_INCLUDED

/*
	TODO
	- const and non-const versions of API
	- memory pool the class construction
	- attribute accessors
	- node navigation
	- handles
	- visit pattern - change streamer?
	- make constructors protected
	- hide copy constructor
	- hide = operator
	- #define to remove mem-pooling, and make thread safe
	- UTF8 support: isAlpha, etc.

	(No reason to ever cast to base)
	XMLBase -> Utility

	XMLNode
		Document
		Pooled
			Element
			Text
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


namespace tinyxml2
{
class XMLDocument;
class XMLElement;
class XMLAttribute;
class XMLComment;
class XMLNode;
class XMLText;

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
	MemPoolT() : root( 0 ), nAlloc( 0 )	{}
	~MemPoolT() {
		// Delete the blocks.
		for( int i=0; i<blockPtrs.Size(); ++i ) {
			delete blockPtrs[i];
		}
	}

	virtual int ItemSize() const	{ return SIZE; }
	int NAlloc() const				{ return nAlloc; }

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
		++nAlloc;
		return result;
	}
	virtual void Free( void* mem ) {
		if ( !mem ) return;
		--nAlloc;
		Chunk* chunk = (Chunk*)mem;
		memset( chunk, 0xfe, sizeof(Chunk) );
		chunk->next = root;
		root = chunk;
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
	int nAlloc;
};


/*
class StringStack
{
public:
	StringStack();
	virtual ~StringStack();

	void Push( const char* str );
	const char* Pop();

	int NumPositive() const { return nPositive; }

private:
	DynArray< char, 50 > mem;
	int nPositive;		// number of strings with len > 0
};
*/

/*
class StringPool
{
public:
	enum { INIT_SIZE=20 };

	StringPool() : size( 0 ) { 
		const char** mem = pool.PushArr( INIT_SIZE );
		memset( (void*)mem, 0, sizeof(char)*INIT_SIZE );
	}
	~StringPool() {}

	const char* Intern( const char* str );

private:
	// FNV hash
	int Hash( const char* s ) {
		#define FNV_32_PRIME ((int)0x01000193)
		int hval = 0;
	    while (*s) {
			hval *= FNV_32_PRIME;
			hval ^= (int)*s++;
		}
		return hval;
	}

	int size;
	DynArray< const char*, INIT_SIZE > pool;		// the hash table
	StringStack store;								// memory for the interned strings
};
*/

class XMLBase
{
public:

public:
	XMLBase() {}
	virtual ~XMLBase() {}

	static const char* SkipWhiteSpace( const char* p )	{ while( isspace( *p ) ) { ++p; } return p; }
	static char* SkipWhiteSpace( char* p )				{ while( isspace( *p ) ) { ++p; } return p; }

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
	inline static int IsAlphaNum( unsigned char anyByte )	{ return ( anyByte <= 127 ) ? isalnum( anyByte ) : 1; }
	inline static int IsAlpha( unsigned char anyByte )		{ return ( anyByte <= 127 ) ? isalpha( anyByte ) : 1; }

	static char* ParseText( char* in, StrPair* pair, const char* endTag, int strFlags );
	static char* ParseName( char* in, StrPair* pair );

private:
};


class XMLNode
{
	friend class XMLDocument;
	friend class XMLElement;
public:
	//void* operator new( size_t size, MemPool* pool );
	//void operator delete( void* mem, MemPool* pool );

	XMLNode* InsertEndChild( XMLNode* addThis );
	virtual void Print( XMLStreamer* streamer );

	const char* Value() const			{ return value.GetStr(); }
	void SetValue( const char* val )	{ value.SetInternedStr( val ); }

	virtual XMLElement* ToElement()		{ return 0; }
	virtual XMLText*	ToText()		{ return 0; }
	virtual XMLComment* ToComment()		{ return 0; }

	XMLNode* FirstChild()	{ return firstChild; }
	XMLElement* FirstChildElement( const char* value=0 );

	// fixme: guarentee null terminator to avoid internal checks
	virtual char* ParseDeep( char* );

	void SetTextParent()		{ isTextParent = true; } 
	bool IsTextParent() const	{ return isTextParent; }
	virtual bool IsClosingElement() const { return false; }

protected:
	XMLNode( XMLDocument* );
	virtual ~XMLNode();
	
	void ClearChildren();

	XMLDocument*	document;
	XMLNode*		parent;
	bool			isTextParent;
	mutable StrPair			value;

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
	virtual void Print( XMLStreamer* streamer );
	const char* Value() { return value.GetStr(); }
	void SetValue( const char* );

	virtual XMLText*	ToText()		{ return this; }

	char* ParseDeep( char* );

protected:
	XMLText( XMLDocument* doc )	: XMLNode( doc )	{}
	virtual ~XMLText()								{}

private:
};


class XMLComment : public XMLNode
{
	friend class XMLBase;
	friend class XMLDocument;
public:
	virtual void Print( XMLStreamer* );
	virtual XMLComment*	ToComment()		{ return this; }

	const char* Value() { return value.GetStr(); }

	char* ParseDeep( char* );

protected:
	XMLComment( XMLDocument* doc );
	virtual ~XMLComment();

private:
};


class XMLAttribute : public XMLBase
{
	friend class XMLElement;
public:
	virtual void Print( XMLStreamer* streamer );

private:
	XMLAttribute( XMLElement* element ) : next( 0 ) {}
	virtual ~XMLAttribute()	{}
	char* ParseDeep( char* p );

	StrPair name;
	StrPair value;
	XMLAttribute* next;
};


class XMLElement : public XMLNode
{
	friend class XMLBase;
	friend class XMLDocument;
public:
	const char* Name() const		{ return Value(); }
	void SetName( const char* str )	{ SetValue( str ); }

	virtual void Print( XMLStreamer* );

	virtual XMLElement* ToElement() { return this; }

	// internal:
	virtual bool IsClosingElement() const { return closing; }
	char* ParseDeep( char* p );

protected:
	XMLElement( XMLDocument* doc );
	virtual ~XMLElement();

private:
	char* ParseAttributes( char* p, bool *closedElement );

	bool closing;
	XMLAttribute* rootAttribute;
	XMLAttribute* lastAttribute;
};


class XMLDocument : public XMLNode
{
	friend class XMLElement;
public:
	XMLDocument(); 
	~XMLDocument();

	int Parse( const char* );
	int Load( const char* );
	int Load( FILE* );

	void Print( XMLStreamer* streamer=0 );

	XMLElement* NewElement( const char* name );

	enum {
		NO_ERROR = 0,
		ERROR_ELEMENT_MISMATCH,
		ERROR_PARSING_ELEMENT,
		ERROR_PARSING_ATTRIBUTE
	};
	void SetError( int error, const char* str1, const char* str2 );
	
	bool Error() const { return errorID != NO_ERROR; }
	int GetErrorID() const { return errorID; }
	const char* GetErrorStr1() const { return errorStr1; }
	const char* GetErrorStr2() const { return errorStr2; }

	char* Identify( char* p, XMLNode** node );

private:

	XMLDocument( const XMLDocument& );	// intentionally not implemented
	void InitDocument();

	int errorID;
	const char* errorStr1;
	const char* errorStr2;
	char* charBuffer;
	//StringStack stringPool;

	MemPoolT< sizeof(XMLElement) >	elementPool;
	MemPoolT< sizeof(XMLAttribute) > attributePool;
	MemPoolT< sizeof(XMLText) >		textPool;
	MemPoolT< sizeof(XMLComment) >	commentPool;
};


class XMLStreamer
{
public:
	XMLStreamer( FILE* file );
	~XMLStreamer()	{}

	void OpenElement( const char* name, bool textParent );
	void PushAttribute( const char* name, const char* value );
	void CloseElement();

	void PushText( const char* text );
	void PushComment( const char* comment );

private:
	void SealElement();
	void PrintSpace( int depth );
	void PrintString( const char* );	// prints out, after detecting entities.
	bool TextOnStack() const { 
		for( int i=0; i<text.Size(); ++i ) { 
			if ( text[i] == 'T' ) 
				return true; 
		} 
		return false; 
	}

	FILE* fp;
	int depth;
	bool elementJustOpened;
	enum {
		ENTITY_RANGE = 64
	};
	bool entityFlag[ENTITY_RANGE];

	DynArray< const char*, 10 > stack;
	DynArray< char, 10 > text;
};


};	// tinyxml2



#endif // TINYXML2_INCLUDED