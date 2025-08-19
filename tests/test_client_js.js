const test = require('node:test');
const assert = require('node:assert');
const { BarcodeAPI } = require('../barcodeapi_js');

test('generate builds correct url', async () => {
  const realFetch = global.fetch;
  const captured = {};
  global.fetch = async (url) => {
    captured.url = url.toString();
    return new Response('img');
  };
  const client = new BarcodeAPI({ baseUrl: 'https://example.com' });
  await client.generate('abc 123');
  assert.strictEqual(captured.url, 'https://example.com/api/auto/abc%20123');
  global.fetch = realFetch;
});

test('decode posts image', async () => {
  const realFetch = global.fetch;
  const captured = {};
  global.fetch = async (url, options) => {
    captured.url = url;
    captured.method = options.method;
    captured.body = options.body;
    return new Response(JSON.stringify({ code: 200, text: '123', format: 'QR' }), {
      status: 200,
      headers: { 'Content-Type': 'application/json' },
    });
  };
  const client = new BarcodeAPI({ baseUrl: 'https://example.com' });
  const result = await client.decode(Buffer.from('123'));
  assert.strictEqual(result.text, '123');
  assert.strictEqual(captured.url, 'https://example.com/decode');
  assert.ok(captured.body instanceof FormData);
  assert.ok(captured.body.get('image'));
  global.fetch = realFetch;
});

test('token header and setter', () => {
  const client = new BarcodeAPI({ baseUrl: 'https://example.com', token: 'abc' });
  assert.strictEqual(client.headers['Authorization'], 'Token=abc');
  client.setToken('xyz');
  assert.strictEqual(client.headers['Authorization'], 'Token=xyz');
  client.setToken(null);
  assert.ok(!('Authorization' in client.headers));
});
