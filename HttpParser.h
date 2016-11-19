#ifndef ___HTTPPARSER_H___
#define ___HTTPPARSER_H___

class HttpParser
	{
	public:
		HttpParser() : state_(INIT), chunkState_(CHUNK_INIT), repCode_(0), contentsLen_(0), chunkSize_(0), method_(NONE) {}
		HttpParser(const char* data, size_t size) 
			: state_(INIT), chunkState_(CHUNK_INIT), repCode_(0), contentsLen_(0), chunkSize_(0), method_(NONE) { Parse(data, size); }
		virtual ~HttpParser() {}

		enum Method { NONE, GET, POST, RESPONSE, INVALID };
		void Parse(const char* data, size_t size);

		int GetRepCode() const { return repCode_; }
		Method GetMethod() const { return method_; }

		const std::string& GetHost() const { return host_; }
		const std::string& GetLocation() const { return location_; }
		const std::string& GetBody() const { return body_; }
		const std::vector<std::string>& GetCookie() const { return cookie_; }

		bool IsOK() { return repCode_ >= 200 && repCode_ < 300; }
		bool IsRedirect() { return repCode_ >= 300 && repCode_ < 400; }
		bool IsPartial() { return state_ != INIT; }
		void Clear();

	private:
		enum State { INIT, HEADER_PARTIAL, BODY_PARTIAL };
		enum ChunkState { CHUNK_INIT, CHUNK_PARTIAL };

		void ParseHeader(const char* b, const char* e);
		bool ParseField(const char* b, const char* e);
		Method ParseMethod(const char* b, const char* e);
		void ParseBody(const char* b, const char* e);
		void ParseChunk(const char* data, const char* e);

		const char* GetChunkSize(const char* b, const char* e, int& len);
		const char* AppendBody(const char* b, const char* e);

		State state_;
		ChunkState chunkState_;

		unsigned int repCode_;
		int contentsLen_;
		int chunkSize_;

		Method method_;

		std::string host_;
		std::string location_;
		std::string body_;
		std::string buf_;

		std::vector<std::string> cookie_;

		bool isChunk_;

		/// for debugging
		//std::string dump_;
	};
	
#endif

