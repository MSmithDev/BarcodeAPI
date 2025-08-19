"""Simple Python wrapper around the BarcodeAPI.org REST interface."""

from __future__ import annotations

import io
import os
from pathlib import Path
from typing import Iterable, Optional, Union
from urllib.parse import quote

try:  # pragma: no cover - used only when requests is unavailable
    import requests
except ModuleNotFoundError:  # pragma: no cover
    class _DummySession:  # minimal stub used for environments without requests
        def get(self, *args, **kwargs):  # pragma: no cover
            raise ModuleNotFoundError("requests library is required")

        def post(self, *args, **kwargs):  # pragma: no cover
            raise ModuleNotFoundError("requests library is required")

        def delete(self, *args, **kwargs):  # pragma: no cover
            raise ModuleNotFoundError("requests library is required")

    class requests:  # type: ignore
        Session = _DummySession


class BarcodeAPI:
    """Client for the BarcodeAPI REST endpoints.

    Parameters
    ----------
    base_url: str
        Base URL of the BarcodeAPI server. Defaults to ``https://barcodeapi.org``.
    session: requests.Session, optional
        Optional :class:`requests.Session` instance to use for requests.
    """

    def __init__(self, base_url: str = "https://barcodeapi.org", session: Optional[requests.Session] = None):
        self.base_url = base_url.rstrip("/")
        self.session = session or requests.Session()

    # ------------------------------------------------------------------
    # Internal helpers
    def _encode_data(self, data: Union[str, int]) -> str:
        return quote(str(data), safe="")

    def _read_file(self, file: Union[str, bytes, os.PathLike, io.BufferedIOBase]) -> bytes:
        if isinstance(file, (str, os.PathLike, Path)):
            with open(file, "rb") as fh:
                return fh.read()
        if isinstance(file, bytes):
            return file
        return file.read()

    # ------------------------------------------------------------------
    # Public API methods
    def generate(
        self,
        data: Union[str, int],
        code_type: str = "auto",
        params: Optional[dict] = None,
        headers: Optional[dict] = None,
    ) -> requests.Response:
        """Generate a barcode for the provided data.

        Parameters
        ----------
        data: str or int
            The data to encode in the barcode.
        code_type: str
            Barcode format to generate, ``"auto"`` by default.
        params: dict, optional
            Additional query parameters for barcode customization.
        headers: dict, optional
            Additional headers to send with the request.

        Returns
        -------
        requests.Response
            The response object. ``response.content`` contains the barcode
            image bytes. Response headers include barcode metadata.
        """

        url = f"{self.base_url}/api/{code_type}/{self._encode_data(data)}"
        resp = self.session.get(url, params=params, headers=headers, stream=True)
        resp.raise_for_status()
        return resp

    def decode(self, image: Union[str, bytes, os.PathLike, io.BufferedIOBase]) -> dict:
        """Decode a barcode image.

        Parameters
        ----------
        image: path-like, bytes or file-like object
            Image containing a barcode. Accepted formats are path strings,
            ``bytes`` objects, or file-like objects opened in binary mode.

        Returns
        -------
        dict
            JSON payload returned by the server.
        """

        data = self._read_file(image)
        files = {"image": ("image.png", data)}
        resp = self.session.post(f"{self.base_url}/decode", files=files)
        resp.raise_for_status()
        return resp.json()

    def bulk_generate(self, csv: Union[str, bytes, os.PathLike, io.BufferedIOBase]) -> bytes:
        """Generate many barcodes using the bulk API.

        The server expects a CSV file whose rows describe the barcodes to
        generate. The response is a ZIP archive containing the generated
        barcodes.

        Parameters
        ----------
        csv: path-like, bytes or file-like object
            CSV file describing the barcodes to generate.

        Returns
        -------
        bytes
            Contents of the returned ZIP archive.
        """

        data = self._read_file(csv)
        files = {"csvFile": ("bulk.csv", data)}
        resp = self.session.post(f"{self.base_url}/bulk", files=files)
        resp.raise_for_status()
        return resp.content

    def get_info(self) -> dict:
        """Fetch server information."""
        resp = self.session.get(f"{self.base_url}/info")
        resp.raise_for_status()
        return resp.json()

    def get_types(self) -> list:
        """Return a list of all supported barcode types."""
        resp = self.session.get(f"{self.base_url}/types")
        resp.raise_for_status()
        return resp.json()

    def get_type(self, type_name: str) -> dict:
        """Return details for a single barcode type."""
        resp = self.session.get(f"{self.base_url}/type", params={"type": type_name})
        resp.raise_for_status()
        return resp.json()

    def get_limiter(self) -> dict:
        """Return rate limit information for the current client."""
        resp = self.session.get(f"{self.base_url}/limiter")
        resp.raise_for_status()
        return resp.json()

    def get_session(self) -> dict:
        """Return session details if the request includes a valid session."""
        resp = self.session.get(f"{self.base_url}/session")
        resp.raise_for_status()
        return resp.json()

    def delete_session(self) -> bool:
        """Delete the current session."""
        resp = self.session.delete(f"{self.base_url}/session")
        resp.raise_for_status()
        return True

    def create_share(self, requests_list: Iterable[str]) -> str:
        """Create a share containing multiple barcode requests.

        Parameters
        ----------
        requests_list: iterable of str
            Each item should be a string representing a request URI
            (e.g. ``"/api/qr/hello"``).

        Returns
        -------
        str
            The share key returned by the server.
        """

        resp = self.session.post(f"{self.base_url}/share", json=list(requests_list))
        resp.raise_for_status()
        return resp.text.strip()

    def get_share(self, key: str) -> dict:
        """Retrieve a previously created share."""
        resp = self.session.get(f"{self.base_url}/share", params={"key": key})
        resp.raise_for_status()
        return resp.json()
