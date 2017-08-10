#!/bin/bash

# Add LaTeX search path; Paths are ':' separated
export TEXINPUTS=".:../LaTeX_Style_Files:"

clean_build=1
GUIDE=SMV_Utilities_Guide

# Build SMV Utilities Guide

gitrevision=`git describe --long --dirty`
echo "\\newcommand{\\gitrevision}{$gitrevision}" > ../Bibliography/gitrevision.tex

pdflatex -interaction nonstopmode $GUIDE &> $GUIDE.err
bibtex $GUIDE &> $GUIDE.err
pdflatex -interaction nonstopmode $GUIDE &> $GUIDE.err
pdflatex -interaction nonstopmode $GUIDE &> $GUIDE.err

# Scan and report any errors in the LaTeX build process
if [[ `grep -E "Undefined control sequence|Error:|Fatal error|! LaTeX Error:|Paragraph ended before|Missing \\\$ inserted|Misplaced" -I $GUIDE.err | grep -v "xpdf supports version 1.5"` == "" ]]
   then
      # Continue along
      :
   else
      echo "LaTeX errors detected:"
      grep -A 1 -E "Undefined control sequence|Error:|Fatal error|! LaTeX Error:|Paragraph ended before|Missing \\\$ inserted|Misplaced" -I $GUIDE.err | grep -v "xpdf supports version 1.5"
      clean_build=0
fi

# Check for LaTeX warnings (undefined references or duplicate labels)
if [[ `grep -E "undefined|multiply defined|multiply-defined" -I $GUIDE.err` == "" ]]
   then
      # Continue along
      :
   else
      echo "LaTeX warnings detected:"
      grep -E "undefined|multiply defined|multiply-defined" -I $GUIDE.err
      clean_build=0
fi

if [[ $clean_build == 0 ]]
   then
      :
   else
      echo "SMV Utilities Guide built successfully!"
fi    
rm -f ../Bibliography/gitrevision.tex
