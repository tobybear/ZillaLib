#
#  ZillaLib
#  Copyright (C) 2010-2016 Bernhard Schelling
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#

use GD;
#use strict;
#use Encode;
use utf8;

unless (@ARGV && ($ARGV[0] =~ /\.tt[fc]$/i) && (-e $ARGV[0]) && $ARGV[1] && ($ARGV[1] =~ /\.png$/i))
{
	print " ZillaLib Font Generator\n=========================\n\nUsage:\n\n$0 \"font file\" \"out file\" [font size] [expand space] [use anti alias]\n".
		"    font file:      .ttc|.ttf file (i.e. \"".$ENV{'windir'}."\\Fonts\\arial.ttf\")\n".
		"    out file:       .png output font image file\n".
		"    font size:      size in points (default 12)\n".
		"    expand space:   expand space around font characters for effects in pixels (default 0)\n".
		"    use anti alias: anti alias rendered font (0 or 1, default 1)\n\n";
	exit;
}

my $font_file = ($ARGV[0] ? $ARGV[0] : $ENV{'windir'}."\\Fonts\\arial.ttf");
#my $font_file = $ENV{'windir'}."\\Fonts\\gautami.ttf";
#my $font_file = $ENV{'windir'}."\\Fonts\\msgothic.ttc";
my $font_size = (int($ARGV[2]) ? int($ARGV[2]) : 12);
my $expand_space = (int($ARGV[3]) ? int($ARGV[3]) : 0);
my $aa = (int($ARGV[4]) ? int($ARGV[4]) : 1);
my $out_file = ($ARGV[1] ? $ARGV[1] : "font.png");

my $fname = $font_file; $fname =~ s/.*[\\\/]//;
print "Creating font $out_file for $fname of size $font_size...\n";

my ($maxabove, $maxbelow, $maxwidth, %widths, %offsets) = (0, 0, 0);
for (0x20 .. 0x7E, 0xA1 .. 0xFF)
{
	my @b = GD::Image->stringFT(($aa?1:-1)*1, $font_file, $font_size*100, 0, 0, 0, chr($_));
	$b[5] = int(100+$b[5]/100.0)-100;
	$b[1] = int(100+$b[1]/100.0)-100+1;
	$b[0] = int(100+$b[0]/100.0)-100;
	$b[2] = int(100+$b[2]/100.0)-100+2;
	#if ($_ >= 0x7F && $_ <= 0xA0) { $b[0] = 0; $b[2] = 1; } 
	#print chr($_)." X = $b[0] to $b[2] -  Y = $b[5] to $b[1]\n";
	$maxabove = $b[5] if $b[5] < $maxabove;
	$maxbelow = $b[1] if $b[1] > $maxbelow;
	#$sumwidth8[int(int(keys(%offsets))/8)] += $b[2]-$b[0] +$expand_space+$expand_space+1;
	$widths{chr($_)} = $b[2]-$b[0];
	$offsets{chr($_)} = 0-$b[0];
	#$sumwidth += ($sumwidth ? 1 : 0) + $expand_space + ($b[2]-$b[0]) + $expand_space;
	#$sumwidth = $sumwidth8[int(int(keys(%offsets))/8)] if $sumwidth8[int(int(keys(%offsets))/8)] > $sumwidth;
	$maxwidth  = $b[2]-$b[0] if $b[2]-$b[0] > $maxwidth;
}
my $lineh = ($expand_space * 2) + $maxbelow - $maxabove + 1;
my $imgwidth = ($maxwidth+$expand_space+$expand_space+1) * 16 + 10;
my $imgheight = $lineh * int((192/16)+0.9999999) - 1;

print "    Metrics: MaxWidth = $maxwidth - MaxHeight = ".($maxbelow - $maxabove)." - Chars = ".int(keys(%widths))."\n";

my $imgFont = new GD::Image($imgwidth , $imgheight, 1);
$imgFont->saveAlpha(1);
$imgFont->alphaBlending(0);
$imgFont->fill(0, 0, 0x7F000000);
$imgFont->alphaBlending(1);
my $color = $imgFont->colorAllocateAlpha(255,255,255,0);

my ($x, $liney, $online, $maximgwidth, $index) = (0, 0, 0, 0);
for (sort keys(%widths))
{
	if ($x + $expand_space + $widths{$_} + $expand_space + 2 >= $imgwidth)
	{
		### find most high and most low pixel row
		my ($ya, $yb) = ($liney, $liney+$lineh-1);
		while (1) { for ($x=0;$x<$imgwidth;$x++) { if ($imgFont->getPixel($x, $ya) != 0x7F000000) { $x = -1; last; } } if ($x != -1) { $ya++; } else { last; } }
		while (1) { for ($x=0;$x<$imgwidth;$x++) { if ($imgFont->getPixel($x, $yb) != 0x7F000000) { $x = -1; last; } } if ($x != -1) { $yb--; } else { last; } }

		### fill out fully transparent rows inside character with almost transparent pixels (like in chars with umlaut)
		for ($y=$ya+1;$y<=$yb;$y++)
		{
			for ($x=0;$x<$imgwidth;$x++) { if ($imgFont->getPixel($x, $y) != 0x7F000000) { $x = -1; last; } }
			if ($x != -1) { for ($x=0;$x<$imgwidth;$x++) { if ($imgFont->getPixel($x, $y-1) != 0x7F000000) { $imgFont->setPixel($x, $y, 0x7D808080); } } }
		}

		$x = 0; $liney += $lineh; $online = 1;
	}
	#if ($online++ == 16) { $x = 0; $liney += $lineh; $online = 1; }

	$x += $expand_space;
	#print ++$index." - ".ord($_)." $_ X = $x - liney = $liney - offset: ".$offsets{$_}."\n";

	if ($_ eq " ") { $imgFont->filledRectangle($x, $liney + $expand_space, $x + $widths{$_}, $liney + $expand_space + $maxbelow - $maxabove - 1, $color); }
	elsif (ord($_) >= 0x7F && ord($_) <= 0xA0) { $imgFont->line($x, $liney + $expand_space, $x, $liney + $expand_space + $maxbelow - $maxabove - 1, $color); }
	else { $imgFont->stringFT(($aa?1:-1)*$color, $font_file, $font_size, 0, $x + $offsets{$_}, $liney + $expand_space - $maxabove, $_); }

	my ($xstart, $xoff, $ya, $yb, $y, $doclear) = ($x, 0, $liney + $expand_space, $liney + $expand_space + $maxbelow - $maxabove - 1);
	$x += $widths{$_} + 3;

	### trim left alpha >=0x6F (of 0x7F)
	for ($y = 0; $y != -1 && $xoff < 10; $y != -1 && $xoff++)
	{
		for ($y=$ya;$y<=$yb;$y++) { if ($imgFont->getPixel($xstart+$xoff, $y) < 0x6F000000) { $y = -1; last; } }
	}
	if ($xoff>0) { $imgFont->alphaBlending(0); for my $xx ($xstart + $xoff .. $x-1) { for my $yy ($ya .. $yb) { $imgFont->setPixel($xx-$xoff, $yy, $imgFont->getPixel($xx, $yy)); } } $imgFont->alphaBlending(1); }

	### find most right pixel column and also trim right side alpha >=0x6F (of 0x7F)
	for ($y = 0; $y != -1 && --$x;)
	{
		for ($y=$ya;$y<=$yb;$y++) { my $p = $imgFont->getPixel($x, $y); $doclear = 1 if $p != 0x7F000000; if ($p < 0x6F000000) { $y = -1; last; } }
		if ($y != -1 && $doclear) { $imgFont->alphaBlending(0); $imgFont->line($x, $ya, $x, $yb, 0x7F000000); $imgFont->alphaBlending(1); }
	}

	### draw a filled rectangle for empty characters
	if ($x <= $xstart)
	{
		$x = $xstart + $widths{" "};
		$imgFont->filledRectangle($x-$widths{" "}, $liney + $expand_space, $x, $liney + $expand_space + $maxbelow - $maxabove - 1, $color);
	}

	### fill out fully transparent columns inside character with almost transparent pixels (like in ")
	while (++$xstart!=$x)
	{
		for ($y=$ya;$y<=$yb;$y++) { if ($imgFont->getPixel($xstart, $y) != 0x7F000000) { $y = -1; last; } }
		if ($y != -1) { for ($y=$ya;$y<=$yb;$y++) { if ($imgFont->getPixel($xstart-1, $y) != 0x7F000000) { $imgFont->setPixel($xstart, $y, 0x7D808080); } } }
	}

	$x += 1 + $expand_space + 1;
	$maximgwidth = $x if $x > $maximgwidth;
}

$imgheight = $liney + $lineh;
my $imgFontReal = new GD::Image($maximgwidth - 1, $imgheight, 1);
$imgFontReal->saveAlpha(1);
$imgFontReal->alphaBlending(0);
$imgFontReal->copy($imgFont,0,0,0,0,$maximgwidth - 1,$imgheight);
#exit;

open(OUT, ">".$out_file.""); binmode(OUT); print OUT $imgFontReal->png; close(OUT);
