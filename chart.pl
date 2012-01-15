#!/usr/bin/perl
use strict;
use warnings;
use IO::File;
use Data::Dumper;
use Chart::Gnuplot;

my $from = $ARGV[0];
my $to   = $ARGV[1];
die "bad arguments\n " unless($from =~ m/\d{1,10}$/ and $to =~ m/\d{1,10}$/);

my $fh = IO::File->new("./stats", 'r');
my $res = {};
my %clients;
my $tot_events = 0;
my $max_time = 0;

while(my $line = $fh->getline()) {
    chomp( $line );
    my @cols = split(/,/, $line );
    my $rec = {};
    for my $c (@cols) {
        my @elem = split(/=/, $c);
        next unless($elem[0]);
        if($elem[0] eq "tot_events") {
            $tot_events = $elem[1];
        }
        if($elem[0] eq "clients") { 
            $clients{$elem[0]}->{cc}++;
        }
        $rec->{$elem[0]} = $elem[1];
    }

    if($rec->{clients} >= $from and $rec->{clients} <= $to) {
        my $l = $rec->{lib};
        my $f = $rec->{files};
        my $cc = $rec->{clients};
        $max_time = $rec->{tot_time} if($rec->{tot_time} > $max_time);
        $res->{$l}->{$cc}->{$f}->{count}++;
        $res->{$l}->{$cc}->{$f}->{files} += $rec->{files};
        $res->{$l}->{$cc}->{$f}->{clients} += $rec->{clients};
        $res->{$l}->{$cc}->{$f}->{tot_time} += $rec->{tot_time};
    }
}

print "$max_time\n";

#warn Dumper($res);

my $chart = Chart::Gnuplot->new(
    output => "./graphs/benchmark-$from-$to-clients-libev-libevent.png",
    title  => {
        text => "Benchmark libev against Libevent",
        font => 'Helvetica, 26',
    },
    xlabel => "File Descriptor Count",
    ylabel => "Time (secs)",
    xrange => [0, 300000],
    yrange => [0, $max_time + 0.1],
    size => "1.5, 1.4",
    imagesize => "1.2, 1.2",
    tmargin => 4,
    bmargin => 2,
    rmargin => 35,
    legend => {
        position => "outside right",
        width    => 3,
        height   => 5,
        align    => "left",
        order    => "vertical",
        title    => "Legend",
        sample   => {
            length   => 5,
            position => "left",
            spacing  => 2,
        },
        border   => {
            linetype => 1,
            width    => 2,
            color    => "blue",
        },
    },
    grid => {
        type  => 'dash',
        width => 1,
    },

);


my $header = {};
my @graphs;
for my $lib ( keys %{$res} ) {
    for my $cc ( sort { $a <=> $b } keys %{$res->{$lib}}) {
        next unless ($cc >= $from and $cc <= $to);
   
        $header->{$cc} = {'title' => "$cc $lib Clients",
                          'style' => 'lines',
                          'width' => 5,
                         };
        my @table;
        for my $fc ( sort { $a <=> $b } keys %{$res->{$lib}->{$cc}} ) {
            my $tmp = $res->{$lib}->{$cc}->{$fc};
            #push(@table, [$fc, $tmp->{tot_time}/$tmp->{count}]);
            push(@{ $header->{$cc}->{xdata} }, $fc); 
            push(@{ $header->{$cc}->{ydata} }, $tmp->{tot_time}/$tmp->{count}); 
        }
    }
    for my $cc (sort {$a <=> $b} keys %{ $header } ) {
        my $g = $header->{$cc};
        my $obj = Chart::Gnuplot::DataSet->new(%{ $g });
        push(@graphs, $obj);
    }
}

$chart->plot2d(@graphs);

