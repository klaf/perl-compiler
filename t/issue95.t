#! /usr/bin/env perl
# http://code.google.com/p/perl-compiler/issues/detail?id=95
# methods not found. see t/testc.sh -DCsP,-v -O0 95
use strict;
BEGIN {
  unshift @INC, 't';
  require "test.pl";
}
use Test::More;
eval "use IO::Socket::SSL";
if ($@) {
  plan skip_all => "IO::Socket::SSL required for testing issue95" ;
} else {
  plan tests => 6;
}

my $issue = <<'EOF';
use IO::Socket::INET   ();
use IO::Socket::SSL    ('inet4');
use Net::SSLeay        ();
use IO                 ();
use Socket             ();

my $handle = IO::Socket::SSL->new(SSL_verify_mode =>0);
$handle->blocking(0);
print "ok";
EOF

my $typed = <<'EOF';
use IO::Socket::SSL();
my IO::Handle $handle = IO::Socket::SSL->new(SSL_verify_mode =>0);
$handle->blocking(0);
print "ok";
EOF

my $new_issue = <<'EOF';
use IO::Socket::INET   ();
use IO::Socket::SSL    ('inet4');

my IO::Handle $fd = IO::Socket::SSL->new(SSL_verify_mode =>0);
my $socket = $fd->accept();
my $ssl_ctx = IO::Socket::SSL::SSL_Context->new(SSL_server     => 1);
IO::Socket::SSL->start_SSL(
            $socket,
            'SSL_reuse_ctx'  => $ssl_ctx,
            'SSL_server'     => 1,
            %SSLARGS
  );
delete ${*$socket}{'SSL_error_trap'};
my ( $NetSSLeayobj, $SSLsocket ) = ( $socket->_get_ssl_object(), 2 );
print "ok";
EOF

sub compile_check {
  my ($num,$b,$base,$script,$cmt) = @_;
  my $name = $base."_$num";
  unlink("$name.c", "$name.pl");
  open F, ">", "$name.pl";
  print F $script;
  close F;
  my $X = $^X =~ m/\s/ ? qq{"$^X"} : $^X;
  $b .= ',-DCsp,-v';
  my ($result,$out,$stderr) =
    run_cmd("$X -Iblib/arch -Iblib/lib -MO=$b,-o$name.c $name.pl", 20);
  unless (-e "$name.c") {
    print "not ok $num # $name B::$b failed\n";
    exit;
  }
  # check stderr for "blocking not found"
  #diag length $stderr," ",length $out;
  if (!$stderr and $out) {
    $stderr = $out;
  }
  my $notfound = $stderr =~ /blocking not found/;
  ok(!$notfound, $cmt . " - blocking found in stderr");
  # check stderr for "save package_pv "blocking" for method_name"
  my $found = $stderr =~ /save package_pv "blocking" for method_name/;
  #TODO: {
  #  local $TODO = "wrong package_pv blocking";
  ok(!$found, $cmt . " - save package_pv blocking for method_name");
  #}
}

compile_check(1,'C,-O3,-UB','ccode95i',$issue,"IO::Socket::blocking method found in \@ISA");
compile_check(2,'C,-O3,-UB','ccode95i',$typed,'type hint');
ctestok(3,'C,-O3,-UB','ccode95i',$issue,'TODO run');
ctestok(4,'C,-O3,-UB','ccode95i',$new_issue,"IO::Socket::SSL->start_SSL");
