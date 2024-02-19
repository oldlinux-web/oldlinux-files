# Test cases for the LXR::Config module
# Uses the associated lxr.conf file

package ConfigTest;
use strict;

use Test::Unit;
use lib "..";
use lib "../lib";

use LXR::Config;

use base qw(Test::Unit::TestCase);

sub new {
	my $self = shift()->SUPER::new(@_);
	$self->{config} = 0;
	return $self;
}

# define tests

# test that the config object was created successfully
sub test_creation {
	my $self = shift;
	$self->assert(defined($self->{config}), "Config init failed");
}

# Access some of the values to check what is found
sub test_access {
	my $self = shift;
	$self->assert($self->{config}->swishindex eq '/test/lxr/bin/swish-e',
		   "swishindex read failed");
	$self->assert($self->{config}->baseurl eq 'http://test/lxr',
		   "Config accessed wrong baseurl");
}

# test access to the variables section
sub test_variables {
	my $self = shift;
	$self->assert($self->{config}->variable('v') == '1.0.6',
		   "Variable default not correct");
	$self->assert(($self->{config}->varrange('v'))[1] =~ /hi hippy/,
		   "Variable value missing");
}

sub test_allvariables {
	my $self = shift;
	my @vars = $self->{config}->allvariables();
    $self->assert(grep {$_ eq 'v'} @vars, "allvariables didn't return v");
	$self->assert(grep {$_ eq 'a'} @vars, "allvariables didn't return a");
	$self->assert($#vars == 2, "Too many variables returned");
}


# set_up and tear_down are used to
# prepare and release resources need for testing

# Prepare a config object
 sub set_up {
	my $self = shift;
	$self->{config} = new LXR::Config("http://test/lxr", "./lxr.conf");
 	}

 sub tear_down {
	my $self = shift;
	$self->{config} = undef;
 }




1;
