#!/usr/bin/perl
#
# Copyright (c) 2013-2014 ADTECH GmbH
# Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
#
# Author: Andreas Pohl

use strict;
use threads;
use threads::shared;
use IO::Socket;
use Time::HiRes qw(gettimeofday tv_interval);

$|++;

my $host = $ARGV[0];
$host ||= "localhost";

my $print_count :shared = 0;
my $print_time :shared = 0;

my $max_threads = 1000;
my $tcount = 1;
for (my $i = 0; $i < $tcount; $i++) {
    threads->create(\&run);
}

while (1) {
    my $interval = 5;
    sleep $interval;
    {
	lock($print_count);
	if ($print_count) {
	    print "$tcount ".($print_count/$interval)." ".($print_time/$print_count)."\n";
	    $print_time = 0;
	    $print_count = 0;
	}
    }
    if ($tcount < $max_threads) {
	$tcount += 10;
	for (my $i = 0; $i < 10; $i++) {
	    threads->create(\&run);
	}
    } else {
	exit;
    }
}

sub run {
    my $c = IO::Socket::INET->new(Proto    => "tcp",
				  PeerAddr => $host,
				  PeerPort => "8080"
				 ) or die "connection failed";
    $c->autoflush(1);

    my $data = "GET /addyn/3/784/123456/0/1/ADTECH;size=1x1;kvtest=test HTTP/1.1
Host: localhost
Connection: keep-alive
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/27.0.1453.116 Safari/537.36
Accept-Encoding: gzip,deflate,sdch
Accept-Language: en-US,en;q=0.8
Cookie: JEB2=jkshfuiwhkjshdfkuhwuefhuksehfkseuhfks

";
    my $count = 0;
    my $time_sum = 0.0;
    my $last_calc = time;
    while (1) {
	my $t0 = [gettimeofday];
	$c->send($data);
	my $buf;
	$c->recv($buf, 1024);
	$time_sum += tv_interval($t0);
	$count++;
	if (!($count % 10)) { # check every 10 reqs
	    my $now = time;
	    if ($now - $last_calc >= 5) {
		# update stats
		lock($print_count);
		$print_count += $count;
		$print_time += ($time_sum/$count);
		$time_sum = 0;
		$count = 0;
	    }
	}
    }
}
