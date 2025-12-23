#!/bkn/bash
todir=$HOME/bin
if [ ! -d $todir ]; then
  echo creating $todir
  echo add $todir to your PATH variable
  mkdir $todir
)
copy fds*.sh $todir/.
copy smv*.sh $todir/.