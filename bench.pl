#!/usr/bin/perl
use strict;
use warnings;
my $max_files = 300000;
my $events = int(10 * 1000000);
my $iterations = 2;
my @libs = ("libevent", "libev");

my $run = 1;
for my $l (@libs) {
    for my $i (1 .. $iterations) {
        print "lib=$l on iterations=$i\n"; 
        my $factor = 1.5;
        my $cc = 1;
        while($run) {
            run_bench($cc, $l, $factor);
            $cc *= 3;
            if($cc > 300000) {
                $cc = 300000;
                $run = 0;
            }
        }
        $run = 1;
    }
}

sub run_bench {
    my ($clients, $library, $factor) = @_;

    my $files  = 1;
    while ($files < $max_files) {
        $files = int($files * $factor + 1);
        if($files > $max_files) {
            $files = $max_files;
        }
        #die("We need more files than clients files=$files clients=$clients\n") if($clients > $files);
        my $cmd = "taskset -c 3 ./bench -c $clients -e $events -f $files -l $library >> stats";
        print "$cmd\n";
        system($cmd);
    }
}



