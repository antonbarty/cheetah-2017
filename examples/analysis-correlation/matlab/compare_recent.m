function [ cc ] = compare_recent( folder, pixel_positions )
%   Display the most updated correlation maps
%   input args include:
%       folder where the correlation files are saved
%       optional [start_pix stop_pix]

if (nargin < 1)
    fprintf('default path (current folder) is used \n');
    folder='./';
end

if (nargin<=1)
   pixel_positions=[30 50 100 200]; 
end

class0 = recentfile(folder, '*angular*class0*h5');
class1 = recentfile(folder, '*angular*class1*h5');

fprintf('class0 most updated file:\n\t %s\n',class0);
fprintf('class1 most updated file:\n\t %s\n',class1);

c0=hdf5read(class0,'/data/data');
c1=hdf5read(class1,'/data/data');

max_x0 = size(c0,1);
max_x1 = size(c1,1);
ccs=zeros(4,1);
figure;
for ii=1:4
    subplot(2,2,ii);
    this_max0 = max(c0(:,pixel_positions(ii)));
    this_max1 = max(c1(:,pixel_positions(ii)));
    cc = corrcoef( c0(:,pixel_positions(ii)), c1(:,pixel_positions(ii)) );
    ccs(ii) = cc(1,2);
    scale = c0(:,pixel_positions(ii)) \ c1(:,pixel_positions(ii) );
    c1(:,pixel_positions(ii) ) = c1(:,pixel_positions(ii) )/scale;
    plot(1:max_x0,c0(:,pixel_positions(ii)),1:max_x1,c1(:,pixel_positions(ii)));
    title(['at pixel ' int2str(pixel_positions(ii))] ,'fontsize',20);
end

cc = mean(ccs);
fprintf('\nThe average correlation coefficient is %f\n',cc);
end

function [filename]=recentfile( path,expression )
d = dir([path '/' expression]);
[dx dx] = sort([d.datenum]);
filename = d(dx(end)).name;
end