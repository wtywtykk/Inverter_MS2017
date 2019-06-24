close all;
i=0:511;
f0=50;
f1=450;
r0=f0*2*pi*i/512;
r1=f1*2*pi*i/512;
phaoff=pi/3;
s=cos(r0+phaoff);
s=s+cos(r1+phaoff/f0*f1+0);
fs=fft(s);
plot(imag(fs));
val0=fs(f0+1);
posf=f1;
if posf>512
    posf=posf-512;
end
val1=fs(posf+1);
ang0=angle(val0);
ang1=angle(val1);
angr=ang1-(ang0/f0*f1);
angr=angr+2*pi;
