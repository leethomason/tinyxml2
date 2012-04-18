/** @mainpage

<h1> TinyXML-2 </h1>

TinyXML is a simple, small, efficient, C++ XML parser that can be 
easily integrated into other programs.

The master is hosted on github:
github.com/leethomason/tinyxml2

The online HTML version of these docs:
http://grinninglizard.com/tinyxml2docs/index.html

Examples are in the "related pages" tab of the HTML docs.

<h2> What it does. </h2>
	
In brief, TinyXML parses an XML document, and builds from that a 
Document Object Model (DOM) that can be read, modified, and saved.

XML stands for "eXtensible Markup Language." It is a general purpose
human and machine readable markup language to describe arbitrary data.
All those random file formats created to store application data can 
all be replaced with XML. One parser for everything.

http://en.wikipedia.org/wiki/XML

There are different ways to access and interact with XML data.
TinyXML-2 uses a Document Object Model (DOM), meaning the XML data is parsed
into a C++ objects that can be browsed and manipulated, and then 
written to disk or another output stream. You can also construct an XML document 
from scratch with C++ objects and write this to disk or another output
stream. You can even use TinyXML-2 to stream XML programmatically from
code without creating a document first.

TinyXML-2 is designed to be easy and fast to learn. It is one header and
one cpp file. Simply add these to your project and off you go. 
There is an example file - xmltest.cpp - to get you started. 

TinyXML-2 is released under the ZLib license, 
so you can use it in open source or commercial code. The details
of the license are at the top of every source file.

TinyXML-2 attempts to be a flexible parser, but with truly correct and
compliant XML output. TinyXML-2 should compile on any reasonably C++
compliant system. It does not rely on exceptions, RTTI, or the STL.

<h2> What it doesn't do. </h2>

TinyXML-2 doesn't parse or use DTDs (Document Type Definitions) or XSLs
(eXtensible Stylesheet Language.) There are other parsers out there 
that are much more fully
featured. But they are also much bigger, take longer to set up in
your project, have a higher learning curve, and often have a more
restrictive license. If you are working with browsers or have more
complete XML needs, TinyXML-2 is not the parser for you.

<h2> TinyXML-1 vs. TinyXML-2 </h2>

Which should you use? TinyXML-2 uses a similar API to TinyXML-1 and the same
rich test cases. But the implementation of the parser is completely re-written
to make it more appropriate for use in a game. It uses less memory, is faster,
and uses far few memory allocations.

TinyXML-2 has no requirement for STL, but has also dropped all STL support. All
strings are query and set as 'const char*'. This allows the use of internal 
allocators, and keeps the code much simpler.

Both parsers:
<ol>
	<li>Simple to use with similar APIs.</li>
	<li>DOM based parser.</li>
	<li>UTF-8 Unicode support. http://en.wikipedia.org/wiki/UTF-8 </li>
</ol>

Advantages of TinyXML-2
<ol>
	<li>The focus of all future dev.</li>	
	<li>Many fewer memory allocation (1/10th to 1/100th), uses less memory (about 40% of TinyXML-1), and faster.</li>
	<li>No STL requirement.</li>
	<li>More modern C++, including a proper namespace.</li>
	<li>Proper and useful handling of whitespace</li>
</ol>

Advantages of TinyXML-1
<ol>
	<li>Can report the location of parsing errors.</li>
	<li>Support for some C++ STL conventions: streams and strings</li>
	<li>Very mature and well debugged code base.</li>
</ol>
	
<h2> Features </h2>

<h3> Memory Model </h3>

An XMLDocument is a C++ object like any other, that can be on the stack, or
new'd and deleted on the heap.

However, any sub-node of the Document, XMLElement, XMLText, etc, can only
be created by calling the appropriate XMLDocument::NewElement, NewText, etc.
method. Although you have pointers to these objects, they are still owned
by the Document. When the Document is deleted, so are all the nodes it contains.

<h3> White Space </h3>

Microsoft has an excellent article on white space: http://msdn.microsoft.com/en-us/library/ms256097.aspx

TinyXML-2 preserves white space in a (hopefully) sane way that is almost complient with the
spec.(TinyXML-1 used a completely outdated model.)

As a first step, all newlines / carriage-returns / line-feeds are normalized to a
line-feed character, as required by the XML spec.

White space in text is preserved. For example:

	<element> Hello,  World</element>

The leading space before the "Hello" and the double space after the comma are 
preserved. Line-feeds are preserved, as in this example:

	<element> Hello again,  
	          World</element>

However, white space between elements is *not* preserved. Although not strictly 
compliant, tracking and reporting inter-element space is awkward, and not normally
valuable. TinyXML-2 sees these as the same XML:

	<document>
	<data>1</data>
	<data>2</data>
	<data>3</data>
	</document>

	<document><data>1</data><data>2</data><data>3</data></document>

<h3> Entities </h3>
TinyXML-2 recognizes the pre-defined "character entities", meaning special
characters. Namely:

	&amp;	&
	&lt;	<
	&gt;	>
	&quot;	"
	&apos;	'

These are recognized when the XML document is read, and translated to there
UTF-8 equivalents. For instance, text with the XML of:

	Far &amp; Away

will have the Value() of "Far & Away" when queried from the XMLText object,
and will be written back to the XML stream/file as an ampersand. 

Additionally, any character can be specified by its Unicode code point:
The syntax "&#xA0;" or "&#160;" are both to the non-breaking space characher. 
This is called a 'numeric character reference'. Any numeric character reference
that isn't one of the special entities above, will be read, but written as a
regular code point. The output is correct, but the entity syntax isn't preserved.

<h3> Printing </h3>

<h4> Print to file </h4>
You can directly use the convenience function:

	XMLDocument doc;
	...
	doc.Save( "foo.xml" );

Or the XMLPrinter class:

	XMLPrinter printer( fp );
	doc.Print( &printer );

<h4> Print to memory </h4>
Printing to memory is supported by the XMLPrinter.

	XMLPrinter printer;
	doc->Print( &printer );
	// printer.CStr() has a const char* to the XML

<h4> Print without an XMLDocument </h4>

When loading, an XML parser is very useful. However, sometimes
when saving, it just gets in the way. The code is often set up
for streaming, and constructing the DOM is just overhead.

The Printer supports the streaming case. The following code
prints out a trivially simple XML file without ever creating
an XML document.

	XMLPrinter printer( fp );
	printer.OpenElement( "foo" );
	printer.PushAttribute( "foo", "bar" );
	printer.CloseElement();

<h2> Examples </h2>

<h4> Load and parse an XML file. </h4>
@verbatim
	/* ------ Example 1: Load and parse an XML file. ---- */	
	{
		XMLDocument doc;
		doc.LoadFile( "dream.xml" );
	}
@endverbatim

<h4> Lookup information. </h4>
@verbatim
	/* ------ Example 2: Lookup information. ---- */	
	{
		XMLDocument doc;
		doc.LoadFile( "dream.xml" );

		// Structure of the XML file:
		// - Element "PLAY"      the root Element, which is the 
		//                       FirstChildElement of the Document
		// - - Element "TITLE"   child of the root PLAY Element
		// - - - Text            child of the TITLE Element
		
		// Navigate to the title, using the convenience function,
		// with a dangerous lack of error checking.
		const char* title = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" )->GetText();
		printf( "Name of play (1): %s\n", title );
		
		// Text is just another Node to TinyXML-2. The more
		// general way to get to the XMLText:
		XMLText* textNode = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" )->FirstChild()->ToText();
		title = textNode->Value();
		printf( "Name of play (2): %s\n", title );
	}
@endverbatim

<h2> Using and Installing </h2>

There are 2 files in TinyXML-2:
<ol>
	<li>tinyxml2.cpp</li>
	<li>tinyxml2.h</li>
</ol>
And additionally a test file:
<ol>
	<li>xmltest.cpp</li>
</ol>

Simply compile and run. There is a visual studio 2010 project included, a simple Makefile, 
an XCode project, and a cmake CMakeLists.txt included to help you. The top of tinyxml.h
even has a simple g++ command line if you are are *nix and don't want to use a build system.

<h2> Documentation </h2>

The documentation is build with Doxygen, using the 'dox' 
configuration file.

<h2> License </h2>

TinyXML-2 is released under the zlib license:

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.

<h2> Contributors </h2>

Thanks very much to everyone who sends suggestions, bugs, ideas, and 
encouragement. It all helps, and makes this project fun.

The original TinyXML-1 has many contributors, who all deserve thanks
in shaping what is a very successful library. Extra thanks to Yves
Berquin and Andrew Ellerton who were key contributors.

TinyXML-2 grew from that effort. Lee Thomason is the original author
of TinyXML-2 (and TinyXML-1) but hopefully TinyXML-2 will be improved
by many contributors.
*/
