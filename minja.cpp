#include "minja.h"


namespace JsonTokenizer
{
	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

	const char * SkipWhitespaces( const char * _pCurr )
	{
		while( *_pCurr == ' ' || *_pCurr == '\t' || *_pCurr == '\n' )
			++_pCurr;

		return _pCurr;
	}

	bool IsOneOf( char _Val, const char * _Chars )
	{
		while( *_Chars != 0 )
		{
			if( *_Chars == _Val )
				return true;
			else
				++_Chars;
		}

		return false;
	}

	bool IsStringDelimiter( char _Val )
	{
		return (_Val == '"') || (_Val == '\'');
	}

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

	ParseResult ReadNumber( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		const char * pStart = _pCurr;

		// must start with [0-9]+-
		if( (*_pCurr < '0' || *_pCurr > '9') && *_pCurr != '-' && *_pCurr != '+' )
		{ 										
			*_ppEnd = _pCurr; 					
			_Ctx.OnError(pStart, *_ppEnd, "Number must start with [0-9+-]"); 		
			return ParseError;					
		}

		++_pCurr;

		// followed by digits
		while( *_pCurr >= '0' && *_pCurr <= '9' && *_pCurr != 0 )
			++_pCurr;

		// optional dot
		if( *_pCurr == '.' )
		{
			++_pCurr;

			// if dot, then must have at least 1 following digit
			if( (*_pCurr < '0' || *_pCurr > '9') )
			{
				*_ppEnd = _pCurr;
				_Ctx.OnError(pStart, *_ppEnd, "Dot in numbers must be followed by one digit at least");
				return ParseError;
			}

			while( *_pCurr >= '0' && *_pCurr <= '9' && *_pCurr != 0 )
				++_pCurr;
		}

		// optional exponent
		if( *_pCurr == 'e' || *_pCurr == 'E')
		{
			++_pCurr;

			// exponent must be followed by [0-9]+-
			if( (*_pCurr < '0' || *_pCurr > '9') && *_pCurr != '-' && *_pCurr != '+' )
			{
				*_ppEnd = _pCurr;
				_Ctx.OnError(pStart, *_ppEnd, "exponent in numbers must be followed by [0-9+-]");
				return ParseError;
			}

			++_pCurr;

			// and some digits after that
			while( *_pCurr >= '0' && *_pCurr <= '9' && *_pCurr != 0 )
				++_pCurr;
		}

		*_ppEnd = _pCurr;

		_Ctx.OnNumber( pStart, *_ppEnd );

		return ParseOK;
	}

	ParseResult ReadKeyword( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		const char * pStart = _pCurr;

		// read "null" with any case
		if(    (*(_pCurr+0) == 'n' || *(_pCurr+0) == 'N')
			&& (*(_pCurr+1) == 'u' || *(_pCurr+1) == 'U')
			&& (*(_pCurr+2) == 'l' || *(_pCurr+2) == 'L')
			&& (*(_pCurr+3) == 'l' || *(_pCurr+3) == 'L') )
		{
			*_ppEnd = _pCurr + 4;

			_Ctx.OnNull( pStart, *_ppEnd );
			return ParseOK;
		}

		// read "true" with any case
		if(    (*(_pCurr+0) == 't' || *(_pCurr+0) == 'T')
			&& (*(_pCurr+1) == 'r' || *(_pCurr+1) == 'R')
			&& (*(_pCurr+2) == 'u' || *(_pCurr+2) == 'U')
			&& (*(_pCurr+3) == 'e' || *(_pCurr+3) == 'E') )
		{
			*_ppEnd = _pCurr + 4;

			_Ctx.OnTrue( pStart, *_ppEnd );
			return ParseOK;
		}

		// read "false" with any case
		if(    (*(_pCurr+0) == 'f' || *(_pCurr+0) == 'F')
			&& (*(_pCurr+1) == 'a' || *(_pCurr+1) == 'A')
			&& (*(_pCurr+2) == 'l' || *(_pCurr+2) == 'L')
			&& (*(_pCurr+3) == 's' || *(_pCurr+3) == 'S') 
			&& (*(_pCurr+4) == 'e' || *(_pCurr+4) == 'E') )
		{
			*_ppEnd = _pCurr + 5;

			_Ctx.OnFalse( pStart, *_ppEnd );
			return ParseOK;
		}

		// something invalid starting with [tTfFnN] was detected
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Syntax error. Was expecting null, true of false");
			return ParseError;
		}
	}

	ParseResult ReadString( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		const char * pStart = _pCurr;

		// a string always starts with a string delimiter
		if( !IsStringDelimiter(*_pCurr) )
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "String must start with a \" or a '");
			return ParseError;
		}

		do
		{
			++_pCurr;

			// special characters are despecialized with a '\'
			if( *_pCurr == '\\' )
			{
				++_pCurr;

				// the only special characters supported are ["\/bfnrtu]
				if( !IsOneOf( *_pCurr, "\"\\/bfnrtu" ) )
				{
					*_ppEnd = _pCurr;
					_Ctx.OnError(pStart, *_ppEnd, "Unknown special character");
					return ParseError;
				}
			}
			else
			{
				// found a matching closing string delimiter
				if( IsStringDelimiter(*_pCurr) )
				{
					*_ppEnd = ++_pCurr;

					_Ctx.OnString( pStart, *_ppEnd );
					return ParseOK;
				}
			}
		}
		while( *_pCurr != 0 );

		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Reach the end while parsing String");
			return ParseError;
		}
	}

	ParseResult ReadArray( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		const char * pStart = _pCurr;

		_Ctx.OnBeginArray( _pCurr );

		// always starts with an open bracket
		if( *_pCurr != '[' )
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Arrays must start with a [");
			return ParseError;
		}

		const char * pItemEnd;
		ParseResult result = ParseNoMatch;

		do
		{
			if( result == ParseOK)
			{
				// if we already found an item, then a separating comma must be present before the next value
				_pCurr = SkipWhitespaces( _pCurr );
				if( *_pCurr != ',' )
					break;
			}

			_pCurr = SkipWhitespaces( ++_pCurr );

			_Ctx.OnNewArrayItem( _pCurr );

			// try to read any type of value
			result = ReadValue( _Ctx, _pCurr, &pItemEnd );
			_pCurr = pItemEnd;
		}
		while( result == ParseOK );

		if( result == ParseError )
		{
			return ParseError;
		}

		// must be closed properly with a closing bracket
		if( *_pCurr != ']' )
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Arrays must end with a ]");
			return ParseError;
		}

		*_ppEnd = ++_pCurr;

		_Ctx.OnEndArray( *_ppEnd );
		return ParseOK;
	}

	ParseResult ReadValue( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		_pCurr = SkipWhitespaces( _pCurr );

		if( IsStringDelimiter(*_pCurr) )
			return ReadString( _Ctx, _pCurr, _ppEnd );

		if( *_pCurr == '{')
			return ReadObject( _Ctx, _pCurr, _ppEnd );

		if( *_pCurr == '[')
			return ReadArray( _Ctx, _pCurr, _ppEnd );

		if( (*_pCurr >= '0' && *_pCurr <= '9') || *_pCurr == '-' || *_pCurr == '+' )
			return ReadNumber( _Ctx, _pCurr, _ppEnd );

		if( IsOneOf( *_pCurr, "nNtTfF" ) )
			return ReadKeyword( _Ctx, _pCurr, _ppEnd );

		*_ppEnd = _pCurr;
		return ParseNoMatch;
	}

	ParseResult ReadPair( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		const char * pStart = _pCurr;

		const char * pItemEnd;
		const char * pKeyStart = _pCurr;

		ParseResult result = ParseNoMatch;

		_Ctx.OnBeginPair( _pCurr );

		// a pair always starts with a string 
		result = ReadString( _Ctx, _pCurr, &pItemEnd );
		if( result != ParseOK )
		{
			return result;
		}

		_pCurr = SkipWhitespaces( pItemEnd );

		// followed by a ':'
		if( *_pCurr != ':' )
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Key and value must be separated by a : in a pair");
			return ParseError;
		}

		_pCurr = SkipWhitespaces( ++_pCurr );

		// and any valid value
		result = ReadValue( _Ctx, _pCurr, &pItemEnd );
		if( result != ParseOK )
		{
			return result;
		}

		_pCurr = pItemEnd;
		*_ppEnd = pItemEnd;

		_Ctx.OnEndPair( *_ppEnd ); 

		return ParseOK;
	}

	ParseResult ReadObject( TokenProcessor & _Ctx,  const char * _pCurr, const char ** _ppEnd )
	{
		const char * pStart = _pCurr;

		_Ctx.OnBeginObject( _pCurr );

		// an object always starts with an open curly brace
		if( *_pCurr != '{' )
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Object must start with a {");
			return ParseError;
		}

		const char * pItemEnd;
		ParseResult result = ParseNoMatch;

		do
		{
			if( result == ParseOK )
			{
				// if we already found a pair, then a separating comma must be present before the next one
				_pCurr = SkipWhitespaces( _pCurr );
				if( *_pCurr != ',' )
					break;
			}

			_pCurr = SkipWhitespaces( ++_pCurr );

			// break here if we've reached the end of the object
			if( *_pCurr == '}' )
				break;

			// try to read a valid pair
			result = ReadPair( _Ctx, _pCurr, &pItemEnd );

			if( result )
				_pCurr = pItemEnd;
		}
		while( result == ParseOK );

		if( result == ParseError )
		{
			return ParseError;
		}

		// an object always ends with a closing curly brace
		if( *_pCurr != '}' )
		{
			*_ppEnd = _pCurr;
			_Ctx.OnError(pStart, *_ppEnd, "Object must end with a }");
			return ParseError;
		}

		*_ppEnd = ++_pCurr;

		_Ctx.OnEndObject( *_ppEnd );

		return ParseOK;
	}

} //namespace JsonTokenizer


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class JsonDummyNode : public JsonNode
{
public:
	virtual bool IsValid() const										{ return false; }
	virtual bool GetBool() const										{ return false; }
	virtual float GetNumber() const										{ return 0.0f; }
	virtual const char * GetString() const								{ return "! invalid Json node"; }
	virtual bool IsLastChild() const									{ return true; }	
	virtual const JsonNode & operator [] ( size_t _Index ) const		{ return * this; }
	virtual const JsonNode & operator [] ( const char * _pName ) const	{ return * this; }

};

JsonDummyNode s_JsonDummyNode;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

JsonNode::JsonNode()
	: m_Type(JsonNodeType_Unknown)
	, m_pParent(NULL)
	, m_pName(NULL)
{
}

JsonNode::~JsonNode()
{
	if( m_Type == JsonNodeType_Object && m_Value.Children != NULL )
	{
		for( size_t c=0; c<m_Value.Children->size(); ++c )
			delete (*m_Value.Children)[c];

		delete m_Value.Children;
	}

	if( m_Type == JsonNodeType_Array && m_Value.Items != NULL )
	{
		for( size_t c=0; c<m_Value.Children->size(); ++c )
			delete (*m_Value.Children)[c];

		delete m_Value.Children;
	}
}

JsonNodeType JsonNode::GetType() const
{
	return m_Type;
}

JsonNode * JsonNode::GetParent() const
{
	return m_pParent;
}

const char * JsonNode::GetName() const
{
	return m_pName;
}

bool JsonNode::IsValid() const 
{ 
	return true;
}

bool JsonNode::GetBool() const
{
	ASSERT( m_Type == JsonNodeType_Bool, "Wrong node type. Not a bool" );
	return m_Value.Bool;
}

float JsonNode::GetNumber() const
{
	ASSERT( m_Type == JsonNodeType_Number, "Wrong node type. Not a number" );
	return m_Value.Number;
}

const char * JsonNode::GetString() const
{
	ASSERT( m_Type == JsonNodeType_String, "Wrong node type. Not a string" );
	return m_Value.String;
}

const size_t JsonNode::GetNbChildren() const
{
	ASSERT( m_Type == JsonNodeType_Object, "Wrong node type. Not an Object" );
	return m_Value.Children->size();
}

const JsonNode * JsonNode::GetChild( const char * _pName ) const
{
	ASSERT( m_Type == JsonNodeType_Object, "Wrong node type. Not an Object" );

	NodeVector::const_iterator iter = m_Value.Children->begin();
	NodeVector::const_iterator iend = m_Value.Children->end();
	for( ; iter!=iend; ++iter )
		if( !strcmp(_pName, (*iter)->m_pName ) )
			return *iter;

	return NULL;
}

const JsonNode * JsonNode::GetChild( size_t _Index ) const
{
	ASSERT( m_Type == JsonNodeType_Object || m_Type == JsonNodeType_Array, "Wrong node type. Not Object nor Array" );
	ASSERT( _Index < m_Value.Children->size(), "Index out of bounds" );

	return (*m_Value.Items)[_Index ];
}

const JsonNode::NodeVector & JsonNode::GetChildren() const
{
	ASSERT( m_Type == JsonNodeType_Object || m_Type == JsonNodeType_Array, "Wrong node type. Not Object nor Array" );
	return (*m_Value.Children);
}

bool JsonNode::IsLastChild() const
{
	return (m_pParent == NULL) || (*(--(m_pParent->end())) == this);
}

JsonNode::iterator JsonNode::begin()
{
	ASSERT( m_Type == JsonNodeType_Object || m_Type == JsonNodeType_Array, "Wrong node type. Not Object nor Array" );
	return m_Value.Children->begin();
}

JsonNode::const_iterator JsonNode::begin() const
{
	ASSERT( m_Type == JsonNodeType_Object || m_Type == JsonNodeType_Array, "Wrong node type. Not Object nor Array" );
	return m_Value.Children->begin();
}

JsonNode::iterator JsonNode::end()
{
	ASSERT( m_Type == JsonNodeType_Object || m_Type == JsonNodeType_Array, "Wrong node type. Not Object nor Array" );
	return m_Value.Children->end();
}

JsonNode::const_iterator JsonNode::end() const
{
	ASSERT( m_Type == JsonNodeType_Object || m_Type == JsonNodeType_Array, "Wrong node type. Not Object nor Array" );
	return m_Value.Children->end();
}

const JsonNode & JsonNode::operator [] ( size_t _Index ) const
{
	const JsonNode * pRet = GetChild( _Index );
	return pRet ? *pRet : s_JsonDummyNode;
}

const JsonNode & JsonNode::operator [] ( const char * _pName ) const
{
	const JsonNode * pRet = GetChild( _pName );
	return pRet ? *pRet : s_JsonDummyNode;
}

JsonNode * JsonNode::CreateNode( const char * _pName, JsonNodeType _Type )
{
	JsonNode * pNode = new JsonNode();
	ASSERT( pNode, "Could not create a new node" );

	pNode->m_pParent = this;
	if( _pName )
	{
		size_t len = strlen(_pName);
		pNode->m_pName = new char [len + 1];
		memcpy( pNode->m_pName, _pName, len );
		pNode->m_pName[len] = 0;
	}

	pNode->m_Type = _Type;
	if( _Type == JsonNodeType_Array || _Type == JsonNodeType_Object )
		pNode->m_Value.Children = new NodeVector;

	m_Value.Children->push_back( pNode );

	return pNode;
}


JsonNode * JsonNode::AddNull( const char * _pName )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_Null );

	pNode->m_Type = JsonNodeType_Null;
	pNode->m_Value.String = NULL;

	return pNode;
}

JsonNode * JsonNode::AddBool( const char * _pName, bool _Value )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_Bool );

	pNode->m_Type = JsonNodeType_Bool;
	pNode->m_Value.Bool = _Value;

	return pNode;
}

JsonNode * JsonNode::AddNumber( const char * _pName, float _Value )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_Number );

	pNode->m_Value.Number = _Value;

	return pNode;
}

JsonNode * JsonNode::AddString( const char * _pName, const char * _pValue )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );
	ASSERT( _pValue, "Cannot set a NULL string" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_String );

	size_t len = strlen(_pValue);
	pNode->m_Value.String = new char [len + 1];
	memcpy( pNode->m_Value.String, _pValue, len );
	pNode->m_Value.String[len] = 0;

	return pNode;
}

JsonNode * JsonNode::AddString( const char * _pName, const char * _pBegin, size_t _Len )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );
	ASSERT( _pBegin, "Cannot set a NULL string" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_String );

	pNode->m_Value.String = new char [_Len + 1];
	memcpy( pNode->m_Value.String, _pBegin, _Len );
	pNode->m_Value.String[_Len] = 0;

	return pNode;
}

JsonNode * JsonNode::AddArray( const char * _pName )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_Array );

	return pNode;
}

JsonNode * JsonNode::AddObject( const char * _pName )
{
	ASSERT( (_pName && m_Type == JsonNodeType_Object) || (!_pName && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );

	JsonNode * pNode = CreateNode( _pName, JsonNodeType_Object );

	return pNode;
}

void JsonNode::AttachNode( JsonNode * _pNode )
{
	ASSERT( (_pNode->GetName() && m_Type == JsonNodeType_Object) || (!_pNode->GetName() && m_Type == JsonNodeType_Array), "Wrong node type/name combination" );
	ASSERT( _pNode, "Cannot add a NULL node" );

	m_Value.Children->push_back( _pNode );
}

bool JsonNode::Visit( JsonNodeVisitor & _Visitor )
{
	bool keepGoing;
	switch( m_Type )
	{
	case JsonNodeType_Null: 
		keepGoing = _Visitor.OnNull( this ); 
		goto visitEarlyOut;

	case JsonNodeType_Bool: 
		keepGoing = _Visitor.OnBool( this ); 
		goto visitEarlyOut;

	case JsonNodeType_Number: 
		keepGoing = _Visitor.OnNull( this ); 
		goto visitEarlyOut;

	case JsonNodeType_String: 
		keepGoing = _Visitor.OnString( this ); 
		goto visitEarlyOut;

	case JsonNodeType_Array: 
		keepGoing = _Visitor.OnArrayBegin( this );
		if( !keepGoing )
			goto visitEarlyOut;
		for( size_t c=0; c<m_Value.Children->size(); ++c )
		{
			keepGoing = (*m_Value.Children)[c]->Visit( _Visitor );
			if( !keepGoing )
				goto visitEarlyOut;
		}
		keepGoing = _Visitor.OnArrayEnd( this ); 
		goto visitEarlyOut;

	case JsonNodeType_Object: 
		keepGoing = _Visitor.OnObjectBegin( this ); 
		for( size_t c=0; c<m_Value.Children->size(); ++c )
		{
			keepGoing = (*m_Value.Children)[c]->Visit( _Visitor );
			if( !keepGoing )
				goto visitEarlyOut;
		}
		keepGoing = _Visitor.OnObjectEnd( this ); 
		goto visitEarlyOut;

	case JsonNodeType_Unknown:
	default:
		break;
	}

visitEarlyOut:
	return keepGoing;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

JsonDocument::JsonDocument()
	: m_pCurrPair( NULL )
	, m_pCurrObject( NULL )
	, m_pName( NULL )
	, m_bUseNextStringAsKey( true )
{
	m_Type = JsonNodeType_Object;
	m_Value.Children = new JsonNode::NodeVector;
	m_pTempName[0] = 0;
}

JsonDocument * JsonDocument::Parse( const char * _pBuffer )
{
	JsonDocument * pDoc = new JsonDocument;

	const char * pParseEnd;
	if( ReadObject( *pDoc, _pBuffer, &pParseEnd ) )
	{
		return pDoc;
	}
	else
	{
		delete pDoc;
		return NULL;
	}
}

JsonDocument * JsonDocument::Create()
{
	JsonDocument * pDoc = new JsonDocument;

	return pDoc;
}

void JsonDocument::OnBeginObject( const char * _pParam1 )
{
	if( m_pCurrObject == NULL )
	{
		m_pCurrObject = this;
		m_pCurrPair = NULL;
	}
	else
	{
		m_pCurrObject = m_pCurrObject->AddObject( m_pName );
	}
}

void JsonDocument::OnEndObject( const char * _pParam1 )
{
	m_pCurrObject = m_pCurrObject->GetParent();
}

void JsonDocument::OnBeginArray( const char * _pParam1 )
{
	m_pCurrPair = m_pCurrObject->AddArray( m_pName );
	m_pCurrObject = m_pCurrPair;
}

void JsonDocument::OnEndArray( const char * _pParam1 )
{
	m_pCurrObject = m_pCurrObject->GetParent();
}

void JsonDocument::OnNewArrayItem( const char * _pParam1 )
{
	m_bUseNextStringAsKey = false;
	m_pName = NULL;
}

void JsonDocument::OnBeginPair( const char * _pParam1 )
{
	m_bUseNextStringAsKey = true;
	m_pName = NULL;
}

void JsonDocument::OnEndPair( const char * _pParam1 )
{
}

void JsonDocument::OnString( const char * _pParam1, const char * _pParam2 )
{
	if( m_bUseNextStringAsKey )
	{
		size_t len = _pParam2 - _pParam1 - 2;
		ASSERT( len < MaxKeyName, "Name too long. Consider a shorter name of changing MaxKeyName" );

		memcpy(m_pTempName, _pParam1+1, len );
		m_pTempName[len] = 0;
		m_pName = m_pTempName;
		m_bUseNextStringAsKey = false;
	}
	else
	{
		size_t len = _pParam2 - _pParam1 - 2;
		m_pCurrPair = m_pCurrObject->AddString( m_pName, _pParam1 + 1, _pParam2 - _pParam1 - 2 );
	}
}

void JsonDocument::OnNumber( const char * _pParam1, const char * _pParam2 )
{
	char text[64];
	size_t size = _pParam2 - _pParam1;
	memcpy( text, _pParam1, size );
	text[size] = 0;

	m_pCurrPair = m_pCurrObject->AddNumber( m_pName, (float) atof(text) );
}

void JsonDocument::OnNull( const char * _pParam1, const char * _pParam2 )
{
	m_pCurrPair = m_pCurrObject->AddNull( m_pName );
}

void JsonDocument::OnTrue( const char * _pParam1, const char * _pParam2 )
{
	m_pCurrPair = m_pCurrObject->AddBool( m_pName, true );
}

void JsonDocument::OnFalse( const char * _pParam1, const char * _pParam2 )
{
	m_pCurrPair = m_pCurrObject->AddBool( m_pName, false );
}

void JsonDocument::OnError( const char * _pParam1, const char * _pParam2, const char * _pParam3 )
{
	const int snippetLength = 64;
	char message[snippetLength];

	size_t len = _pParam2 - _pParam1;
	len = (len > snippetLength-1) ? snippetLength-1 : len;
	const char * pStart = _pParam2 - len;

	memset( message, 0, snippetLength );
	memcpy( message, pStart, len );

	Log( "Json", "%s. Location: %s<-Here!", _pParam3, message );
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
