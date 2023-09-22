#include <platanium.hpp>
cpr::Response platanium::epic::api::request(const std::string uri, HeaderContainer& headers, METHOD method, const std::string body)
{
	cpr::Url url = cpr::Url(uri);
	cpr::Header req_headers;

	for (auto& header : headers)
	{
		req_headers.insert(std::make_pair(header.first, header.second));
	}

	cpr::Response response;
	cpr::Body req_body;

	if (method != METHOD::GET && body.size() != 0)
	{
		req_body = cpr::Body(body);
	}



	switch (method) {
	case METHOD::GET:
		response = cpr::Get(url, req_headers);
		break;
	case METHOD::PATCH:
		response = cpr::Patch(url, req_headers, req_body);
		break;
	case METHOD::POST:
		
		response = cpr::Post(url, req_headers, req_body);
		break;
	}

	return response;
}