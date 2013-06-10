function [ cc ] = show_recent( folder, start_pix, stop_pix )
%   Display the most updated correlation maps
%   input args include:
%       folder where the correlation files are saved
%       optional [start_pix stop_pix]

if (nargin < 1)
    fprintf('default path (current folder) is used \n');
    folder='./';
end

if (nargin<=1)
   start_pix = 30;
   stop_pix  = 300;
elseif(nargin==2)
   stop_pix  = 300;  
end

class0 = recentfile(folder, '*angular*class0*h5');
class1 = recentfile(folder, '*angular*class1*h5');

fprintf('class0 most updated file:\n\t %s\n',class0);
fprintf('class1 most updated file:\n\t %s\n',class1);

c0=hdf5read(class0,'/data/data');
c1=hdf5read(class1,'/data/data');
figure;
subplot(2,1,1);
imagesc(log(abs(c0(:,start_pix:stop_pix))));
title(class0,'fontsize',20);
subplot(2,1,2);
imagesc(log(abs(c1(:,start_pix:stop_pix))));
title(class1,'fontsize',20);
end

function [filename]=recentfile( path,expression )
d = dir([path '/' expression]);
[dx dx] = sort([d.datenum]);
filename = d(dx(end)).name;
end