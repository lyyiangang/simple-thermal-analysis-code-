function plotMeshAndResult
% #num of problem regions and part regions
% 1 3
% #problem region and part regions(x,y,w,h):
% 0 0 1 1
% 0.2 0.2 0.2 0.2
% 0.4 0.5 0.1 0.1
% 0.8 0.1 0.1 0.5
% #x coordinate array and y coordinate array
% 0 0.1 0.2 0.22 0.24 0.26 0.28 0.3 0.32 0.34 0.36 0.38 0.4 0.42 0.44 0.46 0.48 0.5 0.6 0.7 0.8 0.82 0.84 0.86 0.88 1 
% 0 0.1 0.12 0.14 0.16 0.18 0.2 0.22 0.24 0.26 0.28 0.3 0.32 0.34 0.36 0.38 0.4 0.42 0.44 0.46 0.48 0.5 0.52 0.54 0.56 0.58 0.6 0.733333 0.866667 1 
meshFileName='d:\\cfd.mesh';%网格文件
resultFile='d:\\cfd_result_file.dat';%温度结果文件

fidin=fopen(meshFileName,'r'); 
tline=str2num(getDataLine(fidin));
nProblemRegion=tline(1);
nPartRegion=tline(2);
if(nProblemRegion~=1)
    return;
end
problemRegionData=str2num(getDataLine(fidin));
partRegionData=zeros(nPartRegion,4);
for i=1:nPartRegion
    partRegionData(i,:)=str2num(getDataLine(fidin));
end
xPos=str2num(getDataLine(fidin));
yPos=str2num(getDataLine(fidin));
fclose(fidin);
%% 开始绘制网格和结果
nx=length(xPos);
ny=length(yPos);
hold on;
axis equal
%plot reuslt first
plotColorResult(xPos,yPos,resultFile);
yy=zeros(1,nx);
for i=1:ny
    yy(:)=yPos(i);
    plot(xPos,yy,'-b');
end
xx=zeros(1,ny);
for i=1:nx
    xx(:)=xPos(i);
    plot(xx,yPos,'-b');
end
%plot problem region and part region
plotRegion(problemRegionData);
for i=1:nPartRegion
    plotRegion(partRegionData(i,:));
end

hold off;

end

function [tline]= getDataLine(fidin)
while ~feof(fidin) % 判断是否为文件末尾
  tline=fgetl(fidin); % 从文件读入一行文本（不含回车键）
  if(strcmp(tline(1),'#')>0)
      continue;
  else
      break;
  end
end
end

function plotRegion(regionInfo)
x=regionInfo(1);
y=regionInfo(2);
w=regionInfo(3);
h=regionInfo(4);
pos=[x,y;
    x+w,y;
    x+w,y+h;
    x,y+h;
    x,y;];
plot(pos(:,1),pos(:,2),'r-');
end

function plotColorResult(xPos,yPos,resultFile)

resutlMatrix=load(resultFile);
[X,Y]=meshgrid(xPos,yPos);
contourf(X,Y,resutlMatrix,8);
colorbar
end