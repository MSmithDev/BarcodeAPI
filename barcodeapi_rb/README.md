# barcodeapi-rb

Simple Ruby client for [BarcodeAPI](https://barcodeapi.org). Provides convenient
helpers to generate and decode barcodes and to access metadata endpoints.

## Installation

Add the `barcodeapi_rb` folder to your project or package the `barcodeapi.rb`
file as part of your application.

## Usage

```ruby
require_relative 'barcodeapi_rb/barcodeapi'

client = BarcodeAPI.new(token: 'my-token')

# Generate a QR code
resp = client.generate('hello world', 'qr')
File.binwrite('qr.png', resp.body)

# Decode a barcode image
info = client.decode(File.binread('qr.png'))
puts info['text']
```

## Available Methods

- `generate(data, code_type='auto', params: nil, headers: {})`
- `decode(image)`
- `bulk_generate(csv)`
- `get_info`
- `get_types`
- `get_type(type_name)`
- `get_limiter`
- `get_session`
- `delete_session`
- `create_share(requests_list)`
- `get_share(key)`
- `set_token(token)`

