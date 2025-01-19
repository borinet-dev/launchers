#include "stdafx.h"
#include "HttpClient.h"

#include <vector>
#include <curl/curl.h>
#include <openssl/ssl.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "ssleay32MT.lib")
#pragma comment(lib, "libeay32MT.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

std::string HttpClient::ca_cert_bundle_;

struct HttpClient::MemoryResponseBuffer {
	CURL* curl;
	HttpResponseHandler *response_handler;
	bool success;
	std::vector<unsigned char> *buffer;
	HANDLE hFile;
	std::basic_string<TCHAR> filepath;
	DWORD dwErr;
	size_t written_bytes;

	MemoryResponseBuffer(CURL *arg_curl, HttpResponseHandler *arg_response_handler) {
		curl = arg_curl;
		response_handler = arg_response_handler;
		success = false;
		buffer = NULL;
		hFile = NULL;
		dwErr = 0;
		written_bytes = 0;
	}

	MemoryResponseBuffer(CURL *arg_curl, HttpResponseHandler *arg_response_handler, std::vector<unsigned char>* arg_buffer) {
		curl = arg_curl;
		response_handler = arg_response_handler;
		success = false;
		buffer = arg_buffer;
		hFile = NULL;
		dwErr = 0;
		written_bytes = 0;
	}

	~MemoryResponseBuffer()
	{
		if (hFile && (hFile != INVALID_HANDLE_VALUE)) {
			::CloseHandle(hFile);
			hFile = NULL;
		}
	}
};

int HttpClient::globalInit()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	return 0;
}

void HttpClient::setBaseUrl(const std::string& base_url)
{
	base_url_ = base_url;
}

void HttpClient::setBasicAuthorization(const std::string& username, const std::string& password)
{
	basic_username_ = username;
	basic_password_ = password;
}

size_t HttpClient::WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	MemoryResponseBuffer* ctx = (MemoryResponseBuffer*)userp;
	size_t realsize = size * nmemb;

	size_t reqsize = ctx->buffer->capacity() + realsize;
	size_t pos = ctx->buffer->size();
	ctx->buffer->resize(reqsize); 

	memcpy(&(*ctx->buffer)[pos], contents, realsize);
	ctx->written_bytes += realsize;

	ctx->success = true;

	if (ctx->response_handler)
	{
		if (!ctx->response_handler->onHttpResponse(ctx->curl, contents, size, nmemb))
		{
			ctx->success = false;
			return 0;
		}
	}

	return realsize;
}
size_t HttpClient::WriteFileCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	MemoryResponseBuffer* ctx = (MemoryResponseBuffer*)userp;
	size_t realsize = size * nmemb;

	if (!ctx->hFile)
	{
		ctx->hFile = ::CreateFile(ctx->filepath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (!ctx->hFile || (ctx->hFile == INVALID_HANDLE_VALUE)) {
			ctx->dwErr = ::GetLastError();
			return 0;
		}
		ctx->success = true;
	}

	if (ctx->response_handler)
	{
		if (!ctx->response_handler->onHttpResponse(ctx->curl, contents, size, nmemb))
		{
			ctx->success = false;
			return 0;
		}
	}

	DWORD dwWrittenBytes = 0;
	if (!::WriteFile(ctx->hFile, contents, realsize, &dwWrittenBytes, NULL))
	{
		ctx->dwErr = ::GetLastError();
		ctx->success = false;
		return 0;
	}
	ctx->written_bytes += realsize;

	return realsize;
}

std::unique_ptr<CURL, void (*)(CURL*)> HttpClient::createCurl()
{
	std::unique_ptr<CURL, void (*)(CURL*)> curl(curl_easy_init(), curl_easy_cleanup);
	if (curl)
	{
		curl_easy_setopt(curl.get(), CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl.get(), CURLOPT_SSL_CTX_FUNCTION, cbCurlSsl);
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
		if (!basic_username_.empty())
			curl_easy_setopt(curl.get(), CURLOPT_USERNAME, basic_username_.c_str());
		if (!basic_password_.empty())
			curl_easy_setopt(curl.get(), CURLOPT_PASSWORD, basic_password_.c_str());
	}
	return curl;
}

std::unique_ptr<HttpClient::SimpleResponse> HttpClient::simpleRequest(const std::string& url, HttpResponseHandler* response_handler)
{
	std::unique_ptr<SimpleResponse> retval(new SimpleResponse());
	std::unique_ptr<CURL, void (*)(CURL*)> curl = createCurl();
	CURLcode res;
	MemoryResponseBuffer chunk(curl.get(), response_handler, &retval->response_body);

	if (curl) {
		long http_code = 0;
		std::string full_url(base_url_);
		full_url.append(url);

#ifdef _DEBUG
		curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);
#endif

		curl_easy_setopt(curl.get(), CURLOPT_URL, full_url.c_str());

		/* send all data to this function  */
		curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, (void*)&chunk);

		/* some servers don't like requests that are made without a user-agent
		   field, so we provide one */
		curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "autoupdater/1.0");

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl.get());

		curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);

		retval->http_status = http_code;
		retval->written_bytes = chunk.written_bytes;
		retval->sys_error = chunk.dwErr;
		retval->success = chunk.success;

		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		}
	}

	return retval;
}

std::unique_ptr<HttpClient::SimpleResponse> HttpClient::simpleDownload(const std::string& url, const TCHAR* filepath, HttpResponseHandler* response_handler)
{
	std::unique_ptr<SimpleResponse> retval(new SimpleResponse());
	std::unique_ptr<CURL, void (*)(CURL*)> curl = createCurl();
	CURLcode res;
	MemoryResponseBuffer chunk(curl.get(), response_handler);

	chunk.filepath = filepath;

	if (curl) {
		long http_code = 0;
		std::string full_url(base_url_);
		full_url.append(url);

		curl_easy_setopt(curl.get(), CURLOPT_URL, full_url.c_str());

		/* send all data to this function  */
		curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteFileCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, (void*)&chunk);

		/* some servers don't like requests that are made without a user-agent
		   field, so we provide one */
		curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "autoupdater/1.0");

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl.get());

		curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);

		retval->http_status = http_code;
		retval->written_bytes = chunk.written_bytes;
		retval->sys_error = chunk.dwErr;
		retval->success = chunk.success;

		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		}
	}

	return retval;
}

CURLcode HttpClient::cbCurlSsl(CURL* curl, void* sslctx, void* parm)
{
	X509_STORE* pstore;
	int rc;

	if (ca_cert_bundle_.empty())
		return CURLE_OK;

	/* get a pointer to the X509 certificate store (which may be empty) */
	pstore = SSL_CTX_get_cert_store((SSL_CTX*)sslctx);

	{
		STACK_OF(X509_INFO)* inf = NULL;
		const struct iovec* iov;
		X509_INFO* itmp;
		BIO* in = NULL;
		int			 i, count = 0, ok = 0;

		if ((in = BIO_new_mem_buf(ca_cert_bundle_.c_str(), ca_cert_bundle_.length())) == NULL)
			goto done;

		if ((inf = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL)) == NULL)
			goto done;

		for (i = 0; i < sk_X509_INFO_num(inf); i++) {
			itmp = sk_X509_INFO_value(inf, i);
			if (itmp->x509) {
				ok = X509_STORE_add_cert(pstore, itmp->x509);
				if (!ok)
					goto done;
				count++;
			}
			if (itmp->crl) {
				ok = X509_STORE_add_crl(pstore, itmp->crl);
				if (!ok)
					goto done;
				count++;
			}
		}

		ok = count != 0;
	done:
		if (inf != NULL)
			sk_X509_INFO_pop_free(inf, X509_INFO_free);
		if (in != NULL)
			BIO_free(in);
	}

	/* all set to go */
	return CURLE_OK;
}
