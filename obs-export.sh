#!/bin/bash

# This needs a bit more work, mostly on the "discplined engineering" front.
# IOW, instead of this UPSTREAM_BASE hack it would be better to have 3
# branches:
#   1) pristine upstream: for tracking upstream progress/retrogression
#   2) patched upstream: pristine upstream with outr patches applied
#   3) working local: patches upstream + a set of scripts (like this) to
#      do everyday stuff like making new releases, exporting stuff to
#      OBS, etc...


PKG="$(basename `pwd`)"
UPSTREAM_BASE="upstream"
VERSION="`date +'%Y%m%d'`"
HEAD="HEAD"
MODE=gerrit
RELEASE=no
AUTHOR="Policy Team <policy.team@intel.com>"

while [ "${1#-}" != "$1" -a -n "$1" ]; do
    case $1 in
        --name|-n)
            PKG="$2"
            shift 2
            ;;
        --version|-v)
            VERSION="$2"
            shift 2
	    ;;
        --base|-B|-b)
            UPSTREAM_BASE="$2"
            shift 2
            ;;
        --head|-H)
            HEAD="$2"
            shift 2
          ;;
        --obs|-o)
            MODE="obs"
            shift 1
          ;;
        --help|-h)
           echo "usage: $0 [options], where the possible options are"
           echo "  -n <name>           name of your package"
           echo "  -v <version>        version to in rpm/SPEC file"
           echo "  -B <upstream-base>  name or SHA1 of baseline"
           echo "  -H <taget-head>     name or SHA1 of release HEAD"
           echo "  --obs               include tarball for OBS"
           echo ""
           echo "<name> is the name of the package, <version> is the version"
           echo "you want to export to OBS, and <upstream-base> is the name of"
           echo "the upstream git branch or the SHA1 you want to generate your"
           echo "release from and base your patches on top of. On OBS mode the"
           echo "output will be generated in a directory called obs-$VERSION."
           echo "Otherwise in gerrit mode, the output will be generated in a"
           echo "directory called packaging."
	   echo ""
           echo "E.g.:"
           echo "  $0 -n pulseaudio -v 2.0 -B pulseaudio-2.0 -H tizen"
           echo ""
           echo "This will produce a gerrit export with version 2.0 against the"
           echo "SHA1 pulseaudio-2.0, producing patches up till tizen and"
           echo "place the result in a directory called packaging."
           exit 0
           ;;
        --debug|-d)
           set -x
           ;;
        --big-hammer|--release|-r)
           RELEASE=yes
           shift 1
           ;;
        --author|-a)
           AUTHOR="$2"
           shift 2
           ;;
        *) echo "usage: $0 [-n <name>][-v <version>][--obs]"
           echo "          [-b <upstream-base>] [-H <head>"
           exit 1
           ;;
    esac
done

case $MODE in
    gerrit)
        TARBALL=""
        DIR=packaging
        ;;
    obs)
        TARBALL=$PKG-$VERSION.tar
        DIR="obs-$VERSION"
        ;;
    *)
        echo "invalid mode: $MODE"
        exit 1
        ;;
esac

echo "Package name: $PKG"
echo "Package version: $VERSION"
echo "Package baseline: $UPSTREAM_BASE"
echo "Package head: $HEAD"
echo "Output directory: $DIR"

rm -fr $DIR
mkdir $DIR

if [ -n "$TARBALL" ]; then
    echo "Generating tarball..."
        git archive --format=tar --prefix=$PKG-$VERSION/ $UPSTREAM_BASE \
            > $DIR/$TARBALL && \
        gzip $DIR/$TARBALL
fi

echo "Generating patches, creating spec file..."
cd $DIR && \
    git format-patch -n $UPSTREAM_BASE..$HEAD && \
    cat ../$PKG.spec.in | sed "s/@VERSION@/$VERSION/g" > $PKG.spec.in && \
cd - >& /dev/null

cd $DIR
patchlist="`ls *.patch`"
cat $PKG.spec.in | while read -r line; do
    case $line in
        @DECLARE_PATCHES@)
            i=0
            for patch in $patchlist; do
                echo "Patch$i: $patch"
                let i=$i+1
            done
            ;;
        @APPLY_PATCHES@)
            i=0
            for patch in $patchlist; do
                echo "%patch$i -p1"
                let i=$i+1
            done
            ;;
        *)
            echo "$line"
            ;;
    esac
done > $PKG.spec
cd - >& /dev/null

rm -f $DIR/$PKG.spec.in

if [ "$MODE" = "gerrit" -a "$RELEASE" = "yes" ]; then
    stamp="$(date -u +%Y%m%d.%H%M%S)"
    branch="gerrit-release-$stamp"
    tag="submit/trunk/$stamp"
    chlog=packaging/$PKG.changes

    echo "Preparing release branch $branch with tag $tag..."

    git branch $branch $UPSTREAM_BASE && \
        git checkout $branch && \
        git add packaging && \
        git commit -m "release: added packaging for gerrit." packaging && \
        echo "* $(date '+%a %b %d %H:%M:%S %Z %Y') $AUTHOR - $VERSION" \
            > $chlog && \
        echo "- release: releasing $VERSION..." >> $chlog && \
            git add $chlog &&
        echo "" && \
        echo "Okay, branch $branch is prepared for release." && \
        echo "To proceed with the release, please" && \
        echo "" && \
        echo "  1) vi $chlog (and add a real changelog entry)" && \
        echo "  2) git commit -m \"release: updated changelog.\" $chlog" && \
        echo "  3) git tag -a -m \"release: tagged release.\" $tag HEAD" && \
        echo "  4) git push --force tzgerrit HEAD^:refs/heads/master" && \
        echo "  5) git push tzgerrit HEAD:refs/for/master $tag"

    if [ "$?" = "0" ]; then
        echo "Done."
    else
        echo "Failed to prepare release..."
        git branch -D $branch
        exit 1
    fi
fi
