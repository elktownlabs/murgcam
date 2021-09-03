<?php

// function for writing outlined text
function imagettfstroketext(&$image, $size, $angle, $x, $y, &$textcolor, &$strokecolor, $fontfile, $text, $px) {
    for($c1 = ($x-$px); $c1 <= ($x+$px); $c1++) {
        $c2 = $y + round(sqrt($px*$px - ($x-$c1)*($x-$c1)));
        imagettftext($image, $size, $angle, $c1, $c2, $strokecolor, $fontfile, $text);
        $c3 = $y - round(sqrt($px*$px - ($x-$c1)*($x-$c1)));
        imagettftext($image, $size, $angle, $c1, $c3, $strokecolor, $fontfile, $text);
    }
   return imagettftext($image, $size, $angle, $x, $y, $textcolor, $fontfile, $text);
}

?>
