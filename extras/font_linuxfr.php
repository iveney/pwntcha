<?php
   dl("gd.so");
   header("Content-type: image/png");
   $s = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
   $im = imagecreate(strlen($s) * 9, 12);
   $bg = imagecolorallocate($im, 255, 255, 255);
   $fg = imagecolorallocate($im, 0, 0, 0);
   imagestring($im, 5, 0, -3,  $s, $fg);
   imagepng($im);
   imagedestroy($im);
?> 
