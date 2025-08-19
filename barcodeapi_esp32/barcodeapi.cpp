#include "barcodeapi.h"
#include <sstream>
#include <iomanip>

BarcodeAPI::BarcodeAPI(const std::string& baseUrl, const std::string& token, Requester requester)
    : baseUrl_(baseUrl), requester_(requester) {
    if (!token.empty()) {
        setToken(token);
    }
}

void BarcodeAPI::setToken(const std::string& token) {
    if (!token.empty()) {
        headers_["Authorization"] = "Token=" + token;
    } else {
        headers_.erase("Authorization");
    }
}

std::string BarcodeAPI::encode(const std::string& s) const {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex << std::uppercase;
    for (unsigned char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int(c);
        }
    }
    return escaped.str();
}

Response BarcodeAPI::request(const std::string& method,
                             const std::string& url,
                             const Headers& headers,
                             const std::string& body) {
    if (requester_) {
        return requester_(method, url, headers, body);
    }
#if defined(ARDUINO)
    HTTPClient http;
    for (auto& h : headers) {
        http.addHeader(h.first.c_str(), h.second.c_str());
    }
    http.begin(url.c_str());
    int code;
    if (method == "GET") {
        code = http.GET();
    } else if (method == "POST") {
        code = http.POST((uint8_t*)body.data(), body.size());
    } else if (method == "DELETE") {
        code = http.sendRequest("DELETE", body.c_str(), body.size());
    } else {
        code = -1;
    }
    std::string resp = http.getString().c_str();
    http.end();
    return Response{resp};
#elif defined(ESP_PLATFORM)
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    esp_http_client_handle_t client = esp_http_client_init(&config);
    for (auto& h : headers) {
        esp_http_client_set_header(client, h.first.c_str(), h.second.c_str());
    }
    if (method == "POST") {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, body.c_str(), body.size());
    } else if (method == "DELETE") {
        esp_http_client_set_method(client, HTTP_METHOD_DELETE);
    } else {
        esp_http_client_set_method(client, HTTP_METHOD_GET);
    }
    esp_http_client_perform(client);
    int content_length = esp_http_client_get_content_length(client);
    std::string resp;
    resp.resize(content_length > 0 ? content_length : 0);
    if (content_length > 0) {
        esp_http_client_read(client, resp.data(), content_length);
    }
    esp_http_client_cleanup(client);
    return Response{resp};
#else
    return Response{""};
#endif
}

Response BarcodeAPI::generate(const std::string& data,
                              const std::string& codeType,
                              const Headers& params,
                              const Headers& extraHeaders) {
    std::string url = baseUrl_ + "/api/" + codeType + "/" + encode(data);
    if (!params.empty()) {
        std::string sep = "?";
        for (const auto& kv : params) {
            url += sep + encode(kv.first) + "=" + encode(kv.second);
            sep = "&";
        }
    }
    Headers hdr = headers_;
    hdr.insert(extraHeaders.begin(), extraHeaders.end());
    return request("GET", url, hdr, "");
}

Response BarcodeAPI::decode(const std::string& image) {
    std::string boundary = "----BarcodeAPIBoundary";
    std::ostringstream body;
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"image\"; filename=\"image.png\"\r\n";
    body << "Content-Type: application/octet-stream\r\n\r\n";
    body << image;
    body << "\r\n--" << boundary << "--\r\n";
    Headers hdr = headers_;
    hdr["Content-Type"] = "multipart/form-data; boundary=" + boundary;
    return request("POST", baseUrl_ + "/decode/", hdr, body.str());
}

Response BarcodeAPI::bulkGenerate(const std::string& csv) {
    std::string boundary = "----BarcodeAPIBoundary";
    std::ostringstream body;
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"csvFile\"; filename=\"bulk.csv\"\r\n";
    body << "Content-Type: text/csv\r\n\r\n";
    body << csv;
    body << "\r\n--" << boundary << "--\r\n";
    Headers hdr = headers_;
    hdr["Content-Type"] = "multipart/form-data; boundary=" + boundary;
    return request("POST", baseUrl_ + "/bulk/", hdr, body.str());
}

Response BarcodeAPI::getInfo() {
    return request("GET", baseUrl_ + "/info/", headers_, "");
}

Response BarcodeAPI::getTypes() {
    return request("GET", baseUrl_ + "/types/", headers_, "");
}

Response BarcodeAPI::getType(const std::string& typeName) {
    return request("GET", baseUrl_ + "/type/?type=" + encode(typeName), headers_, "");
}

Response BarcodeAPI::getLimiter() {
    return request("GET", baseUrl_ + "/limiter/", headers_, "");
}

Response BarcodeAPI::getSession() {
    return request("GET", baseUrl_ + "/session/", headers_, "");
}

Response BarcodeAPI::deleteSession() {
    return request("DELETE", baseUrl_ + "/session/", headers_, "");
}

Response BarcodeAPI::createShare(const std::vector<std::string>& requestsList) {
    std::ostringstream body;
    body << "[";
    for (size_t i = 0; i < requestsList.size(); ++i) {
        if (i) body << ",";
        body << requestsList[i];
    }
    body << "]";
    Headers hdr = headers_;
    hdr["Content-Type"] = "application/json";
    return request("POST", baseUrl_ + "/share/", hdr, body.str());
}

Response BarcodeAPI::getShare(const std::string& key) {
    return request("GET", baseUrl_ + "/share/?key=" + encode(key), headers_, "");
}

