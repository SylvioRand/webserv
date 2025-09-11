#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import json
import random
from urllib.parse import parse_qs

def read_stdin_content(length):
    """Lit le contenu de stdin de manière sécurisée"""
    if length <= 0:
        return ''
    
    try:
        # Lire le contenu progressivement par blocs
        data = b''
        remaining = length
        
        while remaining > 0:
            chunk_size = min(4096, remaining)
            chunk = sys.stdin.buffer.read(chunk_size)
            if not chunk:
                break
            data += chunk
            remaining -= len(chunk)
        
        return data.decode('utf-8')
    
    except Exception as e:
        print(f"Error reading stdin: {e}", file=sys.stderr)
        return ''

def main():
    # Récupérer la méthode de requête
    method = os.environ.get('REQUEST_METHOD')
    
    # Initialiser la réponse
    response = {
        'method': method,
        'message': 'Requête traitée avec succès'
    }

    if method == 'GET':
        EMOJIS = ["🙈", "🔥", "🎯"]
        query_string = os.environ.get("QUERY_STRING", "")
        params = parse_qs(query_string)
        name = params.get("name", [""])[0]
        display_name = name.strip() if name and name.strip() else "Player"

        version = os.environ.get("VERSION", "HTTP/1.1")
        top_header_string = f"""{version} 200 OK\r\n"""
        guess_str = params.get("guess", [None])[0]
        if guess_str is not None:
            try:
                guess = int(guess_str)   # si tu veux un entier
                # guess = float(guess_str)  # si tu veux un nombre décimal
            except ValueError:
                guess = None
        else:
            guess = None

        secret_number = random.randint(1, 10)

        # Générer le message
        if not guess or (isinstance(guess, str) and not guess.strip()):
            message = f"{EMOJIS[2]} You need to enter a guess to play the game."
        elif guess is None:
            message = "No guess yet! Try a number between 1 and 10."
        elif guess < secret_number:
            message = f"{EMOJIS[0]} Too low! The secret number was {secret_number}"
        elif guess > secret_number:
            message = f"{EMOJIS[0]} Too high! The secret number was {secret_number}"
        else:
            message = f"{EMOJIS[1]} Congrats {display_name}! You guessed the secret number {secret_number}"

        # Contenu HTML
        html_content = f"""<html>
<head><title>Number Guess Game</title></head>
<body style="font-family:sans-serif; text-align:center; padding:50px;">
<p>{message}</p>
</body>
</html>"""

        # Envoi headers HTTP
        sys.stdout.write("HTTP/1.0 200 OK\r\n")
        sys.stdout.write("Content-Type: text/html\r\n")
        sys.stdout.write(f"Content-Length: {len(html_content)}\r\n\r\n")
        sys.stdout.write(html_content)
        sys.stdout.flush()

    # Traiter les données POST
    elif method == 'POST':
        # Lire la longueur du corps de manière sécurisée
        try:
            content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        except (ValueError, TypeError):
            content_length = 0
        
        path = os.environ.get('UPLOAD_DIR', "./")
        
        # Créer le répertoire s'il n'existe pas
        if not os.path.exists(path):
            os.makedirs(path, exist_ok=True)

        # Lire le corps de la requête POST de manière sécurisée
        post_body = read_stdin_content(content_length)
        
        # Parser les données POST
        try:
            params = parse_qs(post_body)
        except Exception as e:
            params = {}
            print(f"Error parsing POST data: {e}", file=sys.stderr)

        name = params.get("name", [""])[0].strip() or "Anonymous"
        email = params.get("email", [""])[0].strip() or "No email"
        message_text = params.get("message", [""])[0].strip() or "No message"

        # Générer un nom de fichier unique
        random_suffix = random.randint(10000, 99999)
        safe_name = ''.join(c for c in name if c.isalnum() or c in ('_', '-')).lower()
        filename = f"{safe_name}_{random_suffix}.txt"

        # Écrire dans le fichier
        try:
            with open(os.path.join(path, filename), "w", encoding="utf-8") as f:
                f.write(f"Name: {name}\n")
                f.write(f"Email: {email}\n")
                f.write(f"Message: {message_text}\n")
        except Exception as e:
            print(f"Error writing file: {e}", file=sys.stderr)
            filename = f"error_{random_suffix}.txt"

        # Réponse HTML
        html_content = f"""<html>
<head><title>POST Response</title></head>
<body style="font-family:sans-serif; text-align:center; padding:50px;">
<p>✅ Thank you, <strong>{name}</strong>. Your message has been successfully saved.</p>
<p>File: <em>{filename}</em></p>
</body>
</html>"""

        # Envoi headers HTTP
        sys.stdout.write("HTTP/1.0 200 OK\r\n")
        sys.stdout.write("Content-Type: text/html\r\n")
        sys.stdout.write(f"Content-Length: {len(html_content)}\r\n\r\n")
        sys.stdout.write(html_content)
        sys.stdout.flush()

if __name__ == '__main__':
    main()
