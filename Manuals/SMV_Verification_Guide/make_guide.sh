#!/bin/bash

# Add LaTeX search path; Paths are ':' separated
export TEXINPUTS=".:../LaTeX_Style_Files:"

clean_build=1

# Build SMV Verification Guide

gitrevision=`git describe --abbrev=7 --long --dirty`
echo "\\newcommand{\\gitrevision}{$gitrevision}" > ../Bibliography/gitrevision.tex

pdflatex -interaction nonstopmode SMV_Verification_Guide &> SMV_Verification_Guide.err
biber SMV_Verification_Guide                             &> SMV_Verification_Guide_biber.err
pdflatex -interaction nonstopmode SMV_Verification_Guide &> SMV_Verification_Guide.err
pdflatex -interaction nonstopmode SMV_Verification_Guide &> SMV_Verification_Guide.err
cat SMV_Verification_Guide_biber.err >> SMV_Verification_Guide.err

# Scan and report any errors in the LaTeX build process
if [[ `grep -E "Undefined control sequence|Error:|Fatal error|! LaTeX Error:|Paragraph ended before|Missing \\\$ inserted|Misplaced" -I SMV_Verification_Guide.err | grep -v "xpdf supports version 1.5"` == "" ]]
   then
      # Continue along
      :
   else
      echo "LaTeX errors detected:"
      grep -A 1 -E "Undefined control sequence|Error:|Fatal error|! LaTeX Error:|Paragraph ended before|Missing \\\$ inserted|Misplaced" -I SMV_Verification_Guide.err | grep -v "xpdf supports version 1.5"
      clean_build=0
fi

# Check for LaTeX warnings (undefined references or duplicate labels)
if [[ `grep -E "undefined|ERROR|WARNING|multiply defined|multiply-defined" -I SMV_Verification_Guide.err` == "" ]]
   then
      # Continue along
      :
   else
      echo "LaTeX warnings detected:"
      grep -E "undefined|ERROR|WARNING|multiply defined|multiply-defined" -I SMV_Verification_Guide.err
      clean_build=0
fi

if [[ $clean_build == 0 ]]
   then
      :
   else
      if [ -e SMV_Verification_Guide.pdf ]; then
         echo "SMV Verification Guide built successfully!"
      else
         echo "***Error: SMV_Verification_Guide.pdf does not exist"
      fi
fi    

rm -f ../Bibliography/gitrevision.tex
