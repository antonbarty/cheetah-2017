#!/usr/bin/perl

# Check number of arguments
if($#ARGV < 2){
  print STDERR "Usage: $0 <srcstring> <dststring> <files>\n";
  exit;
}

$srcstring = shift @ARGV;
$dststring = shift @ARGV;

foreach $srcfile (@ARGV) {
    open (SRCFILE, "<$srcfile") or die "Can't open $srcfile: $!\n";
    $dstfile = "$srcfile.replace";
    open (DSTFILE, ">$dstfile") or die "Can't open $dstfile: $!\n";
    while (<SRCFILE>) {
	s/$srcstring/$dststring/g;
	print DSTFILE;
    }
    close SRCFILE;
    close DSTFILE;
    rename $dstfile, $srcfile;
} 
