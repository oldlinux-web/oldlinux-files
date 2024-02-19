# -*- tab-width: 4 -*- ###############################################
#
# $Id: Perl.pm,v 1.5 2002/03/18 14:55:43 mbox Exp $

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

package LXR::Lang::Perl;

$CVSID = '$Id: Perl.pm,v 1.5 2002/03/18 14:55:43 mbox Exp $ ';

=head1 LXR::Lang::Perl

Da Perl package, man!

=cut

use strict;
use LXR::Common;
use LXR::Lang;

use vars qw(@ISA);
@ISA = ('LXR::Lang');

my @spec = (
			'atom'		=> ('\$\W?',	''),
			'atom'		=> ('\\\\.',	''),
			'include'	=> ('\buse\s+',	';'),
			'include'	=> ('\brequire\s+',	';'),
			'string'	=> ('"',		'"'),
			'comment'	=> ('#',		"\$"),
			'comment'	=> ("^=\\w+",	"^=cut"),
			'string'	=> ("'",		"'"));


sub new {
	my ($self, $pathname, $release) = @_;

	$self = bless({}, $self);

	$$self{'release'} = $release;

   	return $self;
}

sub parsespec {
	return @spec;
}

sub processcode {
	my ($self, $code, @itag) = @_;
	my $sym;

#	$$code =~ s#([\@\$\%\&\*])([a-z0-9_]+)|\b([a-z0-9_]+)(\s*\()#
#		$sym = $2 || $3;
#		$1.($index->issymbol($sym, $$self{'release'})
#			? join($sym, @{$$self{'itag'}})
#			: $sym).$4#geis;
	
	$$code =~ s#\b([a-z][a-z0-9_:]*)\b#
		($index->issymbol($1, $$self{'release'})
		 ? join($1, @{$$self{'itag'}})
		 : $1)#geis;
}

sub modref {
	my $mod = shift;
	my $file = $mod;

	$file =~ s,::,/,g;
	$file .= ".pm";

	return &LXR::Common::incref($mod, "include", $file);
}

sub processinclude {
	my ($self, $frag, $dir) = @_;
	
	$$frag =~ s/(use\s+|require\s+)([\w:]+)/$1.modref($2)/e;
}

sub processcomment {
	my ($self, $comm) = @_;

	if ($$comm =~ /^=/s) {
		# Pod text

		$$comm = join('', map {
			if (/^=head(\d)\s*(.*)/s) {
				"<span class=\"pod\"><font size=\"+".(4-$1)."\">$2<\/font></span>";
			}
			elsif (/^=item\s*(.*)/s) {
				"<span class=\"podhead\">* $1 ".
					("-" x (67 - length($1)))."<\/span>";
			}
			elsif (/^=(pod|cut)/s) {
				"<span class=\"podhead\">".
					("-" x 70)."<\/span>";
			}
			elsif (/^=.*/s) {
				"";
			}
			else {
				if (/^\s/s) {	# Verbatim paragraph
					s|^(.*)$|<span class="pod"><code>$1</code></span>|gm;
				}
				else {			# Normal paragraph
					s|^(.*)$|<span class="pod">$1</span>|gm;
					s/C\0\<(.*?)\0\>/<code>$1<\/code>/g;
				}
				$_;
			}
		} split(/((?:\n[ \t]*)*\n)/, $$comm));
	}
	else {
		$$comm =~ s|^(.*)$|<span class='comment'>$1</span>|gm;
	}
}


sub indexfile {
	my ($self, $name, $path, $fileid, $index, $config) = @_;

	open(PLTAG, $path);
		
	while (<PLTAG>) {
		if (/^sub\s+(\w+)/) {
			print(STDERR "Sub: $1\n");
			$index->index($1, $fileid, $., 'f');
		}
		elsif (/^package\s+([\w:]+)/) {
			print(STDERR "Class: $1\n");
			$index->index($1, $fileid, $., 'c');
		}
		elsif (/^=item\s+[\@\$\%\&\*]?(\w+)/) {
			print(STDERR "Doc: $1\n");
			$index->index($1, $fileid, $., 'i');
		}
	}
	close(PLTAG);
}

