#!/usr/bin/env php
<?php
// --- Gestion du cookie compteur ---
$counter = 0;
if (isset($_COOKIE['counter'])) {
    $counter = intval($_COOKIE['counter']);
}
$counter++;
setcookie("counter", strval($counter), 0, "/");

// --- Récupération des paramètres GET/POST ---
$params = array_merge($_GET, $_POST);
$params_str = empty($params) ? "Aucun paramètre" : http_build_query($params, '', ', ');

// --- Corps HTML ---
$body = <<<HTML
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>CGI PHP Demo</title>
</head>
<body>
  <h1>CGI PHP en action 🚀</h1>
  <p><strong>Méthode :</strong> {$_SERVER['REQUEST_METHOD']}</p>
  <p><strong>Paramètres :</strong> {$params_str}</p>
  <p><strong>Vous avez visité cette page {$counter} fois.</strong></p>
</body>
</html>
HTML;

// --- En-têtes + body ---
// Attention : en mode CGI pur, on doit imprimer les headers nous-mêmes.
$headers = "Content-Type: text/html\r\n";
$headers .= "Content-Length: " . strlen($body) . "\r\n";
$headers .= "\r\n";

echo $headers;
echo $body;
?>

