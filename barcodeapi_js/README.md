# barcodeAPI-js

`barcodeAPI-js` is a minimal Node.js client for the [BarcodeAPI.org](https://barcodeapi.org) REST service. It mirrors the features of the Python wrapper and exposes a simple class for generating and decoding barcodes or querying service metadata.

## Installation

This package is not published to npm. Copy the `barcodeapi_js` directory into your project or install directly from the repository.

## Usage

```javascript
const { BarcodeAPI } = require('./barcodeapi_js');

const api = new BarcodeAPI({ token: 'YOUR_API_TOKEN' }); // token optional

// Generate a barcode and save to disk
(async () => {
  const resp = await api.generate('Hello World', 'qr', { params: { height: 200 } });
  const fs = require('node:fs');
  const buffer = Buffer.from(await resp.arrayBuffer());
  fs.writeFileSync('hello.png', buffer);

  // Decode a barcode image
  const result = await api.decode('hello.png');
  console.log(result.text, result.format);
})();
```

### API Tokens

Pass a token when constructing the client or update it later:

```javascript
const api = new BarcodeAPI({ token: 'my-token' });
api.setToken('new-token'); // clear with null
```

## Available Methods

| Method | Description |
| --- | --- |
| `generate(data, codeType = 'auto', options)` | Generate a barcode image. Returns a `Response` object containing image bytes and metadata. |
| `decode(image)` | Decode a barcode from an image. Returns JSON with `text` and `format`. |
| `bulkGenerate(csv)` | Generate many barcodes at once from a CSV file. Returns ZIP archive bytes. |
| `getInfo()` | Retrieve server information such as uptime and version. |
| `getTypes()` | List all supported barcode types. |
| `getType(name)` | Fetch details for a single barcode type. |
| `getLimiter()` | Return rate‑limit information for the current client. |
| `getSession()` / `deleteSession()` | Inspect or delete the current session. |
| `createShare(list)` / `getShare(key)` | Create or fetch a share that groups multiple barcode requests. |
| `setToken(token)` | Set or clear the API token used for requests. |

For complete API details see [barcodeapi.org/api.html](https://barcodeapi.org/api.html).
