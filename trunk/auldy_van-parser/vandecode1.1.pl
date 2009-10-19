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

@focusbytes=qw/000 464 4D4 4DC 4EC 4FC 524 54E 554 564 5E4 64E 664 6CE 7CE 824 8A4 8C4 8CE 8FC 94E 984 9C4 9CE B0E E24/;

my %ident_lookup = ( '464' => { pos=>1,  name=>'464'},
		     '4D4' => { pos=>2,  name=>'Audio Settings'},
                     '4DC' => { pos=>3,  name=>'Air Con'},
                     '4EC' => { pos=>4,  name=>'CDC Info'},
                     '4FC' => { pos=>5,  name=>'Lights Status'},
                     '524' => { pos=>6,  name=>'Car Status II'},
                     '54E' => { pos=>7,  name=>'Sat Nav 1'},
                     '554' => { pos=>8,  name=>'Radio Info'},
                     '564' => { pos=>9,  name=>'Car Status'},
                     '5E4' => { pos=>10, name=>'Display Settings?'},
                     '64E' => { pos=>11, name=>'Sat Nav 2'},
                     '664' => { pos=>12, name=>'Dash Buttons'},
                     '6CE' => { pos=>13, name=>'Sat Nav 3 (addr)'},
                     '74E' => { pos=>14, name=>'Sat Nav 4'},
                     '7CE' => { pos=>15, name=>'Sat Nav 5?'},
                     '824' => { pos=>16, name=>'Speed & RPM', handler=>\&decode824},
                     '8A4' => { pos=>17, name=>'Dashboard'},
                     '8C4' => { pos=>18, name=>'Radio Buttons'},
                     '8CE' => { pos=>19, name=>'Sat Nav 6'},
                     '8FC' => { pos=>20, name=>'Odometer'},
                     '94E' => { pos=>21, name=>'Sat Nav 7'},
                     '984' => { pos=>22, name=>'Time'},
                     '9C4' => { pos=>23, name=>'Radio Remote', handler=>\&decode9C4},
                     '9CE' => { pos=>24, name=>'Sat Nav 8'},
                     'B0E' => { pos=>25, name=>'B0E'},
                     'E24' => { pos=>26, name=>'VIN:'}, 
                     'unk' => { pos=>27, name=>'Unknown Ident'}
                    );




$SIG{INT} = \&tsktsk; #ctrl+c
$SIG{QUIT} = \&changefocus; #ctrl+\

sub tsktsk {
  print "\e[".(28+$firstline)."H";#cursor line over decode header
  print "\e[m"; #clear any colour  
  print "\e[K"; #erase line
  close(VAN);
 if ($filename=~/\.van/) {
close(LOGGING);
}

  exit;
}


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
($port,$filename)=@ARGV;
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

if ($filename=~/\.van/) {
open(LOGGING,">>$filename");
}
open(VAN,"<$port") || die "failed to open $port";
while($frame=<VAN>) {

if ($filename=~/\.van/) {
print LOGGING $frame;
}
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

	$this_ident = $ident_lookup{$ident} || $ident_lookup{'unk'};
	&difprint($this_ident->{pos}, $linecol, $this_ident->{name},$ident,$atr,$data,$ack);


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
	print ' '.$packstring.': '.$atr.' ';
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
		#if ($ident eq '') {}
		#elsif ($ident eq '9C4') { &decode9C4($newdata{$ident}); }
		if($ident_lookup{$ident}{'handler'}) {
		  &{$ident_lookup{$ident}{'handler'}}($newdata{$ident})
		}

		#
	    }
    }
  }
  $newfocusbyte=0;
}

sub decode824 {
  my ($data)=@_;
  my ($rpm,$speed)=($data=~/(\w{4})(\w{4})/);
  $rpm=int(hex($rpm)/10);
  $speed=int(hex($speed)/100);
  print "Rpm: $rpm\n";
  print "Speed: $speed\n";
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

