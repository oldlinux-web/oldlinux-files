# -*- tab-width: 4; cperl-indent-level: 4 -*- ###############################################
#
# $Id: Lang.pm,v 1.29 2002/03/18 14:55:43 mbox Exp $

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

package LXR::Lang;

$CVSID = '$Id: Lang.pm,v 1.29 2002/03/18 14:55:43 mbox Exp $ ';

use strict;
use LXR::Common;

sub new {
	my ($self, $pathname, $release, @itag) = @_;
	my ($lang, $type);

    foreach $type (values %{$config->filetype}) {
		if ($pathname =~ /$$type[1]/) {
			eval "require $$type[2]";
			die "Unable to load $$type[2] Lang class, $@" if $@;
			my $create = "new $$type[2]".'($pathname, $release, $$type[0])';
			$lang = eval($create);
			die "Unable to create $$type[2] Lang object, $@" unless defined $lang;
			last;
        }
    }
	
	if (!defined $lang) {
        # Try to see if it's a script
		my $fh = $files->getfilehandle($pathname, $release);
		return undef if !defined $fh;
		$fh->getline =~ /^\#!\s*(\S+)/s;

		my $shebang = $1;
		my %filetype = %{$config->filetype};
		my %inter = %{$config->interpreters};
		
		foreach my $patt (keys %inter) {
			if ($shebang =~ /$patt/) {
				eval "require $filetype{$inter{$patt}}[2]";
				die "Unable to load $filetype{$inter{$patt}}[2] Lang class, $@" if $@;
				my $create = "new ".
				  $filetype{$inter{$patt}}[2].'($pathname, $release, $filetype{$inter{$patt}}[0])';
				$lang = eval($create);
				last if defined $lang;
				die "Unable to create $filetype{$inter{$patt}}[2] Lang object, $@";
			}
		}
	}

	# No match for this file
	return undef if !defined $lang;
	
	$$lang{'itag'} = \@itag if $lang;

	return $lang;
}

sub processinclude {
	my ($self, $frag, $dir) = @_;

	$$frag =~ s#(\")(.*?)(\")#	 
	  $1.&LXR::Common::incref($2, "include", $2, $dir).$3 #e;
		$$frag =~ s#(\0<)(.*?)(\0>)#  
		  $1.&LXR::Common::incref($2, "include", $2).$3 #e;
	  }

sub processcomment {
	my ($self, $frag) = @_;

	$$frag = "<span class=\"comment\">$$frag</span>";
	$$frag =~ s#\n#</span>\n<span class=\"comment\">#g;
}

sub referencefile {
	my ($self) = @_;
		
	print(STDERR ref($self), "->referencefile not implemented.\n");
}


1;
