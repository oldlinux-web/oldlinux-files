# -*- tab-width: 4 -*- ###############################################
#
# $Id: SimpleParse.pm,v 1.14 2001/11/14 15:03:29 mbox Exp $

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

package LXR::SimpleParse;

$CVSID = '$Id: SimpleParse.pm,v 1.14 2001/11/14 15:03:29 mbox Exp $ ';

use strict;
use integer;

require Exporter;

use vars qw(@ISA @EXPORT);

@ISA = qw(Exporter);
@EXPORT = qw(&doparse &untabify &init &nextfrag);

my $fileh;			# File handle
my @frags;			# Fragments in queue
my @bodyid;			# Array of body type ids
my @open;			# Fragment opening delimiters
my @term;			# Fragment closing delimiters
my $split;			# Fragmentation regexp
my $open;			# Fragment opening regexp
my $tabwidth;		# Tab width

sub init {
    my @blksep;
	
	$fileh = "";
	@frags = ();
	@bodyid = ();
	@open = ();
	@term = ();
	$split = "";
	$open = "";
	$tabwidth = 8;
	my $tabhint;

    ($fileh, $tabhint, @blksep) = @_;
	$tabwidth = $tabhint || $tabwidth;
		
    while (@_ = splice(@blksep,0,3)) {
		push(@bodyid, $_[0]);
		push(@open, $_[1]);
		push(@term, $_[2]);
    }

    foreach (@open) {
		$open .= "($_)|";
		$split .= "$_|";
    }
    chop($open);
    
    foreach (@term) {
		next if $_ eq '';
		$split .= "$_|";
    }
    chop($split);
}


sub untabify {
    my $t = $_[1] || 8;

    $_[0] =~ s/^(\t+)/(' ' x ($t * length($1)))/ge; # Optimize for common case.
    $_[0] =~ s/([^\t]*)\t/$1.(' ' x ($t - (length($1) % $t)))/ge;
    return($_[0]);
}


sub nextfrag {
    my $btype = undef;
    my $frag = undef;
	my $line = '';

#	print "nextfrag called\n";

    while (1) {

		# read one more line if we have processed 
		# all of the previously read line
		if ($#frags < 0) {
			$line = $fileh->getline;
			
			if ($. <= 2 &&
				$line =~ /^.*-[*]-.*?[ \t;]tab-width:[ \t]*([0-9]+).*-[*]-/) {
				$tabwidth = $1;
			}
			
#			&untabify($line, $tabwidth); # We inline this for performance.
			
			# Optimize for common case.
			if(defined($line)) {
				$line =~ s/^(\t+)/' ' x ($tabwidth * length($1))/ge;
				$line =~ s/([^\t]*)\t/$1.(' ' x ($tabwidth - (length($1) % $tabwidth)))/ge;
				
				# split the line into fragments
				@frags = split(/($split)/, $line);
			}
		}

		last if $#frags < 0;
		
		# skip empty fragments
		if ($frags[0] eq '') {
			shift(@frags);
		}

		# check if we are inside a fragment
		if (defined($frag)) {
			if (defined($btype)) {
				my $next = shift(@frags);
				
				# Add to the fragment
				$frag .= $next;
				# We are done if this was the terminator
				last if $next =~ /^$term[$btype]$/;

			}
			else {
				if ($frags[0] =~ /^$open$/) {
#					print "encountered open token while btype was $btype\n";
					last;
				}
				$frag .= shift(@frags);
			}
		}
		else {
#			print "start of new fragment\n";
			# Find the blocktype of the current block
			$frag = shift(@frags);
			if (defined($frag) && (@_ = $frag =~ /^$open$/)) {
#				print "hit\n";
				# grep in a scalar context returns the number of times
				# EXPR evaluates to true, which is this case will be
				# the index of the first defined element in @_.

				my $i = 1;
				$btype = grep { $i &&= !defined($_) } @_;
				if(!defined($term[$btype])) {
					print "fragment without terminator\n";
					last;
				}
			}
		}
    }
    $btype = $bodyid[$btype] if defined($btype);
    
    return($btype, $frag);
}


1;
