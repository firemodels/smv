#!/bin/bash
KEYWORDTYPE=$1
if [[ "$KEYWORDTYPE" != "ini" ]] && [[ "$KEYWORDTYPE" != "smv" ]]; then
  KEYWORDTYPE="ssf"
fi

if [ ! -e .smv_git ]; then
  echo "***error: the script $0 needs to run in the bot/Firebot directory"
  echo "          $0 aborted"
  exit 1
fi

OUTPUT=output
NAME_PREFIX=${KEYWORDTYPE}_

KEYWORDS_C=$OUTPUT/${NAME_PREFIX}keywords_c.txt
KEYWORDS_TEX=$OUTPUT/${NAME_PREFIX}keywords_tex.txt
KEYWORDS_DIFF=$OUTPUT/${NAME_PREFIX}keywords_diff.txt
KEYWORDS_NODOC=$OUTPUT/${NAME_PREFIX}keywords_nodoc.txt
KEYWORDS_NOSOURCE=$OUTPUT/${NAME_PREFIX}keywords_nosource.txt

#remove files from last comparison
rm -f $OUTPUT/${KEYWORDTYPE}*keywords*txt

# SMV UG directory
tex_dir=../../smv/Manuals/SMV_User_Guide/
# smv source directory
source_dir=../../smv/Source/smokeview
# shared source directory
shared_dir=../../smv/Source/shared

if [ "$KEYWORDTYPE" == "ssf" ]; then
  # generate list of script keywords found in IOscript.c
  grep InitKeyword $source_dir/IOscript.c | awk -F'"' '{print $2}' | awk -F'"' '{print $1}' |  grep -v '^[[:space:]]*$' | sort -u > $KEYWORDS_C

  # generate list of script keywords found in SMV_User_Guide.tex
  grep hitemssf $tex_dir/*.tex | awk -F'{' '{print $2}' | awk -F '}' '{print $1}' | sort -u > $KEYWORDS_TEX
fi
if [ "$KEYWORDTYPE" == "smv" ]; then
  # generate list of script keywords found in IOscript.c
  grep MatchSMV $shared_dir/readsmvfile.c | awk -F'"' '{print $2}' | awk -F'"' '{print $1}' |  grep -v '^[[:space:]]*$' | sort -u > $KEYWORDS_C

  # generate list of script keywords found in SMV_User_Guide.tex
  grep hitemsmv $tex_dir/*.tex | awk -F'{' '{print $2}' | awk -F '}' '{print $1}' | sort -u > $KEYWORDS_TEX
fi
if [ "$KEYWORDTYPE" == "ini" ]; then
  # generate list of script keywords found in IOscript.c
  grep MatchINI $source_dir/readsmv.c | awk -F'"' '{print $2}' | awk -F'"' '{print $1}' |  grep -v '^[[:space:]]*$' | sort -u > $KEYWORDS_C

  # generate list of script keywords found in SMV_User_Guide.tex
  grep hitemini $tex_dir/*.tex | awk -F'{' '{print $2}' | awk -F '}' '{print $1}' | sed 's/\\_/_/g' | sort -u > $KEYWORDS_TEX
fi

#compute difference between tex and c /keywords
git diff --no-index $KEYWORDS_C $KEYWORDS_TEX                                  > $KEYWORDS_DIFF

nlines_nodoc=`grep ^- $KEYWORDS_DIFF | sed 's/^-//g' | grep -v dummy | grep -v -- '--' | wc -l`
echo "undocumented script keywords: $nlines_nodoc"                              > $KEYWORDS_NODOC
grep ^- $KEYWORDS_DIFF | sed 's/^-//g' | grep -v dummy | grep -v -- '--'       >> $KEYWORDS_NODOC

nlines_nosource=`grep ^+ $KEYWORDS_DIFF | sed 's/^+//g' | grep -v \\+\\+ | wc -l`
echo "unimplemented script keywords: $nlines_nosource"                         > $KEYWORDS_NOSOURCE
grep ^+ $KEYWORDS_DIFF | sed 's/^+//g' | grep -v \\+\\+                       >> $KEYWORDS_NOSOURCE

if [ "$nlines_nodoc" == "0" ]; then
  echo "$nlines_nodoc undocumented $KEYWORDTYPE script keywords"
else
  echo
  echo "$nlines_nodoc undocumented $KEYWORDTYPE script keywords: $KEYWORDS_NODOC"
  cat $KEYWORDS_NODOC
fi
if [ "$nlines_nosource" == "0" ]; then
  echo "$nlines_nosource unimplemented $KEYWORDTYPE script keywords"
else
  echo
  echo "$nlines_nosource unimplemented $KEYWORDTYPE script keywords: $KEYWORDS_NOSOURCE"
  cat $KEYWORDS_NOSOURCE
fi

