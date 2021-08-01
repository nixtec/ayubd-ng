<?php

function unix_get_contents($sock)
{
  $data = false;
  $fp = stream_socket_client("unix://$sock", $errno, $errstr);
  if (!$fp) {
    return $data;
    #echo "ERROR: $errno - $errstr<br />\n";
  }

  fwrite($fp, "GETCAPTCHA\n");
  $data = fread($fp, 2048);
  fclose($fp);
  return $data;
}

$captcha_svc_sock = '/vh/p/sock.d/captcha-static-svc.sock';
$resp = unix_get_contents($captcha_svc_sock);
if ($resp) {
  $lines = explode("\n", $resp);
  $cline = end($lines); // captcha line
  list($n, $code) = explode(',', $cline);
  if ($n >= 0 && $code != "") {
    session_start();
    if (!isset($_SESSION['captcha'])) $_SESSION['captcha'] = array();
    $_SESSION['captcha']['code'] = $code;
    header("Content-type: image/png");
    #header("X-Captcha-Choices: " . implode("/", $choices));
    $sapi = "nginx";
    if (stripos($_SERVER["SERVER_SOFTWARE"], "Apache") !== false) { # Apache
      $sapi = "apache";
      #$sendfile_prefix = $_SERVER['DOCUMENT_ROOT'] . "/app/inst/pdf";
      $sendfile_prefix = __DIR__ . "/captcha_dir";
      $accel_prefix = "X-Sendfile";
    } else { # nginx/x.xx.x
      $sapi = "nginx";
      $sendfile_prefix = "/captcha/static";
      $accel_prefix = "X-Accel-Redirect";
    }

    header("$accel_prefix: $sendfile_prefix/$n.png");
    die();
  }
}
http_response_code(400);
die();

?>
