module sum_module
    use iso_c_binding
    implicit none
  contains
    ! Fortran function to add two arrays with C binding
    subroutine famul(a, b, c, len) bind(C)
      implicit none
      ! Input arrays and size
      integer(c_int), intent(in) :: len
      real(c_float), intent(in) :: a(len), b(len)
      ! Output array
      real(c_float), intent(out) :: c(len)
      ! Element-wise addition
      integer(c_int) :: i
      do i = 1, len
       c(i) = a(i) + b(i)
      end do
    end subroutine famul
  end module sum_module
  