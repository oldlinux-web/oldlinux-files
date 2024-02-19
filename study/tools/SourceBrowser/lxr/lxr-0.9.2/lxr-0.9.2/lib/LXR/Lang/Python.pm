# -*- tab-width: 4 -*- ###############################################
#
# $Id: Python.pm,v 1.2 2001/08/15 15:50:27 mbox Exp $

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

package LXR::Lang::Python;

$CVSID = '$Id: Python.pm,v 1.2 2001/08/15 15:50:27 mbox Exp $ ';

use strict;
use LXR::Common;
use LXR::Lang;

use vars qw(@ISA);
@ISA = ('LXR::Lang');

my @spec = ('comment'	=> ('#',		"\$"),
			'string'	=> ('"',		'"'),
			'string'	=> ("'",		"'"),
			'atom'		=> ('\\\\.',	''));

sub new {
	my ($self, $pathname, $release) = @_;

	$self = bless({}, $self);

	$$self{'release'} = $release;

	if ($pathname =~ /(\w+)\.py$/ || $pathname =~ /(\w+)$/) { 
		$$self{'modulename'} = $1;
	}

   	return $self;
}

sub parsespec {
	return @spec;
}

sub processcode {
	my ($self, $code, @itag) = @_;
	
	$$code =~ s/([a-zA-Z_][a-zA-Z0-9_\.]*)/
		($index->issymbol( $$self{'modulename'}.".".$1, $$self{'release'} )
		 ? join('', 
				$$self{'itag'}[0], 
				$$self{'modulename'}.".".$1,
				$$self{'itag'}[1],
				$1,
				$$self{'itag'}[2])
		 : $1)/ge;
}


sub	indexfile {
	my ($self, $name, $path, $fileid, $index, $config) = @_;

	my (@ptag_lines, @single_ptag, $module_name);

	if ($name =~ m|/(\w+)\.py$|) {
		$module_name = $1;
	}
	
	open(PYTAG, $path);
		
	while (<PYTAG>) {
		chomp;
		
		# Function definitions
		if ( $_ =~ /^\s*def\s+([^\(]+)/ ) {
			$index->index($module_name."\.$1", $fileid, $., "f");
		}
		# Class definitions 
		elsif ( $_ =~ /^\s*class\s+([^\(:]+)/ ) {
			$index->index($module_name."\.$1", $fileid, $., "c");
		}
		# Targets that are identifiers if occurring in an assignment..
		elsif ( $_ =~ /^(\w+) *=.*/ ) {
			$index->index($module_name."\.$1", $fileid, $., "v");
		}
		# ..for loop header.
		elsif ( $_ =~ /^for\s+(\w+)\s+in.*/ ) {
			$index->index($module_name."\.$1", $fileid, $., "v");
		}
	}
	close(PYTAG);
}
