package AllTests;

use ConfigTest;

use Test::Unit::TestRunner;
use Test::Unit::TestSuite;

sub new {
	my $class = shift;
	return bless {}, $class;
}

sub suite {
	my $class = shift;
	my $suite = Test::Unit::TestSuite->empty_new("LXR Tests");
	$suite->add_test(Test::Unit::TestSuite->new("ConfigTest"));

	return $suite;
}

1;
