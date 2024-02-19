# -*- tab-width: 4 -*- ###############################################
#
# $Id: Java.pm,v 1.4 2001/11/14 15:27:36 mbox Exp $
#
# Enhances the support for the Java language over that provided by
# Generic.pm
#
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

package LXR::Lang::Java;


my $CVSID = '$Id: Java.pm,v 1.4 2001/11/14 15:27:36 mbox Exp $ ';

use strict;
use LXR::Common;
require LXR::Lang;
require LXR::Lang::Generic;

@LXR::Lang::Java::ISA = ('LXR::Lang::Generic');

# Only override the include handling.  For java, this is really package
# handling, as there is no include mechanism, so deals with "package" 
# and "import" keywords

sub processinclude {
	my ($self, $frag, $dir) = @_;
	# Deal with package declaration of the form
	# "package java.lang.util"
	$$frag =~ s#(package\s+)([\w.]+)#
	    $1.
	    ($index->issymbol($2, $$self{'release'}) ?
		join($2, @{$$self{'itag'}}) : $2)
	    #e;
	# Deal with import declaration of the form
	# "import java.awt.*" by providing link to the package
	$$frag =~ s#(import\s+)([\w.]+)(\.\*)#
		$1.
			($index->issymbol($2, $$self{'release'}) ?
			 join($2, @{$$self{'itag'}}) : $2) . 
				 $3 #e;
	
  	# Deal with import declaration of the form
  	# "import java.awt.classname" by providing links to the
	# package and the class
  	$$frag =~ s#(import\s+)([\w.]+)\.(\w+)(\W)#
  		$1.
  			($index->issymbol($2, $$self{'release'}) ?
  			 join($2, @{$$self{'itag'}}) : $2) . "." .
				 ($index->issymbol($3, $$self{'release'}) ?
				  join($3, @{$$self{'itag'}}) : $3) . $4#e;
			
	  }



1;


