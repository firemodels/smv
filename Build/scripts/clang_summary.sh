#!/bin/bash
OUTFILE=$1
if [ "$OUTFILE" == "" ]; then
  OUTFILE=clang_warnings.txt
fi
HAVE_WARNINGS=
for file in *.chk; do
  if [ -s $file ]; then
    HAVE_WARNINGS=1
    break
  fi
done
echo clang warnings > $OUTFILE
echo -------------- >> $OUTFILE
if [ "$HAVE_WARNINGS" == "" ]; then
  echo no warnings were found > $OUTFILE
fi
total_warnings=0
nfiles=0
nwarnfiles=0
echo ""
for file in *.chk; do
  nfiles=$((nfiles+1))
  if [ -s $file ]; then
    nwarnings=$(awk '/^[0-9]+ warnings? generated\.$/ { count = $1 } END { print count + 0 }' "$file")
    if [ "$nwarnings" -gt 0 ]; then
      total_warnings=$((total_warnings+nwarnings))
      nwarnfiles=$((nwarnfiles+1))
      {
        if [ "$nwarnfiles" -gt 1 ]; then
          printf '%s\n' '--------------------'
        fi
        printf '%s\n' "${file%.chk}"
        cat "$file"
        printf '\n'
      } >> "$OUTFILE"
      echo "$file: $nwarnings warnings"
    fi
  fi
done
echo
if [ "$total_warnings" == "0" ]; then
  echo "$nfiles files scanned, no warnings were found" | tee -a "$OUTFILE"
else
  echo "$nfiles files scanned, $nwarnfiles files have $total_warnings warnings" | tee -a "$OUTFILE"
fi

