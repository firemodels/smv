! Notes:
! No, fortran does not support unsigned integers.

module sha1Module
    use, intrinsic :: iso_fortran_env
    implicit none
    private

    interface SHA1
        module procedure SHA1Scalar, SHA1Array
    end interface

    public :: SHA1

    contains
    function SHA1Scalar(value) result(hash)
        class(*), intent(in) :: value
        character(len=40) :: hash

        integer(int8), dimension(:), allocatable :: bytes
        integer(int64) :: length

        select type (value)
            type is (character(len=*))
                length = len(value)
                allocate(bytes(((length+8)/64 + 1)*64))
                bytes(:length) = transfer(value, bytes(:length))
            type is (integer(int8))
                length = 1
                allocate(bytes(64))
                bytes(1) = value
            type is (integer(int16))
                length = 2
                allocate(bytes(64))
                bytes(1:length) = transfer(value, bytes(1:length))
            type is (integer(int32))
                length = 4
                allocate(bytes(64))
                bytes(1:length) = transfer(value, bytes(1:length))
            type is (integer(int64))
                length = 8
                allocate(bytes(64))
                bytes(1:length) = transfer(value, bytes(1:length))
            type is (real(real32))
                length = 4
                allocate(bytes(64))
                bytes(1:length) = transfer(value, bytes(1:length))
            type is (real(real64))
                length = 8
                allocate(bytes(64))
                bytes(1:length) = transfer(value, bytes(1:length))
            type is (real(real128))
                length = 16
                allocate(bytes(64))
                bytes(1:length) = transfer(value, bytes(1:length))
            class default
                print *, "Error: Unsupported type in SHA1."
                stop
        end select

        hash = SHA1Hash(bytes, length)
        deallocate(bytes)
    end function SHA1Scalar

    function SHA1Array(value) result(hash)
        class(*), dimension(:), intent(in) :: value
        character(len=40) :: hash

        integer(int8), dimension(:), allocatable :: bytes
        integer(int64) :: length
        integer :: i, width

        select type (value)
            type is (character(len=*))
                length = size(value) * len(value(1))
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*len(value(i)) + 1:i*len(value(i))) = &
                    transfer(value(i), bytes((i - 1)*len(value(i)) + 1:i*len(value(i))))
                end do
            type is (integer(int8))
                length = size(value)
                allocate(bytes(((length + 8)/64 + 1)*64))
                bytes(1:length) = value
            type is (integer(int16))
                width = 2
                length = size(value) * width
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*width + 1:i*width) = transfer(value(i), bytes((i - 1)*width + 1:i*width))
                end do
            type is (integer(int32))
                width = 4
                length = size(value) * width
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*width + 1:i*width) = transfer(value(i), bytes((i - 1)*width + 1:i*width))
                end do
            type is (integer(int64))
                width = 8
                length = size(value) * width
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*width + 1:i*width) = transfer(value(i), bytes((i - 1)*width + 1:i*width))
                end do
            type is (real(real32))
                width = 4
                length = size(value) * width
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*width + 1:i*width) = transfer(value(i), bytes((i - 1)*width + 1:i*width))
                end do
            type is (real(real64))
                width = 8
                length = size(value) * width
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*width + 1:i*width) = transfer(value(i), bytes((i - 1)*width + 1:i*width))
                end do
            type is (real(real128))
                width = 16
                length = size(value) * width
                allocate(bytes(((length + 8)/64 + 1)*64))
                do i = 1,size(value)
                    bytes((i - 1)*width + 1:i*width) = transfer(value(i), bytes((i - 1)*width + 1:i*width))
                end do
            class default
                print *, "Error: Unsupported type in SHA1."
                stop
        end select

        hash = SHA1Hash(bytes, length)
        deallocate(bytes)
    end function SHA1Array

    function SHA1Hash(bytes, length)
        integer(int8), dimension(:) :: bytes
        integer(int64), intent(in) :: length
        character(len=40) :: SHA1Hash

        integer(int32) :: h0, h1, h2, h3, h4, a, b, c, d, e, f, k, temp
        integer(int32), dimension(80) :: w
        integer :: i, j

        bytes(length + 1) = ibset(0_int8, 7)
        bytes(length + 2:) = 0_int8
        bytes(size(bytes) - 7:) = transfer(length*8_int64, bytes(size(bytes) - 7:))
        bytes(size(bytes) - 7:) = bytes(size(bytes):size(bytes) - 7:-1)

        h0 = 1732584193_int32 ! z'67452301'
        h1 = -271733879_int32 ! z'EFCDAB89'
        h2 = -1732584194_int32 ! z'98BADCFE'
        h3 = 271733878_int32 ! z'10325476'
        h4 = -1009589776_int32 ! z'C3D2E1F0'

        do i = 1,size(bytes)/64
            do j = 1,16 ! take 512 bit chunk of string
                w(j) = transfer(bytes((i-1)*64 + j*4:(i-1)*64 + (j-1)*4 + 1:-1), w(j)) ! is the source size less than the result size?
            end do
            do j = 17,80 ! Extend the sixteen 32-bit words into eighty 32-bit words
                w(j) = ishftc(ieor(ieor(ieor(w(j-3), w(j-8)), w(j-14)), w(j-16)), 1)
            end do

            a = h0; b = h1; c = h2; d = h3; e = h4
            do j = 1,80
                select case (j)
                    case (1:20)
                        f = ior(iand(b, c), iand(not(b), d))
                        k = 1518500249_int32 ! k = z'5A827999'
                    case (21:40)
                        f = ieor(ieor(b, c), d)
                        k = 1859775393_int32 ! k = z'6ED9EBA1'
                    case (41:60)
                        f = ior(ior(iand(b, c), iand(b, d)), iand(c, d))
                        k = -1894007588_int32 ! k = z'8F1BBCDC'
                    case (61:80)
                        f = ieor(ieor(b, c), d)
                        k = -899497514_int32 ! k = z'CA62C1D6'
                end select

                temp = ishftc(a, 5) + f + e + w(j) + k
                e = d
                d = c
                c = ishftc(b, 30)
                b = a
                a = temp
            end do

            h0 = h0 + a
            h1 = h1 + b
            h2 = h2 + c
            h3 = h3 + d
            h4 = h4 + e
        end do

        write(SHA1Hash(1:8), "(Z8.8)") h0
        write(SHA1Hash(9:16), "(Z8.8)") h1
        write(SHA1Hash(17:24), "(Z8.8)") h2
        write(SHA1Hash(25:32), "(Z8.8)") h3
        write(SHA1Hash(33:40), "(Z8.8)") h4
    end function SHA1Hash
end module sha1Module

program echo_command_line
    use, intrinsic :: iso_fortran_env
    use sha1Module
    implicit none

    integer i, cnt, len, status, file_size
character c*256, b*100
character(1), allocatable, dimension(:) :: prog_file

    print *, "message"
    print *, SHA1("message")

    print *, ""
    print *, SHA1("")

    print *, 0_int64
    print *, SHA1(0_int64)

    print *, 700_int64
    print *, SHA1(700_int64)

call get_command (b, len, status)
if (status .ne. 0) then
   write (*,*) 'get_command failed with status = ', status
   stop
end if
write (*,*) 'command line = ', b (1:len)


call get_command_argument (0, c, len, status)
if (status .ne. 0) then
   write (*,*) 'Getting command name failed with status = ', status
   stop
end if

write(6,*)"command=",c
INQUIRE(FILE=c,SIZE=file_size)
allocate(prog_file(file_size))
write (*,*) 'file size=',file_size
open(5,form='binary',file=c)
read(5)prog_file
    print *, SHA1(prog_file)
    write(6,*)"complete"
end

