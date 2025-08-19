require 'net/http'
require 'uri'
require 'json'

# Simple Ruby wrapper around the BarcodeAPI.org REST interface.
class BarcodeAPI
  attr_reader :base_url, :headers

  def initialize(base_url: 'https://barcodeapi.org', token: nil, requester: nil)
    @base_url = base_url.sub(/\/$/, '')
    @requester = requester || method(:default_requester)
    @headers = {}
    set_token(token) if token
  end

  def set_token(token)
    if token
      @headers['Authorization'] = "Token=#{token}"
    else
      @headers.delete('Authorization')
    end
  end

  # ---------------------------------------------------------------
  def generate(data, code_type = 'auto', params: nil, headers: {})
    encoded = URI.encode_www_form_component(data.to_s).gsub('+', '%20')
    path = "/api/#{code_type}/#{encoded}"
    request(:get, path, params: params, headers: headers)
  end

  def decode(image)
    data = read_file(image)
    boundary = "----------#{rand(1_000_000)}"
    body = build_multipart({ 'image' => data }, boundary)
    headers = { 'Content-Type' => "multipart/form-data; boundary=#{boundary}" }
    resp = request(:post, '/decode/', headers: headers, body: body)
    JSON.parse(resp.body)
  end

  def bulk_generate(csv)
    data = read_file(csv)
    boundary = "----------#{rand(1_000_000)}"
    body = build_multipart({ 'csvFile' => data }, boundary)
    headers = { 'Content-Type' => "multipart/form-data; boundary=#{boundary}" }
    resp = request(:post, '/bulk/', headers: headers, body: body)
    resp.body
  end

  def get_info
    resp = request(:get, '/info/')
    JSON.parse(resp.body)
  end

  def get_types
    resp = request(:get, '/types/')
    JSON.parse(resp.body)
  end

  def get_type(type_name)
    resp = request(:get, '/type/', params: { type: type_name })
    JSON.parse(resp.body)
  end

  def get_limiter
    resp = request(:get, '/limiter/')
    JSON.parse(resp.body)
  end

  def get_session
    resp = request(:get, '/session/')
    JSON.parse(resp.body)
  end

  def delete_session
    request(:delete, '/session/')
    true
  end

  def create_share(requests_list)
    headers = { 'Content-Type' => 'application/json' }
    resp = request(:post, '/share/', headers: headers, body: requests_list.to_json)
    resp.body.strip
  end

  def get_share(key)
    resp = request(:get, '/share/', params: { key: key })
    JSON.parse(resp.body)
  end

  private

  def read_file(file)
    if file.is_a?(String) && File.exist?(file)
      File.binread(file)
    elsif file.is_a?(IO) || file.respond_to?(:read)
      file.read
    else
      file
    end
  end

  def build_multipart(params, boundary)
    parts = []
    params.each do |name, data|
      parts << "--#{boundary}"
      parts << "Content-Disposition: form-data; name=\"#{name}\"; filename=\"#{name}\""
      parts << 'Content-Type: application/octet-stream'
      parts << ''
      parts << data
    end
    parts << "--#{boundary}--"
    parts << ''
    parts.join("\r\n")
  end

  def request(method, path, params: nil, headers: {}, body: nil)
    url = "#{@base_url}#{path}"
    if params && !params.empty?
      query = URI.encode_www_form(params)
      url += (url.include?('?') ? '&' : '?') + query
    end
    all_headers = @headers.merge(headers)
    @requester.call(method, url, all_headers, body)
  end

  def default_requester(method, url, headers, body)
    uri = URI(url)
    req_class = Net::HTTP.const_get(method.capitalize)
    req = req_class.new(uri)
    headers.each { |k, v| req[k] = v }
    req.body = body if body
    Net::HTTP.start(uri.host, uri.port, use_ssl: uri.scheme == 'https') do |http|
      http.request(req)
    end
  end
end
