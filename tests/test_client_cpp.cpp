#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include "../barcodeapi_esp32/barcodeapi.h"

int main() {
    // generate builds URL
    struct Capture { std::string method; std::string url; } cap;
    BarcodeAPI::Requester req = [&](const std::string& method, const std::string& url,
                                    const BarcodeAPI::Headers&, const std::string&) {
        cap.method = method; cap.url = url; return Response{"img"}; };
    BarcodeAPI client("https://example.com", "", req);
    client.generate("abc 123");
    assert(cap.method == "GET");
    assert(cap.url == "https://example.com/api/auto/abc%20123");

    // decode posts image
    struct Capture2 { std::string method; std::string url; std::string body; std::map<std::string,std::string> headers; } cap2;
    BarcodeAPI::Requester req2 = [&](const std::string& method, const std::string& url,
                                     const BarcodeAPI::Headers& headers, const std::string& body) {
        cap2.method = method; cap2.url = url; cap2.body = body; cap2.headers = headers; return Response{"{\"text\":\"123\"}"}; };
    BarcodeAPI client2("https://example.com", "", req2);
    auto resp = client2.decode("123");
    assert(cap2.method == "POST");
    assert(cap2.url == "https://example.com/decode/");
    assert(cap2.headers.count("Content-Type"));
    assert(cap2.headers["Content-Type"].find("multipart/form-data") != std::string::npos);
    assert(cap2.body.find("123") != std::string::npos);
    assert(resp.body.find("123") != std::string::npos);

    // token header and setter
    BarcodeAPI client3("https://example.com", "abc", [](auto, auto, auto, auto){ return Response{""}; });
    assert(client3.headers().at("Authorization") == "Token=abc");
    client3.setToken("xyz");
    assert(client3.headers().at("Authorization") == "Token=xyz");
    client3.setToken("");
    assert(client3.headers().count("Authorization") == 0);

    std::cout << "ok" << std::endl;
    return 0;
}

