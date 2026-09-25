#include <assert.h>

#include "minja.h"

// getch() is an MSVC/conio.h-only "press a key to exit" helper - fall back to a plain
// getchar() everywhere else so this still builds and behaves the same on gcc/clang
#ifdef _WIN32
	#include <conio.h>
#else
	#include <cstdio>
	static int getch() { return getchar(); }
#endif


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class ContextDump : public JsonTokenizer::TokenProcessor
{
	virtual void OnBeginObject( const char * _pParam1 ) 
	{
		Log( "Test", "{" );
	}
	virtual void OnEndObject( const char * _pParam1 ) 
	{
		Log( "Test", "}" );
	}
	virtual void OnBeginArray( const char * _pParam1 ) 
	{
		Log( "Test", "[" );
	}
	virtual void OnEndArray( const char * _pParam1 )
	{
		Log( "Test", "]" );
	}
	virtual void OnBeginPair( const char * _pParam1 ) 
	{
		Log( "Test", "Begin Pair" );
	}
	virtual void OnEndPair( const char * _pParam1 ) 
	{
		Log( "Test", "End Pair" );
	}
	virtual void OnString( const char * _pParam1, const char * _pParam2 )
	{
		char text[2048];
		int size = _pParam2 - _pParam1 - 2;
		// clamp before the memcpy - an input string longer than the buffer used to overflow it
		size = (size < (int)sizeof(text)) ? size : (int)sizeof(text) - 1;
		memcpy( text, _pParam1+1, size );
		text[size] = 0;

		Log( "Test", "string = %s", text );
	}
	virtual void OnNumber( const char * _pParam1, const char * _pParam2 )
	{
		char text[2048];
		int size = _pParam2 - _pParam1;
		size = (size < (int)sizeof(text)) ? size : (int)sizeof(text) - 1;
		memcpy( text, _pParam1, size );
		text[size] = 0;

		Log( "Test", "number = %s", text );
	}
	virtual void OnNull( const char * _pParam1, const char * _pParam2 ) 
	{
		Log( "Test", "null" );
	}
	virtual void OnTrue( const char * _pParam1, const char * _pParam2 ) 
	{
		Log( "Test", "true" );
	}
	virtual void OnFalse( const char * _pParam1, const char * _pParam2 ) 
	{
		Log( "Test", "false" );
	}
	virtual void OnError( const char * _pParam1, const char * _pParam2, const char * _pParam3 )
	{
		Log( "Test", "Error: %s", _pParam3 );
	}
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class JsonPrinter : public JsonNodeVisitor 
{
private:
	int m_TabSpace;
	bool m_LineBreaks;
	int m_Level;

private:
	void Indent()
	{
		for( int l=0; l<m_Level*m_TabSpace; ++l )
			printf( " " );
	}

	void PrintName( JsonNode * _pNode )
	{
		Indent(); 

		if( _pNode->GetName() )
		{	
			printf( "'%s': ", _pNode->GetName() );
		}
	}

	void Endline()
	{
		if( m_LineBreaks )
			printf( "\n" );
	}

	void Comma( JsonNode * _pNode )
	{
		if( !_pNode->IsLastChild() )
			printf( "," );
	}

public:
	JsonPrinter( bool _LineBreaks, int _TabSpace )
		: m_TabSpace( _TabSpace )
		, m_LineBreaks( _LineBreaks )
		, m_Level(0)
	{
	}

	virtual bool OnNull( JsonNode * _pNode )		{ PrintName( _pNode );	printf( "null" );									Comma( _pNode );	Endline(); return true; }
	virtual bool OnBool( JsonNode * _pNode )		{ PrintName( _pNode );	printf( _pNode->GetBool() ? "true" : "false" );		Comma( _pNode );	Endline(); return true; }
	virtual bool OnNumber( JsonNode * _pNode )		{ PrintName( _pNode );	printf( "%f", _pNode->GetNumber() );				Comma( _pNode );	Endline(); return true; }
	virtual bool OnString( JsonNode * _pNode )		{ PrintName( _pNode );	printf( "'%s'", _pNode->GetString() );				Comma( _pNode );	Endline(); return true; }
	virtual bool OnArrayBegin( JsonNode * _pNode )	{ PrintName( _pNode );	printf( "[" );										++m_Level;			Endline(); return true; }
	virtual bool OnArrayEnd( JsonNode * _pNode )	{ --m_Level; Indent();	printf( "]" );										Comma( _pNode );	Endline(); return true; }
	virtual bool OnObjectBegin( JsonNode * _pNode ) { PrintName( _pNode );	printf( "{" );										++m_Level;			Endline(); return true; }
	virtual bool OnObjectEnd( JsonNode * _pNode )	{ --m_Level; Indent();	printf( "}" );										Comma( _pNode );	Endline(); return true; }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int main()
{
	JsonTokenizer::ParseResult res;
	const char * pE;
	JsonTokenizer::TokenProcessor C;



	ASSERT_TRUE( JsonTokenizer::IsOneOf('1', "1234") );
	ASSERT_FALSE( JsonTokenizer::IsOneOf('5', "1234") );

	ASSERT_TRUE( JsonTokenizer::IsStringDelimiter('"') );
	ASSERT_TRUE( JsonTokenizer::IsStringDelimiter('\'') );
	ASSERT_FALSE( JsonTokenizer::IsStringDelimiter('a') );



	res = JsonTokenizer::ReadKeyword( C, "tRuE", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadKeyword( C, "fAlsE", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadKeyword( C, "bbbfalse", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadKeyword( C, "NulL", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadKeyword( C, "aanull", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "-123", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "+123", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123.456", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "-123.456", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "+123.456", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123e456", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123e-456", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123e+456", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123.456e789", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123e", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadNumber( C, "123.", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );



	res = JsonTokenizer::ReadString( C, "\"Marc\"", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadString( C, "\"Quote: \\\" ...\"", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadString( C, "\"Backslash: \\\\ ...\"", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadString( C, "\"Newline: \\n ...\"", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadString( C, "\"Bad spec char: \\g ??\"", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );



	const char text [] = "[ 123, 456, 789.0123, true, False, 'Marco', 'Polo',]";

	res = JsonTokenizer::ReadArray( C, text, &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadArray( C, "[  ]", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadArray( C, "[ 'abc', 'def' ]", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	// adding an extra , after the last item is supported for convenience
	res = JsonTokenizer::ReadArray( C, "[ 'abc', 'def', ]", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	// An empty array with just comma(s) is not valid though
	res = JsonTokenizer::ReadArray( C, "[ ,]", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );



	// valid simple JSON
	const char text1 [] =			\
		"{ 							\
			'Name' : 'Marco', 		\
			'Level' : 100, 			\
			'Items' : [ 			\
				123, 				\
				456, 				\
				789.0123, 			\
				true, 				\
				False 				\
			] 						\
		}";

	// invalid simple JSON - missing a comma
	const char text2 [] =			\
		"{ 							\
			'Name' : 'Marco'		\
			'Level' : 100 			\
		}";

	// valid fairly complex JSON
	const char text3 [] =
		"{																											\
			'glossary':	{																							\
				'title': 'example glossary',																		\
				'GlossDiv':	{																						\
					'title': 'S',																					\
					'GlossList': {																					\
						'GlossEntry': {																				\
							'ID': 'SGML',																			\
							'SortAs': 'SGML',																		\
							'GlossTerm': 'Standard Generalized Markup Language',									\
							'Acronym': 'SGML',																		\
							'Abbrev': 'ISO 8879:1986',																\
							'GlossDef':	{																			\
								'para': 'A meta-markup language, used to create markup languages such as DocBook.', \
								'GlossSeeAlso':	[																	\
									'GML',																			\
									'XML'																			\
								]																					\
							},																						\
							'GlossSee': 'markup'																	\
						}																							\
					}																								\
				}																									\
			}																										\
		}";

	const char text4 [] = "{ \"user\" : { \"name\": \"John\", \"surname\": \"Doe\" }, \"age\": 33 }";

	res = JsonTokenizer::ReadObject( C, text1, &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadObject( C, "{}", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadObject( C, text2, &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );

	// trailing , after last elem is supported for convenience
	res = JsonTokenizer::ReadObject( C, "{ 'a': 1, 'b': 2, }", &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadObject( C, "{,}", &pE );
	ASSERT_NE( JsonTokenizer::ParseOK, res );

	res = JsonTokenizer::ReadObject( C, text3, &pE );
	ASSERT_EQ( JsonTokenizer::ParseOK, res );

	JsonDocument * pDoc = JsonDocument::Parse( text3 );

	if( pDoc )
	{
		// Visit() takes JsonNodeVisitor by non-const reference (the printer tracks indent state
		// as it walks) - MSVC alone tolerates binding that to a temporary, so name it instead
		JsonPrinter printer( true, 2 );
		pDoc->Visit( printer );

		printf( "%s\n", (*pDoc)["glossary"]["GlossDiv"]["GlossList"]["GlossEntry"]["GlossDef"]["GlossSeeAlso"][1].GetString() );
		printf( "%s\n", (*pDoc)["glossary"]["GlossDiv"]["GlossList"]["_DoesNotExist_"]["GlossDef"]["GlossSeeAlso"][1].GetString() );

		JsonNode::const_iterator iter = pDoc->begin();
		JsonNode::const_iterator iend = pDoc->end();

		for( ; iter!=iend; ++iter )
		{
			printf( "%s\n", (*iter)->GetName() );
		}
	}

	//--------------------------------------------------------------------------------------------
	// document-level coverage: the tests above only exercise the tokenizer layer via the raw
	// Read* functions - the following go through JsonDocument/JsonNode instead, one case per
	// primitive type, since that's the API real callers actually use
	//--------------------------------------------------------------------------------------------

	// nested object via JsonDocument::Parse (text4 was declared above but never actually used)
	JsonDocument * pDoc4 = JsonDocument::Parse( text4 );
	ASSERT_TRUE( pDoc4 != NULL );
	ASSERT_EQ( JsonNodeType_Object, (*pDoc4)["user"].GetType() );
	ASSERT_TRUE( !strcmp( "John", (*pDoc4)["user"]["name"].GetString() ) );
	ASSERT_TRUE( !strcmp( "Doe", (*pDoc4)["user"]["surname"].GetString() ) );
	ASSERT_EQ( JsonNodeType_Number, (*pDoc4)["age"].GetType() );
	ASSERT_EQ( 33, (int) (*pDoc4)["age"].GetNumber() );
	delete pDoc4;

	// null, parsed rather than hand-built with AddNull - also exercises Visit() on a null-typed
	// node, which used to read an uninitialized bool (see the fixed Visit() bug)
	JsonDocument * pDocNull = JsonDocument::Parse( "{ 'value': null }" );
	ASSERT_TRUE( pDocNull != NULL );
	ASSERT_EQ( JsonNodeType_Null, (*pDocNull)["value"].GetType() );
	delete pDocNull;

	// true/false, parsed
	JsonDocument * pDocBool = JsonDocument::Parse( "{ 'yes': true, 'no': false }" );
	ASSERT_TRUE( pDocBool != NULL );
	ASSERT_EQ( JsonNodeType_Bool, (*pDocBool)["yes"].GetType() );
	ASSERT_TRUE( (*pDocBool)["yes"].GetBool() );
	ASSERT_FALSE( (*pDocBool)["no"].GetBool() );
	delete pDocBool;

	// a number with more significant digits than a float can hold exactly - this used to lose
	// precision when JsonNumber was hardcoded to float; with the default (double) it round-trips
	JsonDocument * pDocNumber = JsonDocument::Parse( "{ 'big': 123456789012345 }" );
	ASSERT_TRUE( pDocNumber != NULL );
	ASSERT_EQ( JsonNodeType_Number, (*pDocNumber)["big"].GetType() );
	ASSERT_EQ( 123456789012345.0, (*pDocNumber)["big"].GetNumber() );
	delete pDocNumber;

	// an escape sequence in a string value - GetString() must return it un-escaped
	JsonDocument * pDocEscaped = JsonDocument::Parse( "{ \"msg\": \"a\\nb\" }" );
	ASSERT_TRUE( pDocEscaped != NULL );
	ASSERT_TRUE( !strcmp( "a\nb", (*pDocEscaped)["msg"].GetString() ) );
	delete pDocEscaped;

	// single-quoted strings all the way through JsonDocument, not just JsonTokenizer::ReadString
	JsonDocument * pDocSingleQuoted = JsonDocument::Parse( "{ 'msg': 'hello' }" );
	ASSERT_TRUE( pDocSingleQuoted != NULL );
	ASSERT_TRUE( !strcmp( "hello", (*pDocSingleQuoted)["msg"].GetString() ) );
	delete pDocSingleQuoted;

	// the two regression tests below intentionally exceed OnNumber/OnString's internal buffers
	// to prove the runtime clamp holds even when the matching ASSERT is compiled out - so they
	// only make sense, and only run, in a release (NDEBUG) build; in debug the ASSERT already
	// covers the same limit, loudly, which is exactly what it's there for
#ifdef NDEBUG

	// regression test for the (fixed) stack buffer overflow in JsonDocument::OnNumber: a number
	// token deliberately longer than its internal 64-byte scratch buffer
	{
		char textLongNumber[300] = "{ 'n': 1";
		for( int i=0; i<200; ++i )
			strcat( textLongNumber, "9" );
		strcat( textLongNumber, " }" );

		JsonDocument * pDocLongNumber = JsonDocument::Parse( textLongNumber );
		ASSERT_TRUE( pDocLongNumber != NULL );
		ASSERT_EQ( JsonNodeType_Number, (*pDocLongNumber)["n"].GetType() );
		delete pDocLongNumber;
	}

	// regression test for the (fixed) stack buffer overflow when a key exceeds MaxKeyName (255)
	{
		char textLongKey[600] = "{ '";
		for( int i=0; i<300; ++i )
			strcat( textLongKey, "k" );
		strcat( textLongKey, "': 1 }" );

		JsonDocument * pDocLongKey = JsonDocument::Parse( textLongKey );
		ASSERT_TRUE( pDocLongKey != NULL );
		delete pDocLongKey;
	}

#endif //NDEBUG

	JsonDocument * pDoc2 = JsonDocument::Create();
	
	pDoc2->AddString( "first_name", "Marc" );
	pDoc2->AddString( "surname", "Fascia" );
	pDoc2->AddArray( "pets" )	->AddString(NULL, "Chiffon")->GetParent()
								->AddString(NULL, "Yoda")->GetParent()
								->AddString(NULL, "Phoebe")->GetParent()
								->AddString(NULL, "Kuzko")->GetParent()
								->AddString(NULL, "Doki")->GetParent();

	// read the hand-built tree back through the JsonNode API (GetChild/operator[]/iterators),
	// not just print it, to cover AddObject/AddArray round-tripping
	ASSERT_EQ( (size_t)3, pDoc2->GetNbChildren() ); // first_name, surname, pets
	ASSERT_TRUE( !strcmp( "Marc", (*pDoc2)["first_name"].GetString() ) );
	ASSERT_TRUE( !strcmp( "Fascia", (*pDoc2)["surname"].GetString() ) );
	ASSERT_EQ( JsonNodeType_Array, (*pDoc2)["pets"].GetType() );
	ASSERT_EQ( (size_t)5, (*pDoc2)["pets"].GetNbChildren() );
	ASSERT_TRUE( !strcmp( "Chiffon", (*pDoc2)["pets"][(size_t)0].GetString() ) );
	ASSERT_TRUE( !strcmp( "Doki", (*pDoc2)["pets"][4].GetString() ) );

	int nbPets = 0;
	JsonNode::const_iterator petIter = (*pDoc2)["pets"].begin();
	JsonNode::const_iterator petIend = (*pDoc2)["pets"].end();
	for( ; petIter!=petIend; ++petIter )
		++nbPets;
	ASSERT_EQ( 5, nbPets );

	JsonPrinter printer2( true, 2 );
	pDoc2->Visit( printer2 );

	printf( "\nAll tests passed successfully\n" );

	getch();

	return 0;
}

