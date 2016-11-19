#include "StdAfx.h"
#include "HtmlParser.h"

inline HtmlParser::iterator HtmlParser::ConvertLatinSet(iterator b, iterator e, char& c)
	{
	iterator cur = b;
	iterator set_e = ++cur;
	while (set_e < e && *set_e != ';' && !gimmesilver::IsSpace(*set_e))
		++set_e;

	if (set_e != e && !gimmesilver::IsSpace(*set_e))
		{
		int code = 0;
		if (*cur == '#')
			{
			++cur;
			code = atoi(std::string(cur, set_e).c_str());
			}
		else
			{
			static gimmesilver::TypeString latinSet[] = {
				{ 34, "quot" }, { 38, "amp" }, { 60, "lt"}, { 62, "gt" }, { 160, "nbsp" }, 
				{ 161, "iexcl" }, { 162, "cent" }, { 163, "pound" }, { 164, "curren" }, { 165, "yen" },
				{ 166, "brvbar" }, { 166, "brkbar" }, { 167, "sect" }, { 168, "uml" }, { 168, "die" }, { 169, "copy" }, { 170, "ordf" }, 
				{ 171, "laquo" }, { 172, "not" }, { 173, "shy" }, { 174, "reg" }, { 175, "macr" }, { 175, "hibar" },
				{ 176, "deg" }, { 177, "plusmn" }, { 178, "sup2" }, { 179, "sup3" }, { 180, "acute" },
				{ 181, "micro" }, { 182, "para" }, { 183, "middot" }, { 184, "cedil" }, { 185, "sup1" },
				{ 186, "ordm" }, { 187, "raquo" }, { 188, "frac14" }, { 189, "frac12" }, { 190, "frac34" },
				{ 191, "iquest" }, { 192, "Agrave" }, { 193, "Aacute" }, { 194, "Acirc" }, { 195, "Atilde" },
				{ 196, "Auml" }, { 197, "Aring" }, { 198, "AElig" }, { 199, "Ccedil" }, { 200, "Egrave" },
				{ 201, "Eacute" }, { 202, "Ecirc" }, { 203, "Euml" }, { 204, "Igrave" }, { 205, "Iacute" }, 
				{ 206, "Icird" }, { 207, "Iuml" }, { 208, "ETH" }, { 209, "Ntilde" }, { 210, "Ograve" },
				{ 211, "Oacute" }, { 212, "Ocirc" }, { 213, "Otilde" }, { 214, "Ouml" }, { 215, "times" },
				{ 216, "Oslash" }, { 217, "Ugrave" }, { 218, "Uacute" }, { 219, "Ucirc" }, { 220, "Uuml" },
				{ 221, "Yacute" }, { 222, "THORN" }, { 223, "szlig" }, { 224, "agrave" }, { 225, "aacute" },
				{ 226, "acirc" }, { 227, "atilde" }, { 228, "auml" }, { 229, "aring" }, { 230, "aelig" },
				{ 231, "ccedil" }, { 232, "egrave" }, { 233, "eacute" }, { 234, "ecirc" }, { 235, "euml" },
				{ 236, "igrave" }, { 237, "iacute" }, { 238, "icirc" }, { 239, "iuml" }, { 240, "eth" },
				{ 241, "ntilde" }, { 242, "ograve" }, { 243, "oacute" }, { 244, "ocirc" }, { 245, "otilde" },
				{ 246, "ouml" }, { 247, "divide" }, { 248, "oslash" }, { 249, "ugrave" }, { 250, "uacute" },
				{ 251, "ucirc" }, { 252, "uuml" }, { 253, "yacute" }, { 254, "thorn" }, { 255, "yuml" },
			};
			code = gimmesilver::GetTypeForString(std::string(cur, set_e), latinSet, COUNT_OF(latinSet));
			}

		if (code > 0)
			{
			/// ascii 범위를 넘는 값들은 white space처리...
			if (code > 127)
				c = ' ';
			else
				c = code;
			assert(*set_e == ';');
			return set_e;
			}
		}

	c = *b;
	return b;
	}

bool HtmlParser::Parse(const char* html, int dataSize)
	{
	Clear();
	iterator b = html, e = html+dataSize;
	for (; b < e; ++b)
		{
		switch (state_)
			{
			case INIT:
				if (*b == '<')
					state_ = TAG;
				else if (isBody_)
					{
					char c = *b;
					if (c == '&')
						b = ConvertLatinSet(b, e, c);
					if (!gimmesilver::IsSpace(c) || plainText_.empty() || !gimmesilver::IsSpace(*plainText_.rbegin()))
						plainText_ += c;
					}
				break;

			case TAG:
				if (*b == '!' && (b+2 < e) && *(b+1) == '-' && *(b+2) == '-')
					{
					state_ = COMMENT;
					b += 2;
					}
				else
					b = Tag(b, e); 
				break;

			case COMMENT:
				if (*b == '-' && (b+2 < e) && *(b+1) == '-' && *(b+2) == '>')
					{
					state_ = INIT;
					b += 2;
					}
				break;

			default: assert(false);
			}
		}

	return true;
	}

/*
형식: "<" tag *(" " attr=value) ">"
*/
HtmlParser::iterator HtmlParser::Tag(iterator b, iterator e)
	{
	iterator prev = b;
	while (b < e && !gimmesilver::IsSpace(*b) && *b != '>' && *b != '<')
		++b;

	if (*b == '<' || b == e)
		return b;

	iterator tag_e = b;
	while (tag_e < e && *tag_e != '>')
		++tag_e;

	AttrListType attrList;
	std::string val;
	switch (GetTagType(prev, b))
		{
		case A: 
			if (parserType_ & LINK_URL)
				{
				b = ParseAttribute(b, tag_e, attrList);
				val = GetAttrValue(attrList, HREF);
				if (!val.empty())
					urlList_.push_back(val);
				}
			break;

		case FRAME:
			if (parserType_ & FRAME_SRC)
				{
				b = ParseAttribute(b, tag_e, attrList);
				val = GetAttrValue(attrList, SRC);
				if (!val.empty())
					frame_.push_back(val);
				}
			break;

		case SCRIPT: tag_e = RemoveScript(b, e); break;
		case BODY: 
			if (parserType_ & BODY_TEXT)
				isBody_ = true;
			break;
		case BODY_END:
			if (isBody_)
				isBody_ = false;
			break;
		default: break;
		}

	state_ = INIT;
	return tag_e;
	}

int HtmlParser::GetTagType(iterator b, iterator e)
	{
	static gimmesilver::TypeString tagList[] = {
			{ A, "a" },
			{ FRAME, "frame" },
			{ SCRIPT, "script" },
			{ BODY, "body" },
			{ BODY_END, "/body" },
		};

	return gimmesilver::GetTypeForString(std::string(b, e), tagList, COUNT_OF(tagList));
	}

HtmlParser::iterator HtmlParser::ParseAttribute(iterator b, iterator e, AttrListType& attrList)
	{
	static gimmesilver::TypeString attrName[] = {
		HREF, "href",
		SRC, "src",
	};

	assert(*e == '>');
	while (b < e)
		{
		while (b < e && gimmesilver::IsSpace(*b))
			++b;

		iterator prev = b;
		while (b < e && *b != '=' && !gimmesilver::IsSpace(*b))
			++b;

		int attrType = gimmesilver::GetTypeForString(std::string(prev, b), attrName, COUNT_OF(attrName));
		while (b < e && (*b == '=' || gimmesilver::IsSpace(*b)))
			++b;

		if (b == e)
			break;

		std::string value;	/// 인용부호는 미포함
		iterator val_b;
		if (*b == '"' || *b == '\'')
			{
			char sep = *b;
			val_b = ++b;
			while (b < e && *b != sep)
				++b;
			value.assign(val_b, b++);
			}
		else
			{
			val_b = b;
			while (b < e && !gimmesilver::IsSpace(*b))
				++b;
			value.assign(val_b, b);
			}

		if (attrType != gimmesilver::UNKNOWN)
			attrList.push_back(std::make_pair(attrType, value));
		}

	return e;
	}

std::string HtmlParser::GetAttrValue(const AttrListType& attrList, AttrType attr)
	{
	AttrListType::const_iterator b = attrList.begin(), e = attrList.end();
	for (; b != e; ++b)
		{
		if (b->first == attr)
			return b->second;
		}

	return "";
	}

HtmlParser::iterator HtmlParser::RemoveScript(iterator b, iterator e)
	{
	enum ScriptState { S_INIT, S_SCRIPT, S_OUT1, S_OUT2 };

	ScriptState state = S_INIT;
	for (; b < e; ++b)
		{
		switch (state)
			{
			case S_INIT:
				if (*b == '<')
					return b;
				else if (*b == '>')
					state = S_SCRIPT;
				break;

			case S_SCRIPT:
				if (*b == '<')
					state = S_OUT1;
				break;

			case S_OUT1: state = (*b == '/') ? S_OUT2 : S_SCRIPT; break;
			case S_OUT2:
				{
				iterator prev = b;
				while (b < e && isalpha(*b))
					++b;
				if (GetTagType(prev, b) == SCRIPT)
					{
					if (b < e && *b == '>')
						return b;
					}
				else
					{
					state = S_SCRIPT;
					b = prev;
					}
				break;
				}
			}
		}

	return b;
	}

void HtmlParser::Clear()
	{
	state_ = INIT;
	urlList_.clear();
	frame_.clear();
	plainText_.erase();
	isBody_ = false;
	}

