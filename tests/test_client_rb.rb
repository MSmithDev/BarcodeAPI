require 'minitest/autorun'
require_relative '../barcodeapi_rb/barcodeapi'

class TestBarcodeAPI < Minitest::Test
  def test_generate_builds_url
    captured = {}
    requester = lambda do |method, url, headers, body|
      captured[:method] = method
      captured[:url] = url
      Struct.new(:body).new('img')
    end
    client = BarcodeAPI.new(base_url: 'https://example.com', requester: requester)
    client.generate('abc 123')
    assert_equal :get, captured[:method]
    assert_equal 'https://example.com/api/auto/abc%20123', captured[:url]
  end

  def test_decode_posts_image
    captured = {}
    requester = lambda do |method, url, headers, body|
      captured[:method] = method
      captured[:url] = url
      captured[:headers] = headers
      captured[:body] = body
      Struct.new(:body).new('{"text":"123"}')
    end
    client = BarcodeAPI.new(base_url: 'https://example.com', requester: requester)
    result = client.decode('123')
    assert_equal :post, captured[:method]
    assert_equal 'https://example.com/decode', captured[:url]
    assert_match(/image/, captured[:body])
    assert_equal '123', result['text']
  end

  def test_token_header_and_setter
    dummy = ->(*) { Struct.new(:body).new('') }
    client = BarcodeAPI.new(base_url: 'https://example.com', token: 'abc', requester: dummy)
    assert_equal 'Token=abc', client.headers['Authorization']
    client.set_token('xyz')
    assert_equal 'Token=xyz', client.headers['Authorization']
    client.set_token(nil)
    refute client.headers.key?('Authorization')
  end
end
