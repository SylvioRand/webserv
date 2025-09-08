#!/usr/bin/env python3
import os
import sys
import signal
from urllib.parse import parse_qs

signal.signal(signal.SIGPIPE, signal.SIG_DFL)

def main():
    try:
        # Récupérer les paramètres
        query_string = os.environ.get("QUERY_STRING", "")
        params = parse_qs(query_string)
        name = params.get("name", ["inconnu"])[0]
        connection = os.environ.get("CONNECTION", "Connection: close\r\n\r\n")
        version = os.environ.get("VERSION", "HTTP/1.1")
        top_header_string = f"""{version} 200 OK\r\n"""
        connection_string = f"""{connection}"""
        html_content = f"""<html>
<body>
<h1>Bonjour, {name}!</h1>
<p>Welcome to webserv project from 42 school</p>
<p>This page has been generated with python cgi</p>
</body>
</html>"""

        #headers
        sys.stdout.write(top_header_string)
        sys.stdout.write("Content-Type: text/html\r\n")
        sys.stdout.write(f"Content-Length: {len(html_content)}\r\n")
        sys.stdout.write(connection_string)

        # Body
        sys.stdout.write(html_content)
        sys.stdout.flush()

    except Exception as e:
        # En cas d'erreur, envoyer une réponse HTTP valide
        error_msg = f"Error: {str(e)}"
        sys.stdout.write("HTTP/1.0 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n")
        sys.stdout.write(f"Content-Length: {len(error_msg)}\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write(error_msg)
        sys.stdout.flush()

if __name__ == "__main__":
    main()
