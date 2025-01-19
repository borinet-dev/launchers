#pragma once

#include <string>
#include <memory>
#include <vector>

#include <curl/curl.h>

class HttpClient
{
private:
	static std::string ca_cert_bundle_;

	std::string base_url_;
	std::string basic_username_;
	std::string basic_password_;
	struct MemoryResponseBuffer;

	static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);
	static size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp);

	static CURLcode cbCurlSsl(CURL* curl, void* sslctx, void* parm);

public:
	struct SimpleResponse {
		bool success;
		int http_status;
		int sys_error;
		std::vector<unsigned char> response_body;
		size_t written_bytes;

		SimpleResponse() {
			success = false;
			http_status = 0;
			sys_error = 0;
			written_bytes = 0;
		}
	};

	class HttpResponseHandler {
	public:
		virtual bool onHttpResponse(CURL* curl, void* contents, size_t size, size_t nmemb) = 0;
	};

	static int globalInit();
	void setBaseUrl(const std::string& base_url);
	void setBasicAuthorization(const std::string& username, const std::string& password);

	std::unique_ptr<CURL, void (*)(CURL*)> createCurl();
	std::unique_ptr<SimpleResponse> simpleRequest(const std::string& url, HttpResponseHandler* response_handler = NULL);
	std::unique_ptr<SimpleResponse> simpleDownload(const std::string& url, const TCHAR *filepath, HttpResponseHandler* response_handler = NULL);
};

