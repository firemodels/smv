#!/bin/bash
TABLE=BNDFtable.tex
SMVFILE=../../Verification/Visualization/labels.smv

cat << EOF > $TABLE
\begin{table}[bph]
\begin{center}
\caption{BNDF quantities and corresponding labels used in the ini file and smokeview scripts}
\vspace{0.1in}
EOF
./smv2table.sh $SMVFILE BNDF >> $TABLE
cat << EOF >> $TABLE
\label{tabBNDF}
\end{center}
\end{table}
EOF

