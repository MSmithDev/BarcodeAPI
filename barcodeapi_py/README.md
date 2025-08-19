# barcodeAPI-py

`barcodeAPI-py` is a lightweight Python wrapper for the [BarcodeAPI.org](https://barcodeapi.org) REST service. It provides convenient helpers for generating and decoding barcodes and for accessing additional API endpoints such as bulk generation or type information.

## Installation

This library is not published to PyPI. Copy the `barcodeapi_py` directory into your project or install it directly from the repository.

The only dependency is [`requests`](https://requests.readthedocs.io/), which is included with most Python distributions. Install it with:

```bash
pip install requests
```

## Usage

```python
from barcodeapi_py import BarcodeAPI

api = BarcodeAPI()

# Generate a barcode and save it to a file
resp = api.generate("Hello World", code_type="qr", params={"height": 200})
with open("hello.png", "wb") as fh:
    fh.write(resp.content)

# Decode a barcode image
result = api.decode("hello.png")
print(result["text"], result["format"])

# Bulk generation from a CSV file
zip_bytes = api.bulk_generate("requests.csv")
with open("barcodes.zip", "wb") as fh:
    fh.write(zip_bytes)

# Inspect supported types
types = api.get_types()
print("Supported types:", [t["name"] for t in types])
```

## Available Methods

| Method | Description |
| --- | --- |
| `generate(data, code_type="auto", params=None, headers=None)` | Generate a barcode image. Returns a `requests.Response` object containing image bytes and metadata headers. |
| `decode(image)` | Decode a barcode from an image. Returns JSON with `text` and `format` fields. |
| `bulk_generate(csv)` | Generate many barcodes at once from a CSV file. Returns ZIP archive bytes. |
| `get_info()` | Retrieve server information such as uptime and version. |
| `get_types()` | List all supported barcode types. |
| `get_type(type_name)` | Fetch details for a single barcode type. |
| `get_limiter()` | Return rate‑limit information for the current client. |
| `get_session()` / `delete_session()` | Inspect or delete the current session. |
| `create_share(requests_list)` / `get_share(key)` | Create or fetch a share that groups multiple barcode requests. |

For full API documentation please see [barcodeapi.org/api.html](https://barcodeapi.org/api.html).
