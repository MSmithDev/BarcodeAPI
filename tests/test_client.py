import types

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from barcodeapi_py.client import BarcodeAPI


class DummyResponse:
    def __init__(self, *, json_data=None, content=b"", headers=None, status_code=200, text=""):
        self._json = json_data
        self.content = content
        self.headers = headers or {}
        self.status_code = status_code
        self.text = text

    def raise_for_status(self):
        if not (200 <= self.status_code < 300):
            raise RuntimeError("HTTP error")

    def json(self):
        return self._json


def test_generate_builds_correct_url():
    captured = {}

    def fake_get(url, *, params=None, headers=None, stream=None):
        captured["url"] = url
        return DummyResponse(content=b"img")

    client = BarcodeAPI(base_url="https://example.com")
    client.session.get = fake_get  # type: ignore[assignment]
    client.generate("abc 123")
    assert captured["url"] == "https://example.com/api/auto/abc%20123"


def test_decode_posts_image():
    captured = {}

    def fake_post(url, *, files=None):
        captured["url"] = url
        captured["files"] = files
        return DummyResponse(json_data={"code": 200, "text": "123", "format": "QR"})

    client = BarcodeAPI(base_url="https://example.com")
    client.session.post = fake_post  # type: ignore[assignment]
    result = client.decode(b"123")
    assert result["text"] == "123"
    assert captured["url"] == "https://example.com/decode/"
    assert "image" in captured["files"]


def test_token_header_and_setter():
    client = BarcodeAPI(base_url="https://example.com", token="abc")
    assert client.session.headers["Authorization"] == "Token=abc"
    client.set_token("xyz")
    assert client.session.headers["Authorization"] == "Token=xyz"
    client.set_token(None)
    assert "Authorization" not in client.session.headers
