# -*- tab-width: 4 -*- ###############################################
#
# $Id: Files.pm,v 1.6 2001/08/15 15:50:27 mbox Exp $

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

package LXR::Files;

$CVSID = '$Id: Files.pm,v 1.6 2001/08/15 15:50:27 mbox Exp $ ';

use strict;

sub new {
	my ($self, $srcroot) = @_;
	my $files;

	if ($srcroot =~ /^CVS:(.*)/i) {
		require LXR::Files::CVS;
		$srcroot = $1;
		$files = new LXR::Files::CVS($srcroot);
	}
	else {
		require LXR::Files::Plain;
		$files = new LXR::Files::Plain($srcroot);
	}
	return $files;
}


1;
