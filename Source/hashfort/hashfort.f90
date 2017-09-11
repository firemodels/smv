module sha1Module
    use, intrinsic :: iso_fortran_env
    implicit none

    public :: SHA1prog

   contains

    function SHA1prog(LU) result(hash)
      integer, intent(in) :: LU
      character :: hash*40, prog_path*1024
      character(1), allocatable, dimension(:) :: prog_contents
      integer :: len, err
      integer(int8), dimension(:), allocatable :: bytes
      integer(int64) :: prog_size
      
      hash = "unknown"
      
      call get_command_argument (0, prog_path, len, err)
      if(err/=0)return

      inquire(file=prog_path,size=prog_size,iostat=err)
      if(err/=0)return
      
      allocate(prog_contents(prog_size),stat=err)
      if(err/=0)return
      
      open(LU,form='binary',file=prog_path,action='read',iostat=err)
      if(err/=0)then
         deallocate(prog_contents)
         return
      endif
      
      read(LU,iostat=err)prog_contents
      if(err/=0)then
         deallocate(prog_contents)
         return
      endif
      close(LU)

      allocate(bytes(((prog_size+8)/64 + 1)*64),stat=err)
      if(err/=0)then
         deallocate(prog_contents)
         return
      endif
      
      bytes(:prog_size) = transfer(prog_contents, bytes(:prog_size))
      hash = SHA1Hash(bytes, prog_size)
      
      deallocate(bytes)
      deallocate(prog_contents)
    end function SHA1prog

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
                ! is the source size less than the result size?
                w(j) = transfer(bytes((i-1)*64 + j*4:(i-1)*64 + (j-1)*4 + 1:-1), w(j))
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
