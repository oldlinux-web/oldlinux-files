# -*- tab-width: 4 -*- ###############################################
#
# $Id: CVS.pm,v 1.17 2002/02/03 08:22:08 mbox Exp $

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

package LXR::Files::CVS;

$CVSID = '$Id: CVS.pm,v 1.17 2002/02/03 08:22:08 mbox Exp $ ';

use strict;
use FileHandle;
use Time::Local;
use LXR::Common;

use vars qw(%cvs $cache_filename);

sub new {
	my ($self, $rootpath) = @_;

	$self = bless({}, $self);
	$self->{'rootpath'} = $rootpath;
	$self->{'rootpath'} =~ s@/*$@/@;

	return $self;
}

sub filerev {
	my ($self, $filename, $release) = @_;

	if ($release =~ /rev_([\d\.]+)/) {
		return $1;
	}
	elsif ($release =~ /^([\d\.]+)$/) {
		return $1;
	}
	else {
		$self->parsecvs($filename);
		return $cvs{'header'}{'symbols'}{$release};
	}
}								

sub getfiletime {
	my ($self, $filename, $release) = @_;

	return undef if $self->isdir($filename, $release);

	$self->parsecvs($filename);

	my $rev = $self->filerev($filename, $release);

	return undef unless defined($rev);

	my @t = reverse(split(/\./, $cvs{'branch'}{$rev}{'date'}));

	return undef unless @t;

	$t[4]--;
	return timegm(@t);
}

sub getfilesize {
	my ($self, $filename, $release) = @_;

	return length($self->getfile($filename, $release));
}

sub getfile {
	my ($self, $filename, $release) = @_;

	my $fileh = $self->getfilehandle($filename, $release);
	return undef unless $fileh;
	return join('', $fileh->getlines);
}

sub getannotations {
	my ($self, $filename, $release) = @_;

	$self->parsecvs($filename);

	my $rev = $self->filerev($filename, $release);
	return undef unless defined($rev);

	my $hrev = $cvs{'header'}{'head'};
	my $lrev;
	my @anno;
	my $headfh = $self->getfilehandle($filename, $release);
	my @head = $headfh->getlines;

	while (1) {
		if ($rev eq $hrev) {
			@head = 0..$#head;
		}
		
		$lrev = $hrev;
		$hrev = $cvs{'branch'}{$hrev}{'next'} || last;
		
		my @diff = $self->getdiff($filename, $lrev, $hrev);
		my $off = 0;
		
		while (@diff) {
			my $dir = shift(@diff);
			
			if ($dir =~ /^a(\d+)\s+(\d+)/) {
				splice(@diff, 0, $2);
				splice(@head, $1-$off, 0, ('') x $2);
				$off -= $2;
			}
			elsif ($dir =~ /^d(\d+)\s+(\d+)/) {
				map {
					$anno[$_] = $lrev if $_ ne '';
				} splice(@head, $1-$off-1, $2);
				
				$off += $2;
			}
			else {
				warn("Oops! Out of sync!");
			}
		}
	}

	map {
		$anno[$_] = $lrev if $_ ne '';
	} @head;

#	print(STDERR "** Anno: ".scalar(@anno).join("\n", '', @anno, ''));
	return @anno;
}

sub getauthor {
	my ($self, $filename, $revision) = @_;

	$self->parsecvs($filename);

	return $cvs{'branch'}{$revision}{'author'};
}

sub getfilehandle {
	my ($self, $filename, $release) = @_;
	my ($fileh);

	$self->parsecvs($filename);

	my $rev = $self->filerev($filename, $release);
	return undef unless defined($rev);

	$fileh = new FileHandle("co -q -p$rev ".
							$self->toreal($filename, $release).
							" |"); # FIXME: Exploitable?
	die("Error execting \"co\", rcs not installed?") unless $fileh;
	return $fileh;
}

sub getdiff {
	my ($self, $filename, $release1, $release2) = @_;
	my ($fileh);

	$self->parsecvs($filename);

	my $rev1 = $self->filerev($filename, $release1);
	return undef unless defined($rev1);

	my $rev2 = $self->filerev($filename, $release2);
	return undef unless defined($rev2);

	$fileh = new FileHandle("rcsdiff -q -a -n -r$rev1 -r$rev2 ".
							$self->toreal($filename, $release1).
							" |"); # FIXME: Exploitable?
	die("Error execting \"rcsdiff\", rcs not installed?") unless $fileh;
	return $fileh->getlines;
}

sub tmpfile {
	my ($self, $filename, $release) = @_;
	my ($tmp, $buf);

	$buf = $self->getfile($filename, $release);
	return undef unless defined($buf);
	
	$tmp = $config->tmpdir.'/lxrtmp.'.time.'.'.$$.'.'.&LXR::Common::tmpcounter;
	open(TMP, "> $tmp") || return undef;
	print(TMP $buf);
	close(TMP);
	
	return $tmp;
}

sub dirempty {
	my ($self, $pathname, $release) = @_;
	my ($node, @dirs, @files);
	my $DIRH = new IO::Handle;
	my $real = $self->toreal($pathname, $release);

	opendir($DIRH, $real) || return 1;
	while (defined($node = readdir($DIRH))) {
		next if $node =~ /^\.|~$|\.orig$/;
		next if $node eq 'CVS';

		if (-d $real.$node) {
			push(@dirs, $node.'/'); 
		}
		elsif ($node =~ /(.*),v$/) {
			push(@files, $1);
		}
	}
	closedir($DIRH);

	foreach $node (@files) {
		return 0 if $self->filerev($pathname.$node, $release);
	}

	foreach $node (@dirs) {
		return 0 unless $self->dirempty($pathname.$node, $release);
	}
	return 1;
}

sub getdir {
	my ($self, $pathname, $release) = @_;
	my ($node, @dirs, @files);
	my $DIRH = new IO::Handle;
	my $real = $self->toreal($pathname, $release);

	opendir($DIRH, $real) || return ();
	while (defined($node = readdir($DIRH))) {
		next if $node =~ /^\.|~$|\.orig$/;
		next if $node eq 'CVS';

		if (-d $real.$node) {
			if ($node eq 'Attic') {
				push(@files, $self->getdir($pathname.$node.'/', $release));
			}
			else {
				push(@dirs, $node.'/') 
					unless defined($release) 
						&& $self->dirempty($pathname.$node.'/', $release);
			}
		}
		elsif ($node =~ /(.*),v$/) {
			push(@files, $1) 
				if ! defined($release) 
					|| $self->getfiletime($pathname.$1, $release);
		}
	}
	closedir($DIRH);

	return (sort(@dirs), sort(@files));
}

sub toreal {
	my ($self, $pathname, $release) = @_;
	my $real = $self->{'rootpath'}.$pathname;

	return $real if -d $real;
	return $real.',v' if -f $real.',v';
	
	$real =~ s|(/[^/]+/?)$|/Attic$1|;

	return $real if -d $real;
	return $real.',v' if -f $real.',v';

	return undef;
}

sub isdir {
	my ($self, $pathname, $release) = @_;

	return -d $self->toreal($pathname, $release);
}

sub isfile {
	my ($self, $pathname, $release) = @_;

	return -f $self->toreal($pathname, $release);
}

sub getindex {
	my ($self, $pathname, $release) = @_;

	my $index = $self->getfile($pathname, $release);

	return $index =~ /\n(\S*)\s*\n\t-\s*([^\n]*)/gs;
}

sub allreleases {
	my ($self, $filename) = @_;

	$self->parsecvs($filename);

	return sort(keys(%{$cvs{'header'}{'symbols'}}));
}

sub allrevisions {
	my ($self, $filename) = @_;

	$self->parsecvs($filename);

	return sort(keys(%{$cvs{'branch'}}));
}

sub parsecvs {
	# Actually, these days it just parses the header.
	# RCS tools are much better at parsing RCS files.
	# -pok
	my ($self, $filename) = @_;

	return if $cache_filename eq $filename;
	$cache_filename = $filename;

	%cvs = ();

	my $file = '';
	open (CVS, $self->toreal($filename, undef));
	while (<CVS>) {
		if (/^text\s*$/) {
			# stop reading when we hit the text.
			last;
		}
		$file .= $_;
	}
	close (CVS);

	my @cvs = $file =~ /((?:(?:[^\n@]+|@[^@]*@)\n?)+)/gs;

	$cvs{'header'} = { map { s/@@/@/gs;
							 /^@/s && substr($_, 1, -1) || $_ }
					   shift(@cvs) =~ /(\w+)\s*((?:[^;@]+|@[^@]*@)*);/gs };

	$cvs{'header'}{'symbols'}
	= { $cvs{'header'}{'symbols'} =~ /(\S+?):(\S+)/g };
	
	my ($orel, $nrel, $rev);
	while (($orel, $rev) = each %{$cvs{'header'}{'symbols'}}) {
		$nrel = $config->cvsversion($orel);
		next unless defined($nrel);

		if ($nrel ne $orel) {
			delete($cvs{'header'}{'symbols'}{$orel});
			$cvs{'header'}{'symbols'}{$nrel} = $rev if $nrel;
		}
	}

	$cvs{'header'}{'symbols'}{'head'} = $cvs{'header'}{'head'};

	while (@cvs && $cvs[0] !~ /\s*desc/s) {
		my ($r, $v) = shift(@cvs) =~ /\s*(\S+)\s*(.*)/s;
		$cvs{'branch'}{$r} = { map { s/@@/@/gs;
									 /^@/s && substr($_, 1, -1) || $_ }
							   $v =~ /(\w+)\s*((?:[^;@]+|@[^@]*@)*);/gs };
	}
	
	$cvs{'desc'} = shift(@cvs) =~ /\s*desc\s+((?:[^\n@]+|@[^@]*@)*)\n/s;
	$cvs{'desc'} =~ s/^@|@($|@)/$1/gs;

}

1;
