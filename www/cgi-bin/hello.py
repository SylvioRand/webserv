#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

# Récupère la query string depuis les variables d'environnement
query_string = os.environ.get("QUERY_STRING", "")
params = parse_qs(query_string)

# Extrait le paramètre 'name' ou utilise une valeur par défaut
name = params.get("name", ["inconnu"])[0]

# En-tête HTTP
print("Content-Type: text/html\r\n")
print("\r\n")

# Corps HTML
print("<html><body>")
print("<h1>Bonjour, {}!</h1>".format(name))
print("</body></html>")

