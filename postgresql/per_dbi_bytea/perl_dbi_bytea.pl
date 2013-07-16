use strict;
use warnings;
use 5.16.3;

use DBI;
use DBD::Pg;
use DBD::Pg qw(:pg_types);
use File::Slurp;

die("Usage: $0 filename") unless defined($ARGV[0]);
die("File $ARGV[0] doesn't exist") unless (-e $ARGV[0]);
my $filename = $ARGV[0];


my $dbh = DBI->connect("dbi:Pg:dbname=regress","","", {AutoCommit=>0});
$dbh->do(q{
	DROP TABLE IF EXISTS byteatest;
	CREATE TABLE byteatest( blah bytea not null );
});
$dbh->commit();

my $filedata = read_file($filename);
my $sth = $dbh->prepare("INSERT INTO byteatest(blah) VALUES (?)");
$sth->bind_param(1, $filedata, { pg_type => DBD::Pg::PG_BYTEA });
$sth->execute();
undef $filedata;

$dbh->commit();
