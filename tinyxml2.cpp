#include "tinyxml2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

using namespace tinyxml2;

// --------- CharBuffer ----------- //
/*static*/ CharBuffer* CharBuffer::Construct( const char* in )
{
	size_t len = strlen( in );
	size_t size = len + sizeof( CharBuffer );
	CharBuffer* cb = (CharBuffer*) malloc( size );
	cb->length = len;
	strcpy( cb->mem, in );
	return cb;
}


/*static*/ void CharBuffer::Free( CharBuffer* cb )
{
	free( cb );
}


// --------- XMLNode ----------- //

XMLNode::XMLNode( XMLDocument* doc ) :
	document( doc ),
	parent( 0 ),
	firstChild( 0 ), lastChild( 0 ),
	prev( 0 ), next( 0 )
{

}


XMLNode::~XMLNode()
{
	XMLNode* node=firstChild;
	while( node ) {
		XMLNode* temp = node->next;
		delete node;
		node = temp;
	}
}


XMLNode* XMLNode::InsertEndChild( XMLNode* addThis )
{
	if ( lastChild ) {
		TIXMLASSERT( firstChild );
		TIXMLASSERT( lastChild->next == 0 );
		lastChild->next = addThis;
		addThis->prev = lastChild;
		lastChild = addThis;

		addThis->parent = this;
		addThis->next = 0;
	}
	else {
		TIXMLASSERT( firstChild == 0 );
		firstChild = lastChild = addThis;

		addThis->parent = this;
		addThis->prev = 0;
		addThis->next = 0;
	}
	return addThis;
}


void XMLNode::Print( FILE* fp, int depth )
{
	for( int i=0; i<depth; ++i ) {
		fprintf( fp, "    " );
	}
}


const char* XMLNode::ParseText( char* p, const char* endTag, char** next )
{
	TIXMLASSERT( endTag && *endTag );

	char* start = SkipWhiteSpace( p );
	if ( !start )
		return 0;

	char endChar = *endTag;
	p = start;
	int length = strlen( endTag );

	while ( *p ) {
		if ( *p == endChar ) {
			if ( strncmp( p, endTag, length ) == 0 ) {
				*p = 0;
				*next = p + length;
				return start;
			}
		}
		++p;
	}	
	return 0;
}


// --------- XMLComment ---------- //

XMLComment::XMLComment( XMLDocument* doc ) : XMLNode( doc )
{
}


XMLComment::~XMLComment()
{

}


void XMLComment::Print( FILE* fp, int depth )
{
	XMLNode::Print( fp, depth );
	fprintf( fp, "<!-- %s -->\n", value );
}


char* XMLComment::ParseDeep( char* p )
{
	// Comment parses as text.
	value = ParseText( p, "-->", &p );
	return p;
}


// --------- XMLDocument ----------- //
XMLDocument::XMLDocument() : 
	charBuffer( 0 )
{
}


XMLDocument::~XMLDocument()
{
	delete root;
	delete charBuffer;
}



bool XMLDocument::Parse( const char* p )
{
	charBuffer = CharBuffer::Construct( p );
	XMLNode* node = 0;
	char* q = Identify( charBuffer->mem, &node );
	node->ParseDeep( q );
	return true;
}


void XMLDocument::Print( FILE* fp, int depth ) 
{
	for( XMLNode* node = root->firstChild; node; node=node->next ) {
		node->Print( fp, depth );
	}
}


char* XMLDocument::Identify( char* p, XMLNode** node ) 
{
	XMLNode* returnNode = 0;

	p = XMLNode::SkipWhiteSpace( p );
	if( !p || !*p || *p != '<' )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };
	const char* cdataHeader = { "<![CDATA[" };

	if ( XMLNode::StringEqual( p, xmlHeader, 5 ) ) {
		returnNode = new XMLComment( this );
	}
	else {
		TIXMLASSERT( 0 );
	}

	*node = returnNode;
	return p;
}
