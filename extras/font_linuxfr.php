<?php
   if (!extension_loaded('gd')) {
       if (!is_callable("dl") || !dl('gd.so')) {
           http_response_code(500);
           echo "this script requires the gd extension, which is neither loaded nor loadable";
           exit;
       }
   }
   header("Content-type: image/png");
   $s = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
   $im = imagecreate(strlen($s) * 9, 12);
   $bg = imagecolorallocate($im, 255, 255, 255);
   $fg = imagecolorallocate($im, 0, 0, 0);
   imagestring($im, 5, 0, -3,  $s, $fg);
   imagepng($im);
   imagedestroy($im);
