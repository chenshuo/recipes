#!/usr/bin/python3

import io, os, re, sys
from http import HTTPStatus, server

FILE = None

INDEX = """<!DOCTYPE html>
<meta charset="utf-8">
<title>Log Viewer</title>
<script>
var logBox = null;
var lastOffset = 0;
function initialize() {
  logBox = document.getElementById('log');
  lastOffset = 0;
  update();
}

function update() {
  fetch('/get?offset=' + lastOffset).then(function(response) {
    if (response.ok) {
      return response.text();
    }
  }).then(function(text) {
    lastOffset += text.length;
    logBox.value += text;  // FIXME: escape
    logBox.scrollTop = logBox.scrollHeight;  // Scroll to bottom
    setTimeout(update, 3000);
  });
}
</script>
<body onLoad="initialize();">
<textarea id="log" wrap="off" cols="120" rows="50" readonly="readonly">
</textarea>
"""

# INDEX = None  # Dev mode

class HTTPRequestHandler(server.BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path == '/':
            self.send_OK("text/html", INDEX.encode())
        elif self.path.startswith('/get?'):
            # TODO: convert query string to a dict
            m = re.search('offset=(\\d+)', self.path)
            offset = int(m.group(1)) if m else 0
            m = re.search('length=(\\d+)', self.path)
            length = int(m.group(1)) if m else -1
            FILE.seek(offset)
            body = FILE.read(length)
            self.send_OK("text/plain", body)
        else:
            self.send_error(HTTPStatus.NOT_FOUND, "File not found")

    def send_OK(self, content_type, body):
            self.send_response(HTTPStatus.OK)
            self.send_header("Content-Type", content_type)
            self.send_header('Content-Length', int(len(body)))
            self.end_headers()
            self.wfile.write(body)


def main(argv):
    global FILE, INDEX
    FILE = open(argv[1], 'rb')
    if not INDEX:
        INDEX = open(os.path.splitext(argv[0])[0] + '.html').read()
    server.test(HandlerClass=HTTPRequestHandler)
    

if __name__ == '__main__':
    main(sys.argv)
