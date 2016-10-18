clear; clc;

input_dir = './test_result/';
output_dir = './c_test_result/';

mkdir(output_dir);

iids = dir(fullfile(input_dir,'*.mat'));
for i = 1:length(iids)
    if length(iids(i).name) > 10
        continue;
    end
    
    index = str2num(iids(i).name(1:end-4));
    load(fullfile(input_dir,iids(i).name));
    
    fid = fopen(fullfile(output_dir,sprintf('%06d',index)),'w');
    fwrite(fid,'CmMat','*char');
    fwrite(fid,1,'int');
    s = size(fuse);
    fwrite(fid,[s(2) s(1) 5],'int');
    fwrite(fid,fuse','float');
    fclose(fid);
end