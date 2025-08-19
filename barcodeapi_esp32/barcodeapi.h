#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

struct Response {
    std::string body;
};

class BarcodeAPI {
public:
    using Headers = std::map<std::string, std::string>;
    using Requester = std::function<Response(const std::string& method,
                                             const std::string& url,
                                             const Headers& headers,
                                             const std::string& body)>;

    BarcodeAPI(const std::string& baseUrl = "https://barcodeapi.org",
               const std::string& token = "",
               Requester requester = nullptr);

    void setToken(const std::string& token);

    Response generate(const std::string& data,
                      const std::string& codeType = "auto",
                      const Headers& params = {},
                      const Headers& extraHeaders = {});

    Response decode(const std::string& image);
    Response bulkGenerate(const std::string& csv);

    Response getInfo();
    Response getTypes();
    Response getType(const std::string& typeName);
    Response getLimiter();
    Response getSession();
    Response deleteSession();
    Response createShare(const std::vector<std::string>& requestsList);
    Response getShare(const std::string& key);

    const Headers& headers() const { return headers_; }

private:
    std::string baseUrl_;
    Headers headers_;
    Requester requester_;

    std::string encode(const std::string& s) const;
    Response request(const std::string& method,
                     const std::string& url,
                     const Headers& headers,
                     const std::string& body);
};

