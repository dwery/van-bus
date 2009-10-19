#!/usr/bin/perl
#first set com speed
($port)=@ARGV;
unless (-e $port) {
  print "$port not found!\n";
  exit;
}
if ($port=~/tty/) {
  `stty -F $port 500000`;
} elsif ($port=~/COM\d/) {
  `mode $port baud=500000 parity=N data=8`
}
open(VAN,"<$port") || die "failed to open $port";
while($frame=<VAN>) {
  $frame=~s/\s//g;#trim cr+LF
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
  #print "$hour:$min.$sec> raw frame [$frame]\n";
  if ($frame) {
    &decode($frame,sprintf("%02d:%02d.%02d",$hour,$min,$sec));
    if($port=~/\.van/) { #this is a file so delay a bit
      #select(undef,undef,undef,.05);#sleep 100ms
    }
  }
}
close(VAN);


sub decode {
  my ($frame,$time)=@_;
  my @frame=split(//,$frame);
  my $ident=join('',splice(@frame,0,3));
  my ($ext,$rak,$rw,$rtr)=split(//,unpack('B4',chr(hex(shift(@frame).'0'))));
#  $ext=~s/1/EXT/;
  $rak=~s/1/A/;
  $rw=~tr/[01]/[WR]/;
  $rtr=~s/1/T/;
  if (scalar(@frame) >3) {
  my $crc=join('',splice(@frame,-3,3));
  }
  my $data=join('',@frame);
  $frame="$time $ident $rw$rak$rtr $data";
  #print "[$frame]\n";
  if ($ident eq 'E24') { print "VIN:$data\n"; }
  elsif ($ident eq '8C4') { print "Radio Buttons: $data\n"; }
  elsif ($ident eq '9C4') { print "Radio Remote: $data\n"; }
  elsif ($ident eq '8A4') { print "Dashboard: $data\n"; }
  elsif ($ident eq '554') { print "Radio Info: $data\n"; }
  elsif ($ident eq '984') { print "Time: $data\n"; }
  elsif ($ident eq '4FC') { print "Lights Status: $data\n"; }
  elsif ($ident eq '564') { print "Car Status: $data\n"; }
  elsif ($ident eq '524') { print "Car Status II: $data\n"; }
  elsif ($ident eq '664') { print "Dash Buttons: $data\n"; }
  elsif ($ident eq '824') { print "Speed & RPM: $data\n"; }
  elsif ($ident eq '9CE') { print "stalk: $data\n";}
  elsif ($ident eq '9CA') { print "stall: $data\n";}

  #elsif (length($data)==2) { print "$frame\n";}
  else { print "$frame\n"; }
}
#E24::::iden::           VIN
#E24::0-::ascii::        VIN
#
#8A4::::iden::           Dashboard
#8A4::0:0x20:bit::       heartbeat
#8A4::0:0x0F:int::       brightness
#8A4::1:0x01:bit::       contact
#8A4::1:0x02:bit::       engine
#8A4::1:0x04:bit::       economy
#8A4::2::eval:   X/2:    temperature (water)
#8A4::3::int::           oil level
#8A4::4::int::           fuel level
#8A4::5::eval:   X/2:    temperature (oil)
#8A4::6::eval:(X-0x50)/2:temperature (ext)
#
#9C4::::iden::Radio Remote Control
#9C4::0:0x80:bit::seek+
#9C4::0:0x40:bit::seek-
#9C4::0:0x20:bit::?1
#9C4::0:0x10:bit::?2
#9C4::0:0x08:bit::vol+
#9C4::0:0x04:bit::vol-
#9C4::0:0x02:bit::source
#9C4::0:0x01:bit::?3
#9C4::1::int::wheel
#984::::iden::time
#984::3::bcd::time (h)
#984::4::bcd::time (m)
#
#554::::iden::Radio Info
#554:**D6:::::cd track info
#554:**D6:5::bcd::track time (m)
#554:**D6:6::bcd::track time (s)
#554:**D6:7::bcd::current track
#554:**D6:8::bcd::track count
#554:**D1:::::Radio Frequency
#554:**D1:5-4::eval:     (X*0.05)+50     : frequency
#554:**D1:7:0x01:bit:                    : ? TA
#554:**D1:7:0x02:bit:                    : ? RDS
#554:**D1:12-19::ascii:                  : RDS data
#554:**D3:2:0x0F:bcd:                    : preset no.
#554:**D3:2:0xF0:bcd:    (X >> 4)        : preset type
#554:**D3:2:0xF0:cmp:    0xC0            : type is RDS
#554:**D3:2:0xF0:cmp:    0x40            : type is frequency
#554:**D3:3-10::ascii:                   : data
#
#4FC::::iden::Lights
#4FC::5:0x80:bit::head (low)
#4FC::5:0x40:bit::head (high)
#4FC::5:0x20:bit::front fog
#4FC::5:0x10:bit::rear fog
#4FC::5:0x08:bit::right side indicator
#4FC::5:0x04:bit::left side indicator
#4FC::5:0x02:bit::?1
#4FC::5:0x01:bit::?2
#4FC::3::bcd:            : ? unknown gauge 1
#4FC::6::bcd:            : ? unknown gauge 2
#4FC::7::bcd:            : ? unknown gauge 3
#
#8C4:8AC2:::iden::Radio Buttons
#8C4:8AC2:2:0x1F:cmp:0x01:1
#8C4:8AC2:2:0x1F:cmp:0x02:2
#8C4:8AC2:2:0x1F:cmp:0x03:3
#8C4:8AC2:2:0x1F:cmp:0x04:4
#8C4:8AC2:2:0x1F:cmp:0x05:5
#8C4:8AC2:2:0x1F:cmp:0x06:6
#8C4:8AC2:2:0x1F:cmp:0x10:audio up
#8C4:8AC2:2:0x1F:cmp:0x11:audio down
#8C4:8AC2:2:0x1F:cmp:0x16:audio
#8C4:8AC2:2:0x1F:cmp:0x1B:radio
#8C4:8AC2:2:0x1F:cmp:0x1D:cd
#8C4:8AC2:2:0x1F:cmp:0x1E:cdc
#8C4:8AC2:2:0x40:bit::released
#
#564::::iden::           Car Status
#564::7:0x80:bit::       front right
#564::7:0x40:bit::       front left
#564::7:0x20:bit::       rear right
#564::7:0x10:bit::       rear left
#564::7:0x08:bit::       boot
#564::7:0x04:bit::       ?4
#564::7:0x02:bit::       ?5
#564::7:0x01:bit::       ?6
#564::10:0x01:bit::      right stick button
#564::11::int::          mean speed
#564::14-15::int::       range (km)
#564::16-17::eval: X/10: consumption (mean) l/100 km
#564::22-23::int::       consumption (immediate) l/100 km
#564::24-25::int::       mileage (km)
#
#524::::iden::           Car Status 2
#524::5:0x01:bit::       handbrake
#524::6:0x10:bit::       left stick button
#524::6:0x02:bit::       seatbelt warning
#524::8:0x02:bit::       ESP/ASR deactivated
#524::8:0x01:bit::       car locked
#524::9::cmp:0x01:       door(s) open
#524::9::cmp:0x41        : ASP/ESR disabled
#524::9::cmp:0x44        : Automatic lighting active
#524::9::cmp:0x45        : Automatic wiping active
#
#
#
#664::::iden::           Dashboard Buttons
#664::0:0x02:bit::       hazard button
#664::2:0x40:bit::       door lock button
#664::2:0x0F:int::       brightness
#
#824::::iden::           speed and rpm
#824::0-1::eval: X/10:   rpm
#824::2-3::eval: X/100:  speed
#824::4-6::int::         seq
#
#
