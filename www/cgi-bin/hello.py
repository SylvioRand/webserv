#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

def main():
    # Récupérer les paramètres
    query_string = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query_string)
    name = params.get("name", ["inconnu"])[0]
    
    # Générer la réponse
    html_content = f"""
    <html>
    <body>
        <h1>Bonjour, {name}!</h1>
        <p>Query string: {query_string}</p>
        <p>Méthode: {os.environ.get('REQUEST_METHOD', 'inconnue')}</p>
    </body>
    </html>
    """
    
    # Headers HTTP CORRECTS
    print("Content-Type: text/html")
    print(f"Content-Length: {len(html_content)}")
    print("Connection: close")
    print("")  # ⚠️ LIGNE VIDE OBLIGATOIRE
    
    # Body
    print(html_content)

if __name__ == "__main__":
    main()
