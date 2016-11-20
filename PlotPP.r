# First setup of the model
#remove(list=ls()) # Remove all variables from memory

on=1;off=0;
setwd("/Simulations/OpenCL/clPredPrey/clPredPrey")

require(fields)

FID = file("Output.dat", "rb")

NX = readBin(FID, integer(), n = 1, endian = "little");
NY = readBin(FID, integer(), n = 1, endian = "little");
Length=readBin(FID,  numeric(), size=4, n = 1, endian = "little")
NumFrames = readBin(FID, integer(), n = 1, endian = "little");
EndTime=readBin(FID,  integer(), n = 1, endian = "little")

Movie=off
Wait=off
AllWindows=on
DPI=144

Width = 1000*0.5

PGraphMax = 1  # The maximal value of Prey to be plotted. If one goes over that, the value is capped

if (AllWindows==on){
  WinWidth = 1280
  WinHeight = 720
}else {
  WinWidth = 960
  WinHeight = 720
}

Prey_in_Time = Pred_in_Time = 1:NumFrames

water.palette = colorRampPalette(c("white", "blue"))
ColorPalette = colorRampPalette(c("#00007F", "blue", "#007FFF", "cyan",
                                  "#7FFF7F", "yellow", "#FF7F00", "red", "#7F0000"))

if (Movie==off) 
  quartz(width=WinWidth/DPI, height=WinHeight/DPI, dpi=DPI)

for (jj in 0:(NumFrames-1)){  # Here the time loop starts 
  
   if (Movie==on)
      tiff(filename = sprintf("Images/Rplot%03d.tiff",jj),
           width = WinWidth, height = WinHeight, 
           units = "px", pointsize = 24,
           compression="none",
           bg = "white", res = NA,
           type = "quartz")  
   
   Data_Prey = matrix(nrow=NY, ncol=NX, readBin(FID, numeric(), size=4, n = NX*NY, endian = "little"));
   Data_Pred = matrix(nrow=NY, ncol=NX, readBin(FID, numeric(), size=4, n = NX*NY, endian = "little"));
   
   Prey_in_Time[jj+1] = mean(Data_Prey)
   Pred_in_Time[jj+1] = mean(Data_Pred)
   
   if (AllWindows==on){
     par(mar=c(1, 1, 3, 1), mfrow=c(1,2))
   } else {
     par(mar=c(1, 4, 1, 6))
   }
   
   image.plot(pmin(Data_Prey,PGraphMax), zlim=c(0,PGraphMax), xaxt="n", yaxt="n", horizontal = T,
              col = ColorPalette(255),asp=1, bty="n", useRaster=TRUE,
              legend.shrink = 0.85, legend.width = 0.5)  
   
   title("Prey", line=1, cex.main=1)   
   
   if (AllWindows==on){
     
     par(mar=c(1, 1, 3, 1))  
     
     image.plot(Data_Pred, zlim=c(0,1), xaxt='n', yaxt="n", horizontal = T,
                col = ColorPalette(255),asp=1, bty="n", useRaster=TRUE,
                legend.shrink = 0.85, legend.width = 0.5)  
     
     title("Predator", line=1, cex.main=1)  
     
     par(mar=c(1, 4, 1, 6), mfrow=c(1,1))  
   }

   mtext(text=paste("Time : ",sprintf("%1.0f",(jj+1)/NumFrames*EndTime),
                    "of" ,sprintf("%1.0f",EndTime, "days")), 
         side=3, line=-0.5, cex=0.7)   
   
   if (Movie==on) dev.off() else { 
     dev.flush()
     # Sys.sleep(0.01)
     dev.hold()
   }
   if (Wait==on){
     cat ("Press [enter] to continue, [q] to quit")
     line <- readline()
     if (line=='q'){ stop() }
   } 
}

close(FID)

if (Movie==on) { 
  
   InFiles=paste(getwd(),"/Images/Rplot%03d.tiff", sep="")
   OutFile="PredPrey.mp4"
  
   print(paste(" building :", OutFile))
  
   CmdLine=sprintf("ffmpeg -y -r 30 -i %s -c:v libx264 -pix_fmt yuv420p -b:v 50000k %s", InFiles, OutFile)
   cmd = system(CmdLine)
  
   # if (cmd==0) try(system(paste("open ", paste(getwd(),"Mussels_PDE.mp4"))))
} else
{
  #quartz(width=1920/DPI, height=800/DPI, dpi=DPI)
  #plot(x=RecordTimes[-(NumFrames+1)],y=Prey_in_Time,type='l', bty='n', col='green', ylim=c(0,20),
  #     xlab='Time (days)', ylab='Biomass/mm water')
  #lines(RecordTimes[-(NumFrames+1)],Pred_in_Time, col='cyan')
}

system('say All ready')