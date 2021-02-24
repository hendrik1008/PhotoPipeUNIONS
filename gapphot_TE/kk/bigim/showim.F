	parameter (maxpix=1024)
	real zz(maxpix,maxpix),tr(6)
	character*80 toplbl

	data tr /0.,1.,0.,0.,0.,1./


	read(*,'(a)') toplbl

	read(*,*) nx,ny
	if (nx.gt.maxpix .or. ny.gt.maxpix) then
	   write(0,*) 'dimensions too large'
	   stop
	endif

	zmin=1e32
	zmax=-zmin
 1	read(*,*,err=1,end=2) ix,iy,zz(ix,iy)
	zmin=min(zmin,zz(ix,iy))
	zmax=max(zmax,zz(ix,iy))
	goto 1
 2	continue
	
	write(0,*) 'Min,max array values found ',zmin,zmax
	call pgbeg(0,'/xs',1,1)
	call pgenv(0.,nx+1., 0.,ny+1.,1,0)
	call pggray(zz,maxpix,maxpix,1,nx,1,ny,zmin,zmax,tr)
	call pgwedg('R',3.,4.,zmin,zmax,'.')
	call pglabel('X','Y',toplbl)
	call pgsci(2)
	c=0.
	call pgcont(zz,maxpix,maxpix,1,nx,1,ny,c,1,tr)
	call pgend
c PS file copy
	call pgbeg(0,'showim.ps/ps',1,1)
	call pgenv(0.,nx+1., 0.,ny+1.,1,0)
	call pggray(zz,maxpix,maxpix,1,nx,1,ny,zmin,zmax,tr)
	call pgwedg('R',3.,4.,zmin,zmax,'.')
	call pglabel('X','Y',toplbl)
	call pgsci(2)
	c=0.
	call pgcont(zz,maxpix,maxpix,1,nx,1,ny,c,1,tr)
	call pgend


	end
