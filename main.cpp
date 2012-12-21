#include <conio.h>
#include <assert.h>

#include "minja.h"


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
		memcpy( text, _pParam1+1, size );
		text[size] = 0;

		Log( "Test", "string = %s", text );
	}
	virtual void OnNumber( const char * _pParam1, const char * _pParam2 ) 
	{
		char text[2048];
		int size = _pParam2 - _pParam1;
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
	virtual void OnError( const char * _pParam1, const char * _pParam2 ) 
	{
		Log( "Test", "Error: %s", _pParam2 );
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

void main()
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
		pDoc->Visit( JsonPrinter(true, 2) );

		printf( "%s\n", (*pDoc)["glossary"]["GlossDiv"]["GlossList"]["GlossEntry"]["GlossDef"]["GlossSeeAlso"][1].GetString() );
		printf( "%s\n", (*pDoc)["glossary"]["GlossDiv"]["GlossList"]["_DoesNotExist_"]["GlossDef"]["GlossSeeAlso"][1].GetString() );

		JsonNode::const_iterator iter = pDoc->begin();
		JsonNode::const_iterator iend = pDoc->end();

		for( ; iter!=iend; ++iter )
		{
			printf( "%s\n", (*iter)->GetName() );
		}
	}
	
	JsonDocument * pDoc2 = JsonDocument::Create();
	
	pDoc2->AddString( "first_name", "Marc" );
	pDoc2->AddString( "surname", "Fascia" );
	pDoc2->AddArray( "pets" )	->AddString(NULL, "Chiffon")->GetParent()
								->AddString(NULL, "Yoda")->GetParent()
								->AddString(NULL, "Phoebe")->GetParent()
								->AddString(NULL, "Kuzko")->GetParent()
								->AddString(NULL, "Doki")->GetParent();

	pDoc2->Visit( JsonPrinter(true, 2) );

	printf( "\nAll tests passed successfully\n" );

	getch();
}

