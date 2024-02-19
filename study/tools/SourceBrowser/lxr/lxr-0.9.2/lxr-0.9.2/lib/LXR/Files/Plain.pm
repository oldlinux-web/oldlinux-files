# -*- tab-width: 4 -*- ###############################################
#
# $Id: Plain.pm,v 1.19 2002/02/26 15:57:55 mbox Exp $

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

package LXR::Files::Plain;

$CVSID = '$Id: Plain.pm,v 1.19 2002/02/26 15:57:55 mbox Exp $ ';

use strict;
use FileHandle;
use LXR::Common;

sub new {
	my ($self, $rootpath) = @_;

	$self = bless({}, $self);
	$self->{'rootpath'} = $rootpath;
	$self->{'rootpath'} =~ s@/*$@/@;

	return $self;
}

sub filerev {
	my ($self, $filename, $release) = @_;

#	return $release;
	return join("-", $self->getfiletime($filename, $release),
				$self->getfilesize($filename, $release));
}								

sub getfiletime {
	my ($self, $filename, $release) = @_;

	return (stat($self->toreal($filename, $release)))[9];
}

sub getfilesize {
	my ($self, $filename, $release) = @_;

	return -s $self->toreal($filename, $release);
}

sub getfile {
	my ($self, $filename, $release) = @_;
	my ($buffer);
	local ($/) = undef;

	open(FILE, "<", $self->toreal($filename, $release)) || return undef;
	$buffer = <FILE>;
	close(FILE);
	return $buffer;
}

sub getfilehandle {
	my ($self, $filename, $release) = @_;
	my ($fileh);

	$fileh = new FileHandle($self->toreal($filename, $release));
	return $fileh;
}

sub tmpfile {
	my ($self, $filename, $release) = @_;
	my ($tmp, $tries);
	local ($/) = undef;

	$tmp = $config->tmpdir.'/lxrtmp.'.time.'.'.$$.'.'.&LXR::Common::tmpcounter;
	open(TMP, "> $tmp") || return undef;
	open(FILE, "<", $self->toreal($filename, $release)) || return undef;
	print(TMP <FILE>);
	close(FILE);
	close(TMP);
	
	return $tmp;
}

sub getannotations {
	return ();
}

sub getauthor {
	return undef;
}

sub getdir {
	my ($self, $pathname, $release) = @_;
	my ($dir, $node, @dirs, @files);

	$dir = $self->toreal($pathname, $release);
	opendir(DIR, $dir) || die ("Can't open $dir");
	while (defined($node = readdir(DIR))) {
		next if $node =~ /^\.|~$|\.orig$/;
		next if $node eq 'CVS';

		if (-d $dir.$node) {
			push(@dirs, $node.'/');
		}
		else {
			push(@files, $node);
		}
	}
	closedir(DIR);

	return sort(@dirs), sort(@files);
}

# This function should not be used outside this module
# except for printing error messages
# (I'm not sure even that is legitimate use, considering
# other possible File classes.)

sub toreal {
	my ($self, $pathname, $release) = @_;
	
	return ($self->{'rootpath'}.$release.$pathname);
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
	my ($index, %index);
	my $indexname = $self->toreal($pathname, $release)."00-INDEX";

	if (-f $indexname) {
		open(INDEX, "<", $indexname) || 
			warning("Existing $indexname could not be opened.");
		local($/) = undef;
		$index = <INDEX>;

		%index = $index =~ /\n(\S*)\s*\n\t-\s*([^\n]*)/gs;
	}
	return %index;
}

sub allreleases {
	my ($self, $filename) = @_;

	opendir(SRCDIR, $self->{'rootpath'});
	my @dirs = readdir(SRCDIR);
	closedir(SRCDIR);

	return grep { /^[^\.]/ && -r $self->toreal($filename, $_) } @dirs;
}


1;
