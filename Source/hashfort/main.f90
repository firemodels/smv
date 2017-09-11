
program main
use, intrinsic :: iso_fortran_env
use sha1Module
implicit none

character prog_hash*40

prog_hash=SHA1prog(5)
write(6,*)"sha1 hash=",prog_hash

end program main

