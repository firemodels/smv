#!/bin/bash
TABLE=SLCFlabels.tex
SMVFILE=../../Verification/Visualization/labels.smv

cat << EOF > $TABLE
\begin{table}[bph]
\begin{center}
\caption{\&SLCF quantities and associated labels used with the V2\_SLICE ini keyword and the SETSLICEBOUNDS ssf script command.}
\vspace{0.1in}
EOF
./smv2table.sh $SMVFILE SLCF >> $TABLE
cat << EOF >> $TABLE
\label{tabSLCF}
\end{center}
\end{table}
EOF

