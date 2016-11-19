#ifndef ___QMANAGER_H___
#define ___QMANAGER_H___

#include "UrlParser.h"

class QManager
	{
	public:
		static QManager* CreateObj(int type, int limit, const Url& seed, const gimmesilver::path& file);
		QManager(int limit, const Url& seed, const gimmesilver::path& file) 
			: limit_(limit), linkOutput_(file)
			{ 
			urlHistory_.insert(seed); 
			}

		virtual ~QManager();
		struct QException
			{
			QException(const char* msg = "Q is empty!!!") : msg_(msg) {}
			~QException() throw() {}
			const char* what() { return msg_.c_str(); }

			std::string msg_;
			};

		virtual int Size() const = 0;
		virtual bool Full() const = 0;
		virtual bool Empty() const = 0;
		virtual const Url& Front() const = 0;
		virtual void PopFront() = 0;

		void Insert(const Url& baseUrl, const std::vector<std::string>& urlList);

	protected:
		bool Filter(Url& url);
		virtual void Insert(const std::string& url) = 0;

		int limit_;
		std::set<Url> urlHistory_;

		/// for debugging
		void SaveLinkList(const gimmesilver::path& output);
		gimmesilver::path linkOutput_;
	};

class BfsQ : public QManager
	{
	public:
		BfsQ(int limit, const Url& seed, const gimmesilver::path& file) 
			: QManager(limit, seed, file)
			{
			urlQ_.push_back(seed);
			}

		virtual int Size() const { return urlQ_.size(); }
		virtual bool Full() const { return urlQ_.size() > limit_; }
		virtual bool Empty() const { return urlQ_.empty(); }
		virtual const Url& Front() const;
		virtual void PopFront() { assert(!Empty()); urlQ_.pop_front(); }

	protected:
		void Insert(const std::string& url);

	private:
		std::deque<Url> urlQ_;
	};

#endif

