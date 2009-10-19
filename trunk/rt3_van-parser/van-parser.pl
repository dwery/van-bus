#!/usr/bin/perl -w

# Decodes VAN packets from STDIN, using specs
# in van-frames.txt
#
# cat /dev/tts/0 | van-parser.pl [filterspec]
# cat capture-file.txt | van-parser.pl [filterspec]
#
# A filter spec is a combination of [+-]port
# which includes or excludes a port from being
# analyzed.
#
# Copyright (C) 2005-2009 Alessandro Zummo
#

use strict;
use warnings;

use POSIX;
use IO::File;

	my %van;
	my %coverage;

	my %exclude;
	my %include;

	my $donotcompare = 0;

	foreach (@ARGV)
	{
		$include{uc($1)} = 1 if /\+([[:xdigit:]]{3})/;
		$exclude{uc($1)} = 1 if /-([[:xdigit:]]{3})/;
		$donotcompare = 1 if /--show-all/;
	}

	my $filter_mode = scalar keys %include ? 1 : 0;

	print STDERR "van-parser.pl 0.4\n";
	print STDERR "include: ", join(",", keys %include), "\n";
	print STDERR "exclude: ", join(",", keys %exclude), "\n";

	my $defs = load_defs('van-frames.txt');

	$/ = "\015";

	while (<STDIN>)
	{
		s/[\015\012]//g;
		parse($defs, $_);
	}

	print "done.\n";

sub parse
{
	my ($defs, $line) = @_;

	my $bytes = '000102030405060708091011121314151617181920';

	# 664 WA- 00000F04FFFF00FFFFFF00 A

	return unless $line =~ /^(?:(T.+Z), ){0,1}(...)[>:]{0,1} (...) ([[:xdigit:]]{2,64}) ([-|A])/;

	my ($time, $port, $status, $data, $ack) = ($1, $2, $3, $4, $5);

	$time = strftime('T%Y%m%d%TZ', gmtime())
		unless defined $time;

	# if the @include array is not empty,
	# filter out everything else.
	return
		if $filter_mode == 1
		and not defined $include{$port};

	# skip filtered ports
	return
		if defined $exclude{$port};

	unless (defined $van{$port})
	{
		print "$port| new\n";

		$van{$port}{'data'}	= '';
		$van{$port}{'count'}	= 0;
		$van{$port}{'changes'}	= 0;
	}

	$van{$port}{'count'}++;

	if ($van{$port}{'data'} ne $data || $donotcompare)
	{
		$van{$port}{'changes'}++;

		print "$time, $port: $status $data $ack\n";
		print $port, '|';
		printf '%06d', $van{$port}{'count'};
		print '              old: ', $van{$port}{'data'}, "\n";
		print $port, '|                         ', substr($bytes, 0, length($data)), "\n";

		print "$port| unknown\n"
			unless defined $defs->{$port};

		# find a filter that applies
		# to this packet.

		foreach ( @{$defs->{$port}{'filters'}} )
		{
			analyze($defs->{$port}{$_}, $data)
				if $data =~ $_;
		}

		$van{$port}{'data'} = $data;
	}
}

sub analyze
{
	my ($portdef, $frame) = @_;

	my @frame = unpack('C*', pack('H*', $frame));
	my $flen = scalar @frame;

	foreach my $fdef ( @$portdef )
	{
		$fdef->{'end'} = $flen - $fdef->{'start'} - 1
			if $fdef->{'length'} == 0;

		my @data = @frame[ $fdef->{'start'} .. $fdef->{'end'} ];

		my $value = unpack_int($fdef, @data);

		if ($fdef->{'type'} eq 'assert'
		or $fdef->{'type'} eq 'int'
		or $fdef->{'type'} eq 'bcd')
		{
			# Decode and evaluate expression

			my $expression = $fdef->{'args'};

			if (defined $expression)
			{
				$expression =~ s/X/$value/g;

				$value = eval($expression);
			}

			next
				if $value =~ /\D/
				and $value eq 'NaN';

			print_data($fdef, $value),
			next
				if $fdef->{'type'} eq 'int';

			print_data($fdef, $value =~ m/^\d+$/ ? sprintf('%X', $value) : $value),
			next
				if $fdef->{'type'} eq 'bcd';

			print_data($fdef)
				if $value;
			next;
		}
		elsif ($fdef->{'type'} eq 'bit')
		{
			print_data($fdef)
				if $value;
		}
		elsif ($fdef->{'type'} eq 'cmp')
		{
			print_data($fdef)
				if $value == eval($fdef->{'args'});
		}
		elsif ($fdef->{'type'} eq 'ascii')
		{
			print_data($fdef, pack('C*', @data)  );
		}
	}
}

sub print_data
{
	my $fdef = shift;

	printf '%6s, %-16s', $fdef->{'range'}, $fdef->{'desc'};

	print  ': ', @_
		if scalar @_;

	print  "\n";
}

sub unpack_int
{
	my $fdef = shift;

	my ($i, $value) = (1,0);

	foreach (@_)
	{
		next unless defined $_;

		$value += $_ << ( 8 * ($fdef->{'length'} - $i++) );
	}

	$value = $value & $fdef->{'mask'}
		if defined $fdef->{'mask'};

	return $value;
}

sub load_defs
{
	my ($file) = @_;

	my %defs;

	my @types = ( qw ( bit int bcd cmp assert ascii iden ) );

	my $fh = new IO::File $file
		or die "Couldn't load definitions ($file): $!\n";

	while (<$fh>)
	{
		chomp;

		# IDEN:[filter]:range:mask:type:[args]:description 

		my @fields = split(/(?<!\\):/, $_);

		next if scalar @fields < 7;

		

		my ($iden, $filter, $range, $mask, $type, $args, $desc) =
			@fields;

		next if $type eq '';

		# compile filter

		$filter = '*'
			if $filter eq '';
	
		$filter =~ s/\*/./g;

		$filter = qr/^$filter/;

		# range

		my ($start, $length);

		$start = $1, $length = 1
			if $range =~ /^(\d+)$/;

		$start = $1, $length = ($2 ne '' ? abs($2 - $1) + 1 : 0)
			if $range =~ /^(\d+)-(\d*)$/;

		# type check

		print "Unknown type $type, ignored.\n"
			unless grep { $type eq $_ } @types;

		next
			if $type eq 'iden';

		# description and args cleanup
		$desc =~ s/\\:/:/g;
		$desc =~ s/^\s+//;
		$desc =~ s/\s+$//;
		$args =~ s/^\s+//g;
		$args =~ s/\\//g;

		# store our definition

		my %fdef;

		$fdef{'iden'}	= $iden;
		$fdef{'range'}	= $range;
		$fdef{'start'}	= $start;
		$fdef{'end'}	= $start + $length - 1;
		$fdef{'length'}	= $length;
		$fdef{'mask'}	= eval($mask);
		$fdef{'type'}	= $type;
		$fdef{'args'}	= $args ne '' ? $args : undef;
		$fdef{'desc'}	= $desc;

		$coverage{"$iden/$filter"}{$fdef{'start'}} |= $fdef{'mask'}
			if defined $fdef{'mask'} and $fdef{'start'} == $fdef{'end'};

		push(@{$defs{$iden}{$filter}}, \%fdef);
	}

	foreach ( keys %defs )
	{
		$defs{$_}{'filters'} = [ keys %{$defs{$_}} ];
	}

	$fh->close;

#	foreach my $c (keys %coverage)
#	{
#		print "$c\n";
#
#		foreach my $b (keys %{$coverage{$c}})
#		{
#			printf " %d %02X\n", $b, $coverage{$c}{$b};
#		}
#	}

	return \%defs;
}
