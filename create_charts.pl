#!/usr/bin/perl
use strict;
use warnings;
use IO::File;

my $pairs = [
    [1, 10],
    [10, 100],
    [100, 1000],
    [1000, 10000],
    [10000, 100000],
    [100000, 200000],
    [200000, 300000],
];

my @imgs;

for my $p (@{$pairs}) {
    my $s = $p->[0];
    my $e = $p->[1];
    system("./chart.pl $s $e");
    push(@imgs, "<img src=\"benchmark-$s-$e-clients-libev-libevent.png\" />");
}

my $file = IO::File->new("graphs/index.html","w");

$file->write("<html>\n<body>\n");
for my $i (@imgs) {
    $file->write("<br />$i<br /><br />");
}
$file->write("</body>\n</html>\n");




__DATA__
<html>
<body>
<img src="benchmark-1-10-clients-libev-libevent.png" />
<br />
<br />
<br />
<img src="benchmark-10-100-clients-libev-libevent.png" />
<br />
<br />
<br />
<img src="benchmark-100-1000-clients-libev-libevent.png" />
<br />
<br />
<br />
<img src="benchmark-1000-10000-clients-libev-libevent.png" />
<br />
<br />
<br />
<img src="benchmark-10000-100000-clients-libev-libevent.png" />
<br />
<br />
<br />
<img src="benchmark-100000-200000-clients-libev-libevent.png" />
<br />
<br />
<br />
<br />
<img src="benchmark-200000-300000-clients-libev-libevent.png" />
<br />
<body>
<html>

