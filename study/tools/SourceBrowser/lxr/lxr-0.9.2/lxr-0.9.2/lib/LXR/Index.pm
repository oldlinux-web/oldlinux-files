# -*- tab-width: 4 -*- ###############################################
#
# $Id: Index.pm,v 1.9 2001/08/15 15:50:27 mbox Exp $

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

package LXR::Index;

$CVSID = '$Id: Index.pm,v 1.9 2001/08/15 15:50:27 mbox Exp $ ';

use LXR::Common;
use strict;

sub new {
	my ($self, $dbname, @args) = @_;
	my $index;

	if ($dbname =~ /^DBI:/i) {
		require LXR::Index::DBI;
		$index = new LXR::Index::DBI($dbname, @args);
	}
	elsif ($dbname =~ /^DBM:/i) {
	  require LXR::Index::DB;
	  $index = new LXR::Index::DB($dbname, @args);
	}
	else {
	  die "Can't find database, $dbname";
	}
	return $index;
}

# TODO: Add skeleton code here to define the Index interface

1;
