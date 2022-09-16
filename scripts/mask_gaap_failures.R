#break
#
# Script for masking VIKING sum-images with GAAP failures 
#
#

catcols<-c("Xpos","Ypos",
         paste0("MAG_GAAP_", c("Z","Y","J","H","Ks")),
         paste0("FLAG_GAAP_",c("Z","Y","J","H","Ks")))

p <- argparser::arg_parser("Additional Masking")

p<-argparser::add_argument(p,'--pointing',help='Pointing Name')
p<-argparser::add_argument(p,'--maincat',help='Catalogue File')
p<-argparser::add_argument(p,'--Zmask',help='Z-band NIR Mask')
p<-argparser::add_argument(p,'--Ymask',help='Y-band NIR Mask')
p<-argparser::add_argument(p,'--Jmask',help='J-band NIR Mask')
p<-argparser::add_argument(p,'--Hmask',help='H-band NIR Mask')
p<-argparser::add_argument(p,'--Ksmask',help='Ks-band NIR Mask')
p<-argparser::add_argument(p,'--output_end',help='what to append to mask filename at output')
p<-argparser::add_argument(p,'--bw',help='Bandwidth for KDE (arcmin)',default=1/sqrt(12))
p<-argparser::add_argument(p,'--threshold.frac',help='threshold for determination of "bad" areas',default=0.8)

argv<-argparser::parse_args(p,commandArgs(TRUE))

#Read the catalogue {{{
cat<-Rfits::Rfits_read_table(argv$maincat,cols=catcols)
#}}}

#Remove bad magnitudes {{{
cat[MAG_GAAP_Z <= 0 | MAG_GAAP_Z >= 90,MAG_GAAP_Z := NA]
cat[MAG_GAAP_Y <= 0 | MAG_GAAP_Y >= 90,MAG_GAAP_Y := NA]
cat[MAG_GAAP_J <= 0 | MAG_GAAP_J >= 90,MAG_GAAP_J := NA]
cat[MAG_GAAP_H <= 0 | MAG_GAAP_H >= 90,MAG_GAAP_H := NA]
cat[MAG_GAAP_Ks<= 0 | MAG_GAAP_Ks>= 90,MAG_GAAP_Ks:= NA]
#}}}

#Index of unmasked Sources 
ind<-1:nrow(cat)

if (length(ind)>0) { 

  for (filter in c('Z','Y','J','H','Ks')) { 
    
    #Are we modifying this mask?
    if (is.na(argv[[paste0(filter,"mask")]])) { next }

    #Mask filename
    maskname<-argv[[paste0(filter,"mask")]]

    #Read the mask image {{{
    mask<-Rfits::Rfits_read_image(maskname)
    #}}}
  
    #Index of unmasked sources with good GAAP measurements 
    ind2<-which(cat[[paste0("FLAG_GAAP_",filter)]]==0)

    #Outer range function 
    outer_range<-function(X) return=c(floor(min(X)),ceiling(max(X)))

    #Number of image pixels per desired bandwidth
    pixperbw<-abs(argv$bw/60/c(mask$keyvalues$CD1_1,mask$keyvalues$CD2_2))

    #Grid should Nyquist sample the bandwidth 
    pixpergrid<-round(pixperbw/3)

    #Ranges of the image in X&Y
    xrange<-c(0,mask$keyvalues$NAXIS1)
    yrange<-c(0,mask$keyvalues$NAXIS2)

    #Number of steps in the KDE grid in X&Y
    xsteps<-ceiling(diff(xrange)/pixpergrid[1])
    ysteps<-ceiling(diff(yrange)/pixpergrid[2])
    
    #Compute the KDE for all unmaksed sources
    all.hist<-KernSmooth::bkde2D(cbind(cat$Xpos[ind ],cat$Ypos[ind ]),bandwidth=pixperbw,
                                 range=list(xrange,yrange),gridsize=c(xsteps,ysteps)) 
    #Compute the KDE for all good unmaksed sources
    sel.hist<-KernSmooth::bkde2D(cbind(cat$Xpos[ind2],cat$Ypos[ind2]),bandwidth=pixperbw,
                                 range=list(xrange,yrange),gridsize=c(xsteps,ysteps)) 
    #Reformat the matrix 
    all.hist<-list(bincen=list(x=all.hist$x1,y=all.hist$x2),map=all.hist$fhat*length(ind)/sum(all.hist$fhat))
    sel.hist<-list(bincen=list(x=sel.hist$x1,y=sel.hist$x2),map=sel.hist$fhat*length(ind2)/sum(sel.hist$fhat))
    #Remove small probabilities
    all.hist$map[which(all.hist$map<1e-4)]<-0
    sel.hist$map[which(sel.hist$map<1e-4)]<-0
    #Calculate the map ratio: p(good) / p(all)
    rat.hist<-sel.hist$map/all.hist$map
    #Set na pixels to "good" 
    rat.hist[which(is.na(rat.hist))]<-1

    #Identify all pixels with significant losses: 
    new_mask<-array(1,dim=dim(rat.hist))
    new_mask[which(rat.hist<argv$threshold.frac)]<-0 

    #Convert new_mask pixels to mask pixels 
    full.mask<-array(0,dim=dim(mask$imDat))
    mask_pix<-expand.grid(1:nrow(mask$imDat),1:ncol(mask$imDat))
    map_pix<-ceiling(t(t(expand.grid(1:nrow(mask$imDat),1:ncol(mask$imDat)))/pixpergrid))
    
    full.mask[cbind(mask_pix[,1],mask_pix[,2])]<-new_mask[cbind(map_pix[,1],map_pix[,2])]

    #Incorporate the new mask 
    raw.mask<-mask$imDat
    mask$imDat<-mask$imDat*full.mask

    #make output name 
    output_file<-sub('.fits',paste0('.',argv$output_end,'.fits'),maskname)
    output_plot<-sub('.fits',paste0('.',argv$output_end,'.png'),maskname)

    #Output the diagnostic plot 
    png(file=output_plot,width=19.749508*120,height=7.094701*120,res=120)
    layout(cbind(1,2,3))
    #Plot the original sum image
    magicaxis::magimage(1:nrow(mask$imDat),1:ncol(mask$imDat),raw.mask,stretch='lin',hi=1,lo=0,main='Original Sum Image')
    #Plot the data distribution 
    magicaxis::magimage(seq(xrange[1],xrange[2],by=pixpergrid[1]),seq(yrange[1],yrange[2],by=pixpergrid[2]),all.hist$map,stretch='lin',hi=0.95,lo=0,main='Data Distribution')
    points(cat$Xpos[ind],cat$Ypos[ind],pch='.',col=ifelse(cat$FLAG_GAAP_H[ind]==0,hsv(0.5,a=0.1),hsv(0.05,a=0.1)))
    #Plot the new sum image 
    magicaxis::magimage(1:nrow(mask$imDat),1:ncol(mask$imDat),mask$imDat,stretch='lin',hi=1,lo=0,main='New Sum Image')
    points(cat$Xpos[ind],cat$Ypos[ind],pch='.',col=ifelse(cat$FLAG_GAAP_H[ind]==0,hsv(0.5,a=0.1),hsv(0.05,a=0.1)))
    #Close the plot device
    dev.off()

    #Output the new mask 
    Rfits::Rfits_write_image(mask,file=output_file) 
  }
}

