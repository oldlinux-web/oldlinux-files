# -*- tab-width: 4 -*- ###############################################
#
# $Id: Common.pm,v 1.43 2002/07/29 01:17:32 mbox Exp $
#
# FIXME: java doesn't support super() or super.x

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

package LXR::Common;

$CVSID = '$Id: Common.pm,v 1.43 2002/07/29 01:17:32 mbox Exp $ ';

use strict;

require Exporter;

use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS 
			$files $index $config $pathname $identifier $release
			$HTTP $wwwdebug $tmpcounter);

@ISA		= qw(Exporter);

@EXPORT		= qw($files $index $config &fatal);
@EXPORT_OK	= qw($files $index $config $pathname $identifier $release
				 $HTTP
				 &warning &fatal &abortall &fflush &urlargs &fileref
				 &idref &incref &htmlquote &freetextmarkup &markupfile
				 &markupstring &httpinit &makeheader &makefooter
				 &expandtemplate &httpclean);

%EXPORT_TAGS = ('html' => [ @EXPORT_OK ]);


require Local;
require LXR::SimpleParse;
require LXR::Config;
require LXR::Files;
require LXR::Index;
require LXR::Lang;


$wwwdebug = 1;

$tmpcounter = 23;


sub warning {
	my $c = join(", line ", (caller)[0,2]);
	print(STDERR "[",scalar(localtime),"] warning: $c: $_[0]\n");
	print("<h4 align=\"center\"><i>** Warning: $_[0]</i></h4>\n") if $wwwdebug;
}


sub fatal {
	my $c = join(", line ", (caller)[0,2]);
	print(STDERR "[",scalar(localtime),"] fatal: $c: $_[0]\n");
	print(STDERR '[@INC ', join(" ", @INC), ' $0 ', $0, "\n");
	print(STDERR '$config', join(" ", %$config), "\n") if  ref($config) eq "HASH";
	print("<h4 align=\"center\"><i>** Fatal: $_[0]</i></h4>\n") if $wwwdebug;
	exit(1);
}


sub abortall {
	my $c = join(", line ", (caller)[0,2]);
	print(STDERR "[",scalar(localtime),"] abortall: $c: $_[0]\n");
	print("Content-Type: text/html; charset=iso-8859-1\n\n",
		  "<html>\n<head>\n<title>Abort</title>\n</head>\n",
		  "<body><h1>Abort!</h1>\n",
		  "<b><i>** Aborting: $_[0]</i></b>\n",
		  "</body>\n</html>\n") if $wwwdebug;
	exit(1);
}


sub fflush {
	$| = 1; print('');
}


sub tmpcounter {
	return $tmpcounter++;
}

sub urlargs {
	my @args = @_;
	my %args = ();
	my $val;

	foreach (@args) {
		$args{$1} = $2 if /(\S+)=(\S*)/;
	}
	@args = ();

	foreach ($config->allvariables) {
		$val = $args{$_} || $config->variable($_);
		push(@args, "$_=$val") unless ($val eq $config->vardefault($_));
		delete($args{$_});
	}

	foreach (keys(%args)) {
		push(@args, "$_=$args{$_}");
	}

	return ($#args < 0 ? '' : '?'.join(';',@args));
}	 

sub fileref {
	my ($desc, $css, $path, $line, @args) = @_;

	# jwz: URL-quote any special characters.
	$path =~ s|([^-a-zA-Z0-9.\@/_\r\n])|sprintf("%%%02X", ord($1))|ge;

	if ($line > 0 && length($line) < 3) {
		$line = ('0' x (3-length($line))).$line;
	}

	return ("<a class='$css' href=\"$config->{virtroot}/source$path".
			&urlargs(@args).
			($line > 0 ? "#$line" : "").
			"\"\>$desc</a>");
}

sub diffref {
	my ($desc, $css, $path, $darg) = @_;
	my $dval;

	($darg, $dval) = $darg =~ /(.*?)=(.*)/;
	return ("<a class='$css' href=\"$config->{virtroot}/diff$path".
			&urlargs(($darg ? "diffvar=$darg" : ""),
					 ($dval ? "diffval=$dval" : "")).
			"\"\>$desc</a>");
}


sub idref {
	my ($desc, $css, $id, @args) = @_;
	return ("<a class='$css' href=\"$config->{virtroot}/ident".
			&urlargs(($id ? "i=$id" : ""),
					 @args).
			"\"\>$desc</a>");
}


sub incref {
	my ($name, $css, $file, @paths) = @_;
	my ($dir, $path);

	push(@paths, $config->incprefix);

	foreach $dir (@paths) {
		$dir =~ s/\/+$//;
		$path = $config->mappath($dir."/".$file);
		return &fileref($name, $css, $path) if $files->isfile($path, $release);
		
	}
	
	return $name;
}


sub http_wash {
	my $t = shift;
	if(!defined($t)) {
		return(undef);
	}

	$t =~ s/\+/ /g;
	$t =~ s/\%([\da-f][\da-f])/pack("C", hex($1))/gie;

	# Paranoia check. Regexp-searches in Glimpse won't work.
	# if ($t =~ tr/;<>*|\`&$!#()[]{}:\'\"//) {

	# Should be sufficient to keep "open" from doing unexpected stuff.
	if ($t =~ tr/<>|\"\'\`//) {
		&abortall("Illegal characters in HTTP-parameters.");
	}
	
	return($t);
}

# dme: Smaller version of the markupfile function meant for marking up 
# the descriptions in source directory listings.
sub markupstring {
	my ($string, $virtp) = @_;
	
	# Mark special characters so they don't get processed just yet.
	$string =~ s/([\&\<\>])/\0$1/g;
	
	# Look for identifiers and create links with identifier search query.
	# TODO: Is there a performance problem with this?
	$string =~ s#(^|\s)([a-zA-Z_~][a-zA-Z0-9_]*)\b#
		$1.(is_linkworthy($2) ? &idref($2, "", $2) : $2)#ge;
	
	# HTMLify the special characters we marked earlier,
	# but not the ones in the recently added xref html links.
	$string=~ s/\0&/&amp;/g;
	$string=~ s/\0</&lt;/g;
	$string=~ s/\0>/&gt;/g;
	
	# HTMLify email addresses and urls.
	$string =~ s#((ftp|http|nntp|snews|news)://(\w|\w\.\w|\~|\-|\/|\#)+(?!\.\b))#<a href=\"$1\">$1</a>#g;
	# htmlify certain addresses which aren't surrounded by <>
	$string =~ s/([\w\-\_]*\@netscape.com)(?!&gt;)/<a href=\"mailto:$1\">$1<\/a>/g;
	$string =~ s/([\w\-\_]*\@mozilla.org)(?!&gt;)/<a href=\"mailto:$1\">$1<\/a>/g;
	$string =~ s/([\w\-\_]*\@gnome.org)(?!&gt;)/<a href=\"mailto:$1\">$1<\/a>/g;
	$string =~ s/([\w\-\_]*\@linux.no)(?!&gt;)/<a href=\"mailto:$1\">$1<\/a>/g;
	$string =~ s/([\w\-\_]*\@sourceforge.net)(?!&gt;)/<a href=\"mailto:$1\">$1<\/a>/g;
	$string =~ s/([\w\-\_]*\@sf.net)(?!&gt;)/<a href=\"mailto:$1\">$1<\/a>/g;
	$string =~ s/(&lt;)(.*@.*)(&gt;)/$1<a href=\"mailto:$2\">$2<\/a>$3/g;
	
	# HTMLify file names, assuming file is in the current directory.
	$string =~ s#\b(([\w\-_\/]+\.(c|h|cc|cp|hpp|cpp|java))|README)\b#<a href=\"$config->{virtroot}/source$virtp$1\">$1</a>#g;
	
	return($string);
}

# dme: Return true if string is in the identifier db and it seems like its
# use in the sentence is as an identifier and its not just some word that
# happens to have been used as a variable name somewhere. We don't want
# words like "of", "to" and "a" to get links. The string must be long 
# enough, and  either contain "_" or if some letter besides the first 
# is capitalized
sub is_linkworthy{
	my ($string) = @_;

	if ($string =~ /....../ 
		&& ($string =~ /_/ || $string =~ /.[A-Z]/)
#		&& defined($xref{$string}) FIXME
		) {
		return (1);
	}
	else {
		return (0);
	}
}

sub markspecials {
	$_[0] =~ s/([\&\<\>])/\0$1/g;
}


sub htmlquote {
	$_[0] =~ s/\0&/&amp;/g;
	$_[0] =~ s/\0</&lt;/g;
	$_[0] =~ s/\0>/&gt;/g;
}


sub freetextmarkup {
	$_[0] =~ s{((f|ht)tp://[^\s<>\0]*[^\s<>\0.])}
			  {<a class='offshore' href="$1">$1</a>}g;
	$_[0] =~ s{(\0<([^\s<>\0]+@[^\s<>\0]+)\0>)}
			  {<a class='offshore' href="mailto:$2">$1</a>}g;
}


sub markupfile {
	#_PH_ supress block is here to avoid the <pre> tag output
	#while called from diff
	my ($fileh, $outfun) = @_;
	my ($dir) = $pathname =~ m|^(.*/)|;

	my $line = '001';
	my @ltag = &fileref(1, "fline", $pathname, 1) =~ /^(<a)(.*\#)001(\">)1(<\/a>)$/;
	$ltag[0] .= ' name=';
	$ltag[3] .= " ";
	
	my @itag = &idref(1, "fid", 1) =~ /^(.*=)1(\">)1(<\/a>)$/;
	my $lang = new LXR::Lang($pathname, $release, @itag);
	
	# A source code file
	if ($lang) {
		my $language = $lang->language;  # To get back to the key to lookup the tabwidth.
	  
	    &LXR::SimpleParse::init($fileh, $config->filetype->{$language}[3], $lang->parsespec);

		my ($btype, $frag) = &LXR::SimpleParse::nextfrag;

		#&$outfun("<pre class=\"file\">\n");
		&$outfun(join($line++, @ltag)) if defined($frag);
		
		while (defined($frag)) {
			&markspecials($frag);

			if ($btype eq 'comment') {
				# Comment
				# Convert mail adresses to mailto:
				&freetextmarkup($frag);
				$lang->processcomment(\$frag);
			} 
			elsif ($btype eq 'string') {
				# String
				$frag = "<span class='string'>$frag</span>";
			} 
			elsif ($btype eq 'include') { 
				# Include directive
				$lang->processinclude(\$frag, $dir);
			} 
			else {
				# Code
				$lang->processcode(\$frag);
			}
		
			&htmlquote($frag);
			my $ofrag = $frag;

			($btype, $frag) = &LXR::SimpleParse::nextfrag;

			$ofrag =~ s/\n$// unless defined($frag);
			$ofrag =~ s/\n/"\n".join($line++, @ltag)/ge;

			&$outfun($ofrag);
		}
		#&$outfun("</pre>");
	} 
	elsif ($pathname =~ /$config->graphicfile/) {
		&$outfun("<ul><table><tr><th valign=\"center\"><b>Image: </b></th>");
		&$outfun("<img src=\"$config->{virtroot}/source".
				 $pathname.&urlargs("raw=1").
				 "\" border=\"0\" alt=\"$pathname\">\n");
		&$outfun("</tr></td></table></ul>");
	} 
	elsif ($pathname =~ m|/CREDITS$|) {
		while (defined($_ = $fileh->getline)) {
			&LXR::SimpleParse::untabify($_);
			&markspecials($_);
			&htmlquote($_);
			s/^N:\s+(.*)/<strong>$1<\/strong>/gm;
			s/^(E:\s+)(\S+@\S+)/$1<a href=\"mailto:$2\">$2<\/a>/gm;
			s/^(W:\s+)(.*)/$1<a href=\"$2\">$2<\/a>/gm;
			# &$outfun("<a name=\"L$.\"><\/a>".$_);
			&$outfun(join($line++, @ltag).$_);
		}
	} 
	else {
		return unless defined ($_ = $fileh->getline);

		# If it's not a script or something with an Emacs spec header and
		# the first line is very long or containts control characters...
		if (! /^\#!/ && ! /-\*-.*-\*-/i &&
			(length($_) > 132 || /[\000-\010\013\014\016-\037\200-Ÿ]/)) 
		{
			# We postulate that it's a binary file.
			&$outfun("<ul><b>Binary File: ");
			
			# jwz: URL-quote any special characters.
			my $uname = $pathname;
			$uname =~ s|([^-a-zA-Z0-9.\@/_\r\n])|sprintf("%%%02X", ord($1))|ge;
			
			&$outfun("<a href=\"$config->{virtroot}/source".$uname.
					 &urlargs("raw=1")."\">");
			&$outfun("$pathname</a></b>");
			&$outfun("</ul>");
			
		} 
		else {
			#&$outfun("<pre class=\"file\">\n");
			do {
				&LXR::SimpleParse::untabify($_);
				&markspecials($_);
				&freetextmarkup($_);
				&htmlquote($_);
				#		&$outfun("<a name=\"L$.\"><\/a>".$_);
				&$outfun(join($line++, @ltag).$_);
			} while (defined($_ = $fileh->getline));
			#&$outfun("</pre>");
		}
	}
}


sub fixpaths {
	my $node = '/'.shift;
	
	while ($node =~ s|/[^/]+/\.\./|/|g) {}
	$node =~ s|/\.\./|/|g;
	
	$node .= '/' if $files->isdir($node);
	$node =~ s|//+|/|g;
	
	return $node;
}


sub printhttp {

	# Print out a Last-Modified date that is the larger of: the
	# underlying file that we are presenting; and the "source" script
	# itself (passed in as an argument to this function.)  If we can't
	# stat either of them, don't print out a L-M header.  (Note that this
	# stats lxr/source but not lxr/lib/LXR/Common.pm.  Oh well, I can
	# live with that I guess...)	-- jwz, 16-Jun-98
	
	# Made it stat all currently loaded modules.  -- agg.

	# Todo: check lxr.conf.

	print("HTTP/1.0 200 OK\n");

	my $time = $files->getfiletime($pathname, $release);
	my $time2 = (stat($config->confpath))[9];
	$time = $time2 if $time2 > $time;

	# Remove this to see if we get a speed increase by not stating all
	# the modules.  Since for most sites the modules change rarely,
	# this is a big hit for each access.
	
# 	my %mods = ('main' => $0, %INC);
# 	my ($mod, $path);
# 	while (($mod, $path) = each %mods) {
# 		$mod  =~ s/.pm$//;
# 		$mod  =~ s|/|::|g;
# 		$path =~ s|/+|/|g;

# 		no strict 'refs';
# 		next unless $ {$mod.'::CVSID'};

# 		$time2 = (stat($path))[9];
# 		$time = $time2 if $time2 > $time;
# 	}

	if ($time > 0) {
		my ($sec, $min, $hour, $mday, $mon, $year,$wday) = gmtime($time);
		my @days = ("Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun");
		my @months = ("Jan", "Feb", "Mar", "Apr", "May", "Jun",
					  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec");
		$year += 1900;
		$wday = $days[$wday];
		$mon = $months[$mon];
		# Last-Modified: Wed, 10 Dec 1997 00:55:32 GMT
		printf("Last-Modified: %s, %2d %s %d %02d:%02d:%02d GMT\n",
			   $wday, $mday, $mon, $year, $hour, $min, $sec);
	}

	if ($HTTP->{'param'}->{'raw'}) {
		#FIXME - need more types here
		my %type = ('gif'	=> 'image/gif',
					'html'	=> 'text/html');

		if ($pathname =~ /\.([^.]+)$/ && $type{$1}) {
			print("Content-type: ", $type{$1}, "\n");
		}
		else {
			print("Content-Type: text/plain\n");
		}
	} 
	else {
		print("Content-Type: text/html; charset=iso-8859-1\n");
		# print("Content-Type: text/html\n");
	}

	# Close the HTTP header block.
	print("\n");
}

# init - Returns the array ($config, $HTTP, $Path)
#
# Path:
# file	- Name of file without path
# realf - The current file
# real	- The directory portion of the current file
# root	- The root of the sourcecode, same as sourceroot in $config
# virtf - Name of file within the sourcedir 
# virt	- Directory portion of same
# xref	- Links to the different portions of the patname
#
# HTTP:
# path_info - 
# param		- Array of parameters
# this_url	- The current url
#
# config:
# maplist -		A list of the different mappings 
#				that are applied to the filename
# sourcedirhead - Corresponds to the configig options
# sourcehead -
# htmldir -
# sourceroot -
# htmlhead -
# incprefix -
# virtroot -
# glimpsebin -
# srcrootname -
# baseurl -
# htmltail -					  
sub httpinit {
	$SIG{__WARN__} = \&warning;
	$SIG{__DIE__}  = \&fatal;

	$HTTP->{'path_info'} = http_wash($ENV{'PATH_INFO'});

	$HTTP->{'this_url'}	 = 'http://'.$ENV{'SERVER_NAME'};
	$HTTP->{'this_url'}	.= ':'.$ENV{'SERVER_PORT'} if
		$ENV{'SERVER_PORT'} != 80;
	$HTTP->{'this_url'}	.= $ENV{'SCRIPT_NAME'}.$ENV{'PATH_INFO'};
	$HTTP->{'this_url'}	.= '?'.$ENV{'QUERY_STRING'} if
		$ENV{'QUERY_STRING'};

	$HTTP->{'param'} = { map { http_wash($_) }
						 $ENV{'QUERY_STRING'} =~ /([^;&=]+)(?:=([^;&]+)|)/g };

	$HTTP->{'param'}->{'v'} ||= $HTTP->{'param'}->{'version'};
	$HTTP->{'param'}->{'a'} ||= $HTTP->{'param'}->{'arch'};
	$HTTP->{'param'}->{'i'} ||= $HTTP->{'param'}->{'identifier'};

	$identifier = $HTTP->{'param'}->{'i'};
	$config = new LXR::Config($HTTP->{'this_url'});
	die "Can't find config for ".$HTTP->{'this_url'} if !defined($config);
	$files  = new LXR::Files($config->sourceroot);
	die "Can't create Files for ".$config->sourceroot if !defined($files);
	$index  = new LXR::Index($config->dbname);
	die "Can't create Index for ".$config->dbname if !defined($index);
	
	foreach ($config->allvariables) {
		$config->variable($_, $HTTP->{'param'}->{$_}) if $HTTP->{'param'}->{$_};
	}

	$release  = $config->variable('v');
	$pathname = fixpaths($HTTP->{'path_info'} || $HTTP->{'param'}->{'file'});

	printhttp;
}

sub httpclean {
	$config = undef;
	$files = undef;
	$index = undef;
}


sub expandtemplate {
	my ($templ, %expfunc) = @_;
	my ($expfun, $exppar);

	while ($templ =~ s/(\{[^\{\}]*)\{([^\{\}]*)\}/$1\01$2\02/s) {}
	
	$templ =~ s/(\$(\w+)(\{([^\}]*)\}|))/{
		if (defined($expfun = $expfunc{$2})) {
			if ($3 eq '') {
				&$expfun(undef);
			} 
			else {
				$exppar = $4;
				$exppar =~ s#\01#\{#gs;
				$exppar =~ s#\02#\}#gs;
				&$expfun($exppar);
			}
		} 
		else {
			$1;
		}
	}/ges;

	$templ =~ s/\01/\{/gs;
	$templ =~ s/\02/\}/gs;
	return($templ);
}


# What follows is somewhat less hairy way of expanding nested
# templates than it used to be.  State information is passed via
# function arguments, as God intended.
sub bannerexpand {
	my ($templ, $who) = @_;

	if ($who eq 'source' || $who eq 'sourcedir' || $who eq 'diff') {
		my $fpath = '';
		my $furl  = fileref($config->sourcerootname.'/', "banner", '/');
		
		foreach ($pathname =~ m|([^/]+/?)|g) {
			$fpath .= $_;

			# jwz: put a space after each / in the banner so that it's
			# possible for the pathnames to wrap.  The <wbr> tag ought
			# to do this, but it is ignored when sizing table cells,
			# so we have to use a real space.  It's somewhat ugly to
			# have these spaces be visible, but not as ugly as getting
			# a horizontal scrollbar...
			$furl .= ' '.fileref($_, "banner", "/$fpath");
		} 
		$furl =~ s|/</a>|</a>/|gi;
		
		return "<span class=\"banner\">$furl</span>";
	}
	else {
		return '';
	}
}

sub pathname {
	return $pathname;
}

sub titleexpand {
	my ($templ, $who) = @_;

	if ($who eq 'source' || $who eq 'diff' || $who eq 'sourcedir') {
		return $config->sourcerootname.$pathname;
	} 
	elsif ($who eq 'ident') {
		my $i = $HTTP->{'param'}->{'i'};
		return $config->sourcerootname.' identfier search'.($i ? " \"$i\"" : '');
	} 
	elsif ($who eq 'search') {
		my $s = $HTTP->{'param'}->{'string'};
		return $config->sourcerootname.' freetext search'.($s ? " \"$s\"" : '');
	} 
	elsif ($who eq 'find') {
		my $s = $HTTP->{'param'}->{'string'};
		return $config->sourcerootname.' file search'.($s ? " \"$s\"" : '');
	}
}


sub thisurl {
	my $url = $HTTP->{'this_url'};

	$url =~ s/([\?\&\;\=])/sprintf('%%%02x',(unpack('c',$1)))/ge;
	return($url);
}


sub baseurl {
	(my $url = $config->baseurl) =~ s|/*$|/|;

	return $url;
}

sub stylesheet {
	return $config->stylesheet;
}

sub dotdoturl {
	my $url = $config->baseurl;
	$url =~ s@/$@@;
	$url =~ s@/[^/]*$@@;
	return($url);
}

# This one isn't too bad either.  We just expand the "modes" template
# by filling in all the relevant values in the nested "modelink"
# template.
sub modeexpand {
	my ($templ, $who) = @_;
	my $modex = '';
	my @mlist = ();
	my $mode;
	
	if ($who eq 'source' || $who eq 'sourcedir') {
		push(@mlist, "<span class='modes-sel'>source navigation</span>");
	} 
	else {
		push(@mlist, fileref("source navigation", "modes", $pathname));
	}
	
	if ($who eq 'diff') {
		push(@mlist, "<span class='modes-sel'>diff markup</span>");
	} 
	elsif ($who eq 'source' && $pathname !~ m|/$|) {
		push(@mlist, diffref("diff markup", "modes", $pathname));
	}
	
	if ($who eq 'ident') {
		push(@mlist, "<span class='modes-sel'>identifier search</span>");
	} 
	else {
		push(@mlist, idref("identifier search", "modes", ""));
	}

	if ($who eq 'search') {
		push(@mlist, "<span class='modes-sel'>freetext search</span>");
	} 
	else {
		push(@mlist, "<a class=\"modes\" ".
			 "href=\"$config->{virtroot}/search".
			 urlargs."\">freetext search</a>");
	}
	
	if ($who eq 'find') {
		push(@mlist, "<span class='modes-sel'>file search</span>");
	} 
	else {
		push(@mlist, "<a class='modes' ".
			 "href=\"$config->{virtroot}/find".
			 urlargs."\">file search</a>");
	}
	
	foreach $mode (@mlist) {
		$modex .= expandtemplate($templ,
								 ('modelink' => sub { return $mode }));
	}
	
	return($modex);
}

# This is where it gets a bit tricky.  varexpand expands the
# "variables" template using varname and varlinks, the latter in turn
# expands the nested "varlinks" template using varval.
sub varlinks {
	my ($templ, $who, $var) = @_;
	my $vlex = '';
	my ($val, $oldval);
	my $vallink;
	
	$oldval = $config->variable($var);
	foreach $val ($config->varrange($var)) {
		if ($val eq $oldval) {
			$vallink = "<span class=\"var-sel\">$val</span>";
		} 
		else {
			if ($who eq 'source' || $who eq 'sourcedir') {
				$vallink = &fileref($val, "varlink",
									$config->mappath($pathname,
													 "$var=$val"),
									0,
									"$var=$val");

			} 
			elsif ($who eq 'diff') {
				$vallink = &diffref($val, "varlink", $pathname, "$var=$val");
			}
			elsif ($who eq 'ident') {
				$vallink = &idref($val, "varlink", $identifier, "$var=$val");
			} 
			elsif ($who eq 'search') {
				$vallink = "<a class=\"varlink\" href=\"$config->{virtroot}/search".
					&urlargs("$var=$val",
							 "string=".$HTTP->{'param'}->{'string'}).
								 "\">$val</a>";
			} 
			elsif ($who eq 'find') {
				$vallink = "<a class=\"varlink\" href=\"$config->{virtroot}/find".
					&urlargs("$var=$val",
							 "string=".$HTTP->{'param'}->{'string'}).
								 "\">$val</a>";
			}
		}

		$vlex .= expandtemplate($templ,
								('varvalue' => sub { return $vallink }));

	}
	return($vlex);
}


sub varexpand {
	my ($templ, $who) = @_;
	my $varex = '';
	my $var;
	
	foreach $var ($config->allvariables) {
		$varex .= expandtemplate
			($templ,
			 ('varname'	 => sub { $config->vardescription($var) },
			  'varlinks' => sub { varlinks(@_, $who, $var) }));
	}
	return($varex);
}


sub devinfo {
	my ($templ) = @_;
	my (@mods, $mod, $path);
	my %mods = ('main' => $0, %INC);

	while (($mod, $path) = each %mods) {
		$mod  =~ s/.pm$//;
		$mod  =~ s|/|::|g;
		$path =~ s|/+|/|g;

		no strict 'refs';
		next unless $ {$mod.'::CVSID'};

		push(@mods, [ $ {$mod.'::CVSID'}, $path, (stat($path))[9] ]);
	}

	return join('', 
				map { expandtemplate
						  ($templ, 
						   ('moduleid'	=> sub { $$_[0] },
							'modpath'	=> sub { $$_[1] },
							'modtime' 	=> sub { scalar(localtime($$_[2])) }));
				  }
				sort { $$b[2] <=> $$a[2] } @mods);
}


sub makeheader {
	my $who = shift;
	my $tmplname;
	my $template = "<html><body>\n<hr>\n";

	$tmplname = $who."head";
	
	unless ($who ne "sourcedir" || $config->sourcedirhead) {
		$tmplname = "sourcehead";
	}
	unless ($config->value($tmplname)) {
		$tmplname = "htmlhead";
	}

	if ($config->value($tmplname)) {
		if (open(TEMPL, $config->value($tmplname))) {
			local($/) = undef;
			$template = <TEMPL>;
			close(TEMPL);
		}
		else {
			warning("Template ".$config->value($tmplname)." does not exist.");
		}
	}

	#CSS checked _PH_
	print(expandtemplate($template,
						 ('title'		=> sub { titleexpand(@_, $who) },
						  'banner'		=> sub { bannerexpand(@_, $who) },
						  'baseurl'		=> sub { baseurl(@_) },
						  'stylesheet'  => sub { stylesheet(@_) },
						  'dotdoturl'	=> sub { dotdoturl(@_) },
						  'thisurl'		=> sub { thisurl(@_) },
						  'pathname'	=> sub { pathname(@_) },
						  'modes'		=> sub { modeexpand(@_, $who) },
						  'variables'	=> sub { varexpand(@_, $who) },
						  'devinfo'		=> sub { devinfo(@_) })));
}


sub makefooter {
	my $who = shift;
	my $tmplname;
	my $template = "<hr>\n</body>\n";

	$tmplname = $who."tail";
	
	unless ($who ne "sourcedir" || $config->sourcedirhead) {
		$tmplname = "sourcetail";
	}
	unless ($config->value($tmplname)) {
		$tmplname = "htmltail";
	}

	if ($config->value($tmplname)) {
		if (open(TEMPL, $config->value($tmplname))) {
			local($/) = undef;
			$template = <TEMPL>;
			close(TEMPL);
		}
		else {
			warning("Template ".$config->value($tmplname)." does not exist.");
		}
	}

	print(expandtemplate($template,
						 ('banner'		=> sub { bannerexpand(@_, $who) },
						  'thisurl'		=> sub { thisurl(@_) },
						  'modes'		=> sub { modeexpand(@_, $who) },
						  'variables'	=> sub { varexpand(@_, $who) },
						  'devinfo'		=> sub { devinfo(@_) })),
		  "</html>\n");
}


1;
