# -*- tab-width: 4 -*- ###############################################
#
# $Id: Config.pm,v 1.26 2002/02/26 15:59:32 mbox Exp $

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

package LXR::Config;

$CVSID = '$Id: Config.pm,v 1.26 2002/02/26 15:59:32 mbox Exp $ ';

use strict;

use LXR::Common;

require Exporter;

use vars qw($AUTOLOAD $confname);

$confname = 'lxr.conf';

sub new {
    my ($class, @parms) = @_;
    my $self = {};
    bless($self);
    $self->_initialize(@parms);
    return($self);
	die("Foo!\n");
}

sub readfile {
    local($/) = undef;		# Just in case; probably redundant.
    my $file  = shift;
    my @data;

    open(INPUT, $file) || fatal("Config: cannot open $file\n");
    $file = <INPUT>;
    close(INPUT);

    @data = $file =~ /([^\s]+)/gs;

    return wantarray ? @data : $data[0];
}


sub _initialize {
    my ($self, $url, $confpath) = @_;
    my ($dir, $arg);

    unless ($url) {
		$url = 'http://'.$ENV{'SERVER_NAME'}.':'.$ENV{'SERVER_PORT'};
		$url =~ s/:80$//;

		$url .= $ENV{'SCRIPT_NAME'};
    }
    
    $url =~ s|^http://([^/]*):443/|https://$1/|;

    unless ($confpath) {
		($confpath) = ($0 =~ /(.*?)[^\/]*$/);
		$confpath .= $confname;
    }
    
    unless (open(CONFIG, $confpath)) {
		die("Couldn't open configuration file \"$confpath\".");
    }

	$$self{'confpath'} = $confpath;
    
    local($/) = undef;
    my @config = eval("\n#line 1 \"configuration file\"\n".
					  <CONFIG>);
    die($@) if $@;

    my $config;
    foreach $config (@config) {
		if ($config->{baseurl}) {
			my $root = quotemeta($config->{baseurl});
			next unless $url =~ /^$root/;
		}
		
		%$self = (%$self, %$config);
    }

	die "Can't find config for $url\n" if !defined $$self{baseurl};
}


sub allvariables {
    my $self = shift;

    return keys(%{$self->{variables} || {}});
}


sub variable {
    my ($self, $var, $val) = @_;

    $self->{variables}{$var}{value} = $val if defined($val);
    return $self->{variables}{$var}{value} || 
		$self->vardefault($var);
}


sub vardefault {
    my ($self, $var) = @_;

    return $self->{variables}{$var}{default} || 
		$self->{variables}{$var}{range}[0];
}


sub vardescription {
    my ($self, $var, $val) = @_;

    $self->{variables}{$var}{name} = $val if defined($val);

    return $self->{variables}{$var}{name};
}


sub varrange {
    my ($self, $var) = @_;

	if (ref($self->{variables}{$var}{range}) eq "CODE") {
		return &{$self->{variables}{$var}{range}};
	}

    return @{$self->{variables}{$var}{range} || []};
}


sub varexpand {
    my ($self, $exp) = @_;
    $exp =~ s/\$\{?(\w+)\}?/$self->variable($1)/ge;

    return $exp;
}


sub value {
    my ($self, $var) = @_;

    if (exists($self->{$var})) {
		my $val = $self->{$var};
		
		if (ref($val) eq 'ARRAY') {
			return map { $self->varexpand($_) } @$val;
		}
		elsif (ref($val) eq 'CODE') {
			return $val;
		}
		else {
			return $self->varexpand($val);
		}
    }
    else {
		return undef;
    }
}


sub AUTOLOAD {
    my $self = shift;
    (my $var = $AUTOLOAD) =~ s/.*:://;

	my @val = $self->value($var);
	
	if (ref($val[0]) eq 'CODE') {
		return $val[0]->(@_);
	}
	else {
		return wantarray ? @val : $val[0];
	} 
}


sub mappath {
    my ($self, $path, @args) = @_;
    my %oldvars;
    my ($m, $n);
    
    foreach $m (@args) {
		if ($m =~ /(.*?)=(.*)/) {
			$oldvars{$1} = $self->variable($1);
			$self->variable($1, $2);
		}
    }

    while (($m, $n) = each %{$self->{maps} || {}}) {
		$path =~ s/$m/$self->varexpand($n)/e;
    }

    while (($m, $n) = each %oldvars) {
		$self->variable($m, $n);
    }

    return $path;
}


1;
