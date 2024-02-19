From: quale@saavik.cs.wisc.edu (Douglas E. Quale)
Subject: better graphics for Nethack
Date: 13 Apr 92 14:32:16 GMT

If there are any Nethackers out there, you can use the nicer DECgraphics
display if you make a small addition to your termcap.  I had hesitated
posting this before because earlier releases didn't quite get all the
vt100 graphics characters right, but 0.95c+ has changed some of console.c
and everything seems ok now.  Great work.

Use ``export NETHACKOPTIONS=DECgraphics'' and make the following
addition to your termcap entry to add the alternate character set start
and end strings.  I also added the km (has meta key) flag so we can end
all the ``Why doesn't the Alt key work in emacs?'' questions.  These
additions probably belong in the root disk distribution of /etc/termcap.

        km:as=^N\E(0:ae=\E(B:

--
Doug Quale
quale@saavik.cs.wisc.edu
