#!/bin/bash
TABLE=BNDFlabels.tex
SMVFILE=../../Verification/Visualization/labels.smv

cat << EOF > $TABLE
\begin{table}[bph]
\begin{center}
\caption{\&BNDF quantities and associated labels used with the V2\_BOUNDARY ini keyword and the SETBOUNDBOUNDS ssf script command.}
\vspace{0.1in}
EOF
./smv2table.sh $SMVFILE BNDF >> $TABLE
cat << EOF >> $TABLE
\label{tabBNDF}
\end{center}
\end{table}
EOF

