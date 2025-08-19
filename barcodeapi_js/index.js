const { readFile } = require('node:fs/promises');
const { URL } = require('node:url');

class BarcodeAPI {
  constructor({ baseUrl = 'https://barcodeapi.org', token } = {}) {
    this.baseUrl = baseUrl.replace(/\/+$/, '');
    this.headers = {};
    if (token) {
      this.setToken(token);
    } else {
      this.token = null;
    }
  }

  setToken(token) {
    this.token = token || null;
    if (this.token) {
      this.headers['Authorization'] = `Token=${this.token}`;
    } else {
      delete this.headers['Authorization'];
    }
  }

  _encodeData(data) {
    return encodeURIComponent(String(data));
  }

  async _toBlob(file) {
    if (file instanceof Blob) {
      return file;
    }
    if (Buffer.isBuffer(file)) {
      return new Blob([file]);
    }
    if (typeof file === 'string') {
      const data = await readFile(file);
      return new Blob([data]);
    }
    if (file && typeof file.arrayBuffer === 'function') {
      const data = await file.arrayBuffer();
      return new Blob([data]);
    }
    throw new TypeError('Unsupported file type');
  }

  async generate(data, codeType = 'auto', { params = {}, headers = {} } = {}) {
    const url = new URL(`${this.baseUrl}/api/${codeType}/${this._encodeData(data)}`);
    Object.entries(params).forEach(([k, v]) => url.searchParams.set(k, v));
    const resp = await fetch(url, { headers: { ...this.headers, ...headers } });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp;
  }

  async decode(image) {
    const blob = await this._toBlob(image);
    const form = new FormData();
    form.append('image', blob, 'image.png');
    const resp = await fetch(`${this.baseUrl}/decode/`, {
      method: 'POST',
      headers: this.headers,
      body: form,
    });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }

  async bulkGenerate(csv) {
    const blob = await this._toBlob(csv);
    const form = new FormData();
    form.append('csvFile', blob, 'bulk.csv');
    const resp = await fetch(`${this.baseUrl}/bulk/`, {
      method: 'POST',
      headers: this.headers,
      body: form,
    });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return Buffer.from(await resp.arrayBuffer());
  }

  async getInfo() {
    const resp = await fetch(`${this.baseUrl}/info/`, { headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }

  async getTypes() {
    const resp = await fetch(`${this.baseUrl}/types/`, { headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }

  async getType(typeName) {
    const url = new URL(`${this.baseUrl}/type/`);
    url.searchParams.set('type', typeName);
    const resp = await fetch(url, { headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }

  async getLimiter() {
    const resp = await fetch(`${this.baseUrl}/limiter/`, { headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }

  async getSession() {
    const resp = await fetch(`${this.baseUrl}/session/`, { headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }

  async deleteSession() {
    const resp = await fetch(`${this.baseUrl}/session/`, { method: 'DELETE', headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return true;
  }

  async createShare(requestsList) {
    const resp = await fetch(`${this.baseUrl}/share/`, {
      method: 'POST',
      headers: { ...this.headers, 'Content-Type': 'application/json' },
      body: JSON.stringify(Array.from(requestsList)),
    });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.text();
  }

  async getShare(key) {
    const url = new URL(`${this.baseUrl}/share/`);
    url.searchParams.set('key', key);
    const resp = await fetch(url, { headers: this.headers });
    if (!resp.ok) {
      throw new Error('HTTP error');
    }
    return resp.json();
  }
}

module.exports = { BarcodeAPI };
