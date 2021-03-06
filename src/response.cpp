#include "response.hpp"
#include <algorithm> // for std::find


namespace everest {

Response::Response(const std::shared_ptr<internals::Connection>& connection, const bool isClosing)
	: mStatus(200),
	  mConnection(connection),
	  mIsClosing(isClosing)
{

}

Response::~Response()
{
}

void Response::setStatus(const unsigned short status)
{
	mStatus = status;
}

unsigned short Response::getStatus()
{
	return mStatus;
}

void Response::setClosing(bool isClosing)
{
	mIsClosing = isClosing;
}

bool Response::isClosing()
{
	return mIsClosing;
}

headers_t::iterator Response::findHeader(const std::string& key)
{
	return std::find_if(mHeaders.begin(), mHeaders.end(), [&key](auto header) {
		return header.first == key;
	});
}

void Response::setHeader(const std::string& key, const std::string& value)
{
	// Is this key duplicated?
	auto it = findHeader(key);
	if (it != mHeaders.end()) {
		it->second = value;
		return;
	}

	mHeaders.emplace_back(key, value);
}

const std::string Response::getHeader(const std::string& key)
{
	auto it = findHeader(key);
	if (it != mHeaders.end()) {
		return it->second;
	}
	return "";
}

const headers_t& Response::getHeaders()
{
	return mHeaders;
}

void Response::appendBody(const std::string& chunk)
{
	mBodyStream << chunk;
}

std::string Response::getBody()
{
	return mBodyStream.str();
}

void Response::send()
{
	// Set Connection: close header
	if (mIsClosing) {
		setHeader("Connection", "close");
	}

	// Set Content-Length header
	mBodyStream.seekg(0, mBodyStream.end);
	ssize_t size = mBodyStream.tellg();
	setHeader("Content-Length", std::to_string(size));
	mBodyStream.seekg(0, mBodyStream.beg);

	// Connection will handle sending data.
	// Just pass "self" to be send.
	mConnection->send(shared_from_this());
}

} // namespace everest
