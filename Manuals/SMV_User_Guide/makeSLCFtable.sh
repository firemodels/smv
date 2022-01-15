#!/bin/bash
TABLE=SLCFtable.tex
SMVFILE=../../Verification/Visualization/labels.smv

cat << EOF > $TABLE
\begin{table}[bph]
\begin{center}
\caption{SLCF quantities and corresponding labels used in the ini file and smokeview scripts}
\vspace{0.1in}
EOF
./smv2table.sh $SMVFILE SLCF >> $TABLE
cat << EOF >> $TABLE
\label{tabSLCF}
\end{center}
\end{table}
EOF

