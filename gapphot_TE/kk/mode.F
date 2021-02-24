      program mode
c     calculate mode of series of numbers (input is one number per line)
c     estimate this as median of interval of sqrt(N) values with smallest span

      parameter (m=1000000)
      real x(m)
      do i=1,m
 1       read(*,*,err=1,end=2) x(i)
      enddo
 2    n=i-1
      if (n.eq.1) then
         write(*,*) x(i)
      else
         call sort(n,x)
c     width of window
         k=int(sqrt(1.*n))*2
         w=x(n)-x(1)
         do i=1,n-k
            d=x(i+k)-x(i)
            if (d.lt.w) then
               w=d
               imode=i
            endif
         enddo
         write(*,*) x(imode+k/2)
      endif
      end

      
