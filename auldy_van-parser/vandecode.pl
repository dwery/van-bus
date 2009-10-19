#!/usr/bin/perl
#todo
#always log to file for later analysis (when reading from tty rather than file)
#
#syntax: ./vandecode.pl inputdata
#
#inputdata can be a filename or a serial port device
#
#Examples:
#./vandecode.pl /dev/ttyS0	decode in real time from data stream at COM1 (must be able to support 500kbaud)
#./vandecode.pl /dev/ttyUSB0	decode in real time from data stream at first USB serial port
#./vandecode.pl capturefile.van	decode from capture file sleeping for $linesleep after each input
#./vandecode.pl capturefile.txt decode from capture file as fast as possible
#
#capture files can be created by piping serial port input into a file a-la cat /dev/ttyUSB0 > capturefile.van
#
#
$linesleep=.01; #.05 is nice to look at .01 is almost like in car
my $firstline=0;	#first line to print output on minusone

$SIG{INT} = \&tsktsk; #ctrl+c
$SIG{QUIT} = \&changefocus; #ctrl+\

sub tsktsk {
  print "\e[".(28+$firstline)."H";#cursor line over decode header
  print "\e[m"; #clear any colour  
  print "\e[K"; #erase line
  close(VAN);
  exit;
}

@focusbytes=qw/000 464 4D4 4DC 4EC 4FC 524 54E 554 564 5E4 64E 664 6CE 7CE 824 8A4 8C4 8CE 8FC 94E 984 9C4 9CE B0E E24/;
sub changefocus {
#  ;set a focus byte to help  with decoding
$newfocusbyte=1;  
  $focusbyte++;
  if($focusbyte == scalar(@focusbytes)) {
		  $focusbyte=0;
	  }
  $focusident=$focusbytes[$focusbyte];
}
#
#
#first set com speed
($port)=@ARGV;
unless (-e $port) {
  print "$port not found!\n";
  exit;
}

if ($port=~/tty/) {
  `stty -F $port 500000`;
}

$red="\e[31m"; 
$green="\e[32m"; 
$blue="\e[34m"; 
$clear="\e[m";

print "\e[m\e[2J\n"; #clear colour clear screen


open(VAN,"<$port") || die "failed to open $port";
while($frame=<VAN>) {
  $frame=~s/\s//g;#trim cr+LF
  if (length($frame) > (3+1+3+1+4)) {

    my @frame=split(//,$frame);
    my $ident=join('',splice(@frame,0,3));
    my ($ext,$rak,$rw,$rtr)=split(//,unpack('B4',chr(hex(shift(@frame).'0'))));
    #  $ext=~s/1/EXT/;
    my $rak=~s/1/A/;
    my $rw=~tr/[01]/[WR]/;
    my $rtr=~s/1/T/;
    my $ack=pop(@frame);
    my $crc=join('',splice(@frame,-4,4));
    my $data=join('',@frame);
    my $atr="$rw$rak$rtr";
    $newdata{$ident}=$data;
  
    #colour different stuff?
    unless ($olddata{$ident}) { #colour whole line
      $linecol=$red;
    } else {
      $linecol=$clear;
    }

    if ($ident == 0) { ;}#don't be silly
    elsif ($ident eq '464') { &difprint(1,$linecol,"464:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '4D4') { &difprint(2,$linecol,"Audio Settings:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '4DC') { &difprint(3,$linecol,"Air Con:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '4EC') { &difprint(4,$linecol,"CDC Info:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '4FC') { &difprint(5,$linecol,"Lights Status:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '524') { &difprint(6,$linecol,"Car Status II:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '54E') { &difprint(7,$linecol,"Sat Nav 1:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '554') { &difprint(8,$linecol,"Radio Info:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '564') { &difprint(9,$linecol,"Car Status:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '5E4') { &difprint(10,$linecol,"Display Settings?:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '64E') { &difprint(11,$linecol,"Sat Nav 2:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '664') { &difprint(12,$linecol,"Dash Buttons:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '6CE') { &difprint(13,$linecol,"Sat Nav 3 (addr):",$ident,$atr,$data,$ack); }
    elsif ($ident eq '74E') { &difprint(14,$linecol,"Sat Nav 4:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '7CE') { &difprint(15,$linecol,"Sat Nav 5?:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '824') { &difprint(16,$linecol,"Speed & RPM:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '8A4') { &difprint(17,$linecol,"Dashboard:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '8C4') { &difprint(18,$linecol,"Radio Buttons:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '8CE') { &difprint(19,$linecol,"Sat Nav 6:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '8FC') { &difprint(20,$linecol,"Odometer:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '94E') { &difprint(21,$linecol,"Sat Nav 7:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '984') { &difprint(22,$linecol,"Time:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '9C4') { &difprint(23,$linecol,"Radio Remote:",$ident,$atr,$data,$ack); }
    elsif ($ident eq '9CE') { &difprint(24,$linecol,"Sat Nav 8:",$ident,$atr,$data,$ack); }
    elsif ($ident eq 'B0E') { &difprint(25,$linecol,"B0E:",$ident,$atr,$data,$ack); }
    elsif ($ident eq 'E24') { &difprint(26,$linecol,"VIN:",$ident,$atr,$data,$ack); }
    #unknown packets
    else { &difprint(27,"\e[K$linecol","Unknown Ident:",$ident,$atr,$data,$ack); }
    #flush prints
    print "\n";
    
    &focusdecode(28,$focusident);

    # if ($data) {print "$frame\n";}
    

  $olddata{$ident}=$data;
  if($port=~/\.van/) { #this is a file so delay a bit
    select(undef,undef,undef,$linesleep);#sleep s
  }
  }
}
close(VAN);
&tsktsk;


sub difprint {
	my ($termpos,$linecol,$packstring,$ident,$atr,$data,$ack)=@_;
	$termpos+=$firstline;
	$termpos="\e[$termpos;0H";
	print $termpos.$linecol;
	if ($focusident eq $ident) {
	  print $green.$ident.$clear;
        } else {
	  print $ident;
        }
	print ' '.$packstring.' '.$atr.' ';
  foreach $offset (0 .. length($data)-1) {
	$newchar=substr($data, $offset, 1);
 	if ($newchar ne substr($olddata{$ident}, $offset, 1)) {
		print $red.$newchar.$clear;
	} else {
		print $newchar;
	}
	#if $offset is odd then print space
	#unless ($offset/2 == int($offset/2)) { print ' '; }
  }
  print ' '.$ack;
}



sub difprint_forfocus {
	my ($ident,$data)=@_;
  foreach $offset (0 .. length($data)-1) {
	$newchar=substr($data, $offset, 1);
 	if ($newchar ne substr($olddata{$ident}, $offset, 1)) {
		print $red.$newchar.$clear;
	} else {
		print $newchar;
	}
	#if $offset is odd then print space
	unless ($offset/2 == int($offset/2)) { print ' '; }
  }
  print "\n";
}


sub focusdecode { #want this to only run once for each packet coming in!
  my ($termpos,$ident)=@_;
	$termpos+=$firstline;
	print "\e[$termpos;0H"; #set line and clear it to the end of the screen

	if (($newfocusbyte) || ($newdata{$ident} ne $olddata{$ident})) {
		print "\e[J"; 
	} #clear to end of screen but only if the byte has changed 

  if ($focusbyte == 0) {
    print "No byte in focus for decode help. ctrl+\\ to cycle bytes.\n";
  } else {
    print $green."Decoding IDEN $ident:$clear\n";
    unless ($newdata{$ident}) {
	print "No packets seen yet.\n";
    } else {
	    if (($newfocusbyte) || ($newdata{$ident} ne $olddata{$ident})) {
		print "Data: $newdata{$ident}\n";
		&difprint_forfocus($ident,$newdata{$ident});
		#now based on packet you may display custom stuff from here
		#packet data is stored in $newdata{$ident}
		if ($ident eq '') {}
		elsif ($ident eq '9C4') { &decode9C4($newdata{$ident}); }
		#
	    }
    }
  }
  $newfocusbyte=0;
}

sub decode9C4 {
  my ($data)=@_;
  my ($btns,$wheel)=($data=~/(\w{2})(\w{2})/);
  print "Buttons:";
  print " Seekup" if (0x80 & hex($btns));
  print " Seekdown" if (0x40 & hex($btns));
  print " Volup" if (0x08 & hex($btns));
  print " Voldown" if (0x04 & hex($btns));
  print " Src" if (0x02 & hex($btns));

  print "\nWheel position: $wheel\n";
}
