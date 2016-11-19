#ifndef ___HTMLPARSER_H___
#define ___HTMLPARSER_H___

class HtmlParser
	{
	public:
		enum ParserType { NONE = 0, LINK_URL = 0x01, FRAME_SRC = 0x02, BODY_TEXT = 0x04, ALL = LINK_URL|FRAME_SRC|BODY_TEXT };
		typedef const char* iterator;

		HtmlParser(ParserType parserType = ALL) : parserType_(parserType), state_(INIT), isBody_(false) 
			{
			plainText_.reserve(1024*1024);
			}

		~HtmlParser() {}

		bool Parse(const char* html, int dataSize);
		const std::vector<std::string>& ExtractLinkUrlList() const { return urlList_; }
		const std::vector<std::string>& ExtractFrameUrlList() const { return frame_; }
		const std::string& GetPlainText() const { return plainText_; }

	private:
		enum State { INIT, TAG, COMMENT };
		enum TagType { A, FRAME, SCRIPT, BODY, BODY_END };
		enum AttrType { HREF, SRC };

		typedef std::vector<std::pair<int,std::string> > AttrListType;

		iterator Tag(iterator b, iterator e);
		iterator RemoveScript(iterator b, iterator e);
		int GetTagType(iterator b, iterator e);

		iterator ParseAttribute(iterator b, iterator e, AttrListType& attrList);
		std::string GetAttrValue(const AttrListType& attrList, AttrType attr);

		inline iterator ConvertLatinSet(HtmlParser::iterator b, HtmlParser::iterator e, char& c);

		void Clear();

		ParserType parserType_;
		State state_;
		std::vector<std::string> urlList_;
		std::vector<std::string> frame_;
		std::string plainText_;
		bool isBody_;
	};

#endif

