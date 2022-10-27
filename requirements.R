#
# Required packages for the mask_gaap_failures.R script 
#

install.packages("Rcpp",repos='https://cloud.r-project.org/')
install.packages("data.table",repos='https://cloud.r-project.org/')
install.packages("remotes",repos='https://cloud.r-project.org/')
remotes::install_github("ASGR/Rfits")
install.packages("KernSmooth",repos='https://cloud.r-project.org/')
install.packages("argparser",repos='https://cloud.r-project.org/')

