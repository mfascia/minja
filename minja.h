//------------------------------------------------------------------------------
//  
//  Copyright (c) 2004-2012, Marc Fascia
//  All rights reserved.
//  
//  Permission to use, copy, modify, and distribute this software and its 
//  documentation for any purpose and without fee is hereby granted, 
//  provided that the above copyright notice appear in all copies and that 
//  both the above copyright notice and this permission notice and warranty 
//  disclaimer appear in supporting documentation.
//  
//  AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
//	ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL 
//	AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR 
//	ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
//	IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT 
//	OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//  
//------------------------------------------------------------------------------

#ifndef __JSON_PARSER__
#define __JSON_PARSER__


#include <vector>


/// \todo:
/// - support for \u in strings
/// - support for comments? /* and */ make most sense as JSON will most likely be condensed in 1 liners
/// - check uniqueness of keys when adding pairs


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include <assert.h>
#define ASSERT( cond, msg )		assert( (cond) && (msg) )
#define ASSERT_TRUE( cond )		assert( (cond) )
#define ASSERT_FALSE( cond )	assert( !(cond) )
#define ASSERT_EQ( a, b )		assert( (a) == (b) )
#define ASSERT_NE( a, b )		assert( (a) != (b) )
// ##__VA_ARGS__ swallows the trailing comma when no variadic args are passed (e.g. Log("cat", "msg"))
// - standard variadic macros require at least one, which MSVC alone doesn't enforce; gcc/clang
// have both adopted this token-paste trick as the portable workaround
#define Log( cat, msg, ... )	printf( msg, ##__VA_ARGS__ )


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// minja targets memory constrained platforms (see the 0-allocation tokenizer layer), so the
// storage type used for JSON numbers is a build choice rather than hardcoded: define
// MINJA_USE_FLOAT before including this header to trade precision for 4 bytes per number
// instead of 8. Defaults to double, which matches the JSON spec's numbers more faithfully.
#ifdef MINJA_USE_FLOAT
typedef float JsonNumber;
#else
typedef double JsonNumber;
#endif


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//--- Forward declarations
class JsonNode;
class JsonNodeVisitor;
class JsonDocument;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

namespace JsonTokenizer
{
	class TokenProcessor
	{
	public:
		// how many nested arrays/objects ReadArray/ReadObject will accept before bailing out with
		// a ParseError instead of recursing forever and blowing the native stack
		enum { MaxNestingDepth = 200 };

		TokenProcessor() : m_NestingDepth(0) {}

		virtual void OnBeginObject( const char * _pParam1 ) {}
		virtual void OnEndObject( const char * _pParam1 ) {}
		virtual void OnBeginArray( const char * _pParam1 ) {}
		virtual void OnEndArray( const char * _pParam1 ) {}
		virtual void OnNewArrayItem( const char * _pParam1 ) {}
		virtual void OnBeginPair( const char * _pParam1 ) {}
		virtual void OnEndPair( const char * _pParam1 ) {}
		virtual void OnString( const char * _pParam1, const char * _pParam2 ) {}
		virtual void OnNumber( const char * _pParam1, const char * _pParam2 ) {}
		virtual void OnNull( const char * _pParam1, const char * _pParam2 ) {}
		virtual void OnTrue( const char * _pParam1, const char * _pParam2 ) {}
		virtual void OnFalse( const char * _pParam1, const char * _pParam2 ) {}
		virtual void OnError( const char * _pParam1, const char * _pParam2, const char * _pParam3 ) {}

		int m_NestingDepth;
	};

	enum ParseResult
	{
		ParseError,
		ParseNoMatch,
		ParseOK,
	};

	const char * SkipWhitespaces( const char * _pCurr );
	bool IsOneOf( char _Val, const char * _Chars );
	bool IsStringDelimiter( char _Val );

	ParseResult ReadKeyword( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
	ParseResult ReadArray( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
	ParseResult ReadPair( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
	ParseResult ReadObject( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
	ParseResult ReadString( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
	ParseResult ReadNumber( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
	ParseResult ReadValue( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd );
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

enum JsonNodeType
{
	JsonNodeType_Unknown,
	JsonNodeType_Null,
	JsonNodeType_Bool,
	JsonNodeType_Number,
	JsonNodeType_String,
	JsonNodeType_Object,
	JsonNodeType_Array
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class JsonNode
{
protected:
	typedef std::vector<JsonNode *>		NodeVector;

public:
	typedef NodeVector::iterator		iterator;
	typedef NodeVector::const_iterator	const_iterator;

protected:
	JsonNodeType m_Type;
	JsonNode * m_pParent;
	char * m_pName;

	union
	{
		char * String;
		bool Bool;
		JsonNumber Number;
		NodeVector * Children;
		NodeVector * Items;
	} m_Value;

public:
	JsonNode();
	virtual ~JsonNode();

	JsonNodeType GetType() const;
	JsonNode * GetParent() const;
	const char * GetName() const;
	virtual bool IsValid() const;

	virtual bool GetBool() const;
	virtual JsonNumber GetNumber() const;
	virtual const char * GetString() const;

	virtual const size_t GetNbChildren() const;
	virtual const JsonNode * GetChild( const char * _pName ) const;
	virtual const JsonNode * GetChild( size_t _Index ) const;
	virtual const NodeVector & GetChildren() const;
	virtual bool IsLastChild() const;

	virtual iterator begin();
	virtual const_iterator begin() const;
	virtual iterator end();
	virtual const_iterator end() const;

	virtual const JsonNode & operator [] ( size_t _Index ) const;
	virtual const JsonNode & operator [] ( const char * _pName ) const;

	JsonNode * AddNull( const char * _pName );
	JsonNode * AddBool( const char * _pName, bool _Value );
	JsonNode * AddNumber( const char * _pName, JsonNumber _Value );
	JsonNode * AddString( const char * _pName, const char * _pValue );
	JsonNode * AddString( const char * _pName, const char * _pBegin, size_t _Len );
	JsonNode * AddArray( const char * _pName );
	JsonNode * AddObject( const char * _pName );

	void AttachNode( JsonNode * _pNode );

	bool Visit( JsonNodeVisitor & _Visitor );

protected:
	JsonNode * CreateNode( const char * _pName, JsonNodeType _Type );

private:
	// JsonNode owns raw pointers (m_pName, string/children storage) - copying it would alias
	// and eventually double-free that storage, so copy is disabled rather than implemented
	JsonNode( const JsonNode & _Rhs );
	JsonNode & operator = ( const JsonNode & _Rhs );
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class JsonNodeVisitor 
{
public:
	virtual ~JsonNodeVisitor() {}
	virtual bool OnNull( JsonNode * _pNode ) { return true; }
	virtual bool OnBool( JsonNode * _pNode ) { return true; }
	virtual bool OnNumber( JsonNode * _pNode ) { return true; }
	virtual bool OnString( JsonNode * _pNode ) { return true; }
	virtual bool OnArrayBegin( JsonNode * _pNode ) { return true; }
	virtual bool OnArrayEnd( JsonNode * _pNode ) { return true; }
	virtual bool OnObjectBegin( JsonNode * _pNode ) { return true; }
	virtual bool OnObjectEnd( JsonNode * _pNode ) { return true; }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class JsonDocument : public JsonNode, public JsonTokenizer::TokenProcessor
{
	enum { MaxKeyName = 255 };

protected:
	char m_pTempName[ MaxKeyName + 1 ];
	JsonNode * m_pCurrPair;
	JsonNode * m_pCurrObject;
	char * m_pName;
	bool m_bUseNextStringAsKey;

protected:
	virtual void OnBeginObject( const char * _pParam1 );
	virtual void OnEndObject( const char * _pParam1 );
	virtual void OnBeginArray( const char * _pParam1 );
	virtual void OnEndArray( const char * _pParam1 );
	virtual void OnNewArrayItem( const char * _pParam1 );
	virtual void OnBeginPair( const char * _pParam1 );
	virtual void OnEndPair( const char * _pParam1 );
	virtual void OnString( const char * _pParam1, const char * _pParam2 );
	virtual void OnNumber( const char * _pParam1, const char * _pParam2 );
	virtual void OnNull( const char * _pParam1, const char * _pParam2 );
	virtual void OnTrue( const char * _pParam1, const char * _pParam2 );
	virtual void OnFalse( const char * _pParam1, const char * _pParam2 );
	virtual void OnError( const char * _pParam1, const char * _pParam2, const char * _pParam3 );

public:
	static JsonDocument * Create();
	static JsonDocument * Parse( const char * _pBuffer );

private:
	JsonDocument();

	// see JsonNode - same reasoning, copy is disabled rather than implemented
	JsonDocument( const JsonDocument & _Rhs );
	JsonDocument & operator = ( const JsonDocument & _Rhs );
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


#endif //__JSON_PARSER__
