#pragma once
#include "../gSLIC_defines.h"
#include "../objects/gSLIC_spixel_info.h"

_CPU_AND_GPU_CODE_ inline void rgb2xyz(const gSLIC::Vector4u& pix_in, gSLIC::Vector4f& pix_out)
{
	float _b = (float)pix_in.x / 255;
	float _g = (float)pix_in.y / 255;
	float _r = (float)pix_in.z / 255;

	if (_b <= 0.04045f)    _b = _b / 12.92f;
	else                   _b = pow((_b + 0.055f) / 1.055f, 2.4f);
	if (_g <= 0.04045f)    _g = _g / 12.92f;
	else                   _g = pow((_g + 0.055f) / 1.055f, 2.4f);
	if (_r <= 0.04045f)    _r = _r / 12.92f;
	else                   _r = pow((_r + 0.055f) / 1.055f, 2.4f);

	pix_out.x = _r*0.4124564f + _g*0.3575761f + _b*0.1804375f;
	pix_out.y = _r*0.2126729f + _g*0.7151522f + _b*0.0721750f;
	pix_out.z = _r*0.0193339f + _g*0.1191920f + _b*0.9503041f;
}

_CPU_AND_GPU_CODE_ inline void rgb2CIELab(const gSLIC::Vector4u& pix_in, gSLIC::Vector4f& pix_out)
{
	float _b = (float)pix_in.x / 255;
	float _g = (float)pix_in.y / 255;
	float _r = (float)pix_in.z / 255;

	if (_b <= 0.04045f)    _b = _b / 12.92f;
	else                   _b = pow((_b + 0.055f) / 1.055f, 2.4f);
	if (_g <= 0.04045f)    _g = _g / 12.92f;
	else                   _g = pow((_g + 0.055f) / 1.055f, 2.4f);
	if (_r <= 0.04045f)    _r = _r / 12.92f;
	else                   _r = pow((_r + 0.055f) / 1.055f, 2.4f);

	float x = _r*0.4124564f + _g*0.3575761f + _b*0.1804375f;
	float y = _r*0.2126729f + _g*0.7151522f + _b*0.0721750f;
	float z = _r*0.0193339f + _g*0.1191920f + _b*0.9503041f;

	float epsilon = 0.008856f;	//actual CIE standard
	float kappa = 903.3f;		//actual CIE standard

	float Xr = 0.950456f;	//reference white
	float Yr = 1.0f;		//reference white
	float Zr = 1.088754f;	//reference white

	float xr = x / Xr;
	float yr = y / Yr;
	float zr = z / Zr;

	float fx, fy, fz;
	if (xr > epsilon)	fx = pow(xr, 1.0f / 3.0f);
	else				fx = (kappa*xr + 16.0f) / 116.0f;
	if (yr > epsilon)	fy = pow(yr, 1.0f / 3.0f);
	else				fy = (kappa*yr + 16.0f) / 116.0f;
	if (zr > epsilon)	fz = pow(zr, 1.0f / 3.0f);
	else				fz = (kappa*zr + 16.0f) / 116.0f;

	pix_out.x = 116.0f*fy - 16.0f;
	pix_out.y = 500.0f*(fx - fy);
	pix_out.z = 200.0f*(fy - fz);
}

_CPU_AND_GPU_CODE_ inline void cvt_img_space_shared(const gSLIC::Vector4u* inimg, gSLIC::Vector4f* outimg, const gSLIC::Vector2i& img_size, int x, int y, const gSLIC::COLOR_SPACE& color_space)
{
	int idx = y * img_size.x + x;

	switch (color_space)
	{
	case gSLIC::RGB:
		outimg[idx].x = inimg[idx].x;
		outimg[idx].y = inimg[idx].y;
		outimg[idx].z = inimg[idx].z;
		break;
	case gSLIC::XYZ:
		rgb2xyz(inimg[idx], outimg[idx]);
		break;
	case gSLIC::CIELAB:
		rgb2CIELab(inimg[idx], outimg[idx]);
		break;
	}
}

_CPU_AND_GPU_CODE_ inline void init_cluster_centers_shared(const gSLIC::Vector4f* inimg, gSLIC::objects::spixel_info* out_spixel, gSLIC::Vector2i map_size, gSLIC::Vector2i img_size, int spixel_size, int x, int y)
{
	int cluster_idx = y * map_size.x + x;

	int img_x = x * spixel_size + spixel_size / 2;
	int img_y = y * spixel_size + spixel_size / 2;

	img_x = img_x >= img_size.x ? (x * spixel_size + img_size.x) / 2 : img_x;
	img_y = img_y >= img_size.y ? (y * spixel_size + img_size.y) / 2 : img_y;

	// TODO: go one step towards gradients direction

	out_spixel[cluster_idx].id = cluster_idx;
	out_spixel[cluster_idx].center = gSLIC::Vector2f((float)img_x, (float)img_y);
	out_spixel[cluster_idx].color_info = inimg[img_y*img_size.x + img_x];
	
	out_spixel[cluster_idx].no_pixels = 0;
}

_CPU_AND_GPU_CODE_ inline float compute_slic_distance(const gSLIC::Vector4f& pix, int x, int y, const gSLIC::objects::spixel_info& center_info, float weight, float normalizer_xy, float normalizer_color)
{
	float dcolor = (pix.x - center_info.color_info.x)*(pix.x - center_info.color_info.x)
				 + (pix.y - center_info.color_info.y)*(pix.y - center_info.color_info.y)
				 + (pix.z - center_info.color_info.z)*(pix.z - center_info.color_info.z);

	float dxy = (x - center_info.center.x) * (x - center_info.center.x)
			  + (y - center_info.center.y) * (y - center_info.center.y);


	float retval = dcolor * normalizer_color + weight * dxy * normalizer_xy;
	return sqrtf(retval);
}

_CPU_AND_GPU_CODE_ inline void find_center_association_shared(const gSLIC::Vector4f* inimg, const gSLIC::objects::spixel_info* in_spixel_map, int* out_idx_img, gSLIC::Vector2i map_size, gSLIC::Vector2i img_size, int spixel_size, float weight, int x, int y, float max_xy_dist, float max_color_dist)
{
	int idx_img = y * img_size.x + x;

	int ctr_x = x / spixel_size;
	int ctr_y = y / spixel_size;

	int minidx = -1;
	float dist = 999999.9999f;

	// search 3x3 neighborhood
	for (int i = -1; i <= 1; i++) for (int j = -1; j <= 1; j++)
	{
		int ctr_x_check = ctr_x + j;
		int ctr_y_check = ctr_y + i;
		if (ctr_x_check >= 0 && ctr_y_check >= 0 && ctr_x_check < map_size.x && ctr_y_check < map_size.y)
		{
			int ctr_idx = ctr_y_check*map_size.x + ctr_x_check;
			float cdist = compute_slic_distance(inimg[idx_img], x, y, in_spixel_map[ctr_idx], weight, max_xy_dist, max_color_dist);
			if (cdist < dist)
			{
				dist = cdist;
				minidx = in_spixel_map[ctr_idx].id;
			}
		}
	}

	if (minidx >= 0) out_idx_img[idx_img] = minidx;
}

_CPU_AND_GPU_CODE_ inline void draw_superpixel_boundry_shared(const int* idx_img, gSLIC::Vector4u* sourceimg, gSLIC::Vector4u* outimg, gSLIC::Vector2i img_size, int x, int y)
{
	int idx = y * img_size.x + x;

	if (idx_img[idx] != idx_img[idx + 1]
	 || idx_img[idx] != idx_img[idx - 1]
	 || idx_img[idx] != idx_img[idx - img_size.x]
	 || idx_img[idx] != idx_img[idx + img_size.x])
	{
		outimg[idx] = gSLIC::Vector4u(0, 0, 0, 0);
	}
	else
	{
		outimg[idx] = sourceimg[idx];
	}
}

_CPU_AND_GPU_CODE_ inline void finalize_reduction_result_shared(const gSLIC::objects::spixel_info* accum_map, gSLIC::objects::spixel_info* spixel_list, gSLIC::Vector2i map_size, int no_blocks_per_spixel, int x, int y)
{
	int spixel_idx = y * map_size.x + x;

	spixel_list[spixel_idx].center = gSLIC::Vector2f(0, 0);
	spixel_list[spixel_idx].color_info = gSLIC::Vector4f(0, 0, 0, 0);
	spixel_list[spixel_idx].no_pixels = 0;

	for (int i = 0; i < no_blocks_per_spixel; i++)
	{
		int accum_list_idx = spixel_idx * no_blocks_per_spixel + i;

		spixel_list[spixel_idx].center += accum_map[accum_list_idx].center;
		spixel_list[spixel_idx].color_info += accum_map[accum_list_idx].color_info;
		spixel_list[spixel_idx].no_pixels += accum_map[accum_list_idx].no_pixels;
	}

	if (spixel_list[spixel_idx].no_pixels != 0)
	{
		spixel_list[spixel_idx].center /= (float)spixel_list[spixel_idx].no_pixels;
		spixel_list[spixel_idx].color_info /= (float)spixel_list[spixel_idx].no_pixels;
	}
	else
	{
		spixel_list[spixel_idx].center = gSLIC::Vector2f(-100, -100);
		spixel_list[spixel_idx].color_info = gSLIC::Vector4f(-100, -100, -100, -100);
	}
}

_CPU_AND_GPU_CODE_ inline void supress_local_lable(const int* in_idx_img, int* out_idx_img, gSLIC::Vector2i img_size, int x, int y)
{
	int clable = in_idx_img[y*img_size.x + x];

	// don't suppress boundary
	if (x < 2 || y < 2 || x >= img_size.x - 2 || y >= img_size.y - 2)
	{ 
		out_idx_img[y*img_size.x + x] = clable;
		return; 
	}

	int diff_count = 0;
	int diff_lable = -1;

	for (int j = -2; j <= 2; j++) for (int i = -2; i <= 2; i++)
	{
		int nlable = in_idx_img[(y + j)*img_size.x + (x + i)];
		if (nlable != clable)
		{
			diff_lable = nlable;
			diff_count++;
		}
	}
    
	if (diff_count > 16)
		out_idx_img[y*img_size.x + x] = diff_lable;
	else
		out_idx_img[y*img_size.x + x] = clable;
}

_CPU_AND_GPU_CODE_ inline void supress_local_lable_2(const int* in_idx_img, int* out_idx_img, gSLIC::Vector2i img_size, int x, int y)
{
	int pixel_idx = y*img_size.x + x;
	int clable = in_idx_img[pixel_idx];
	if (x < 1 || y < 1 || x >= img_size.x - 1 || y >= img_size.y - 1)
	{
		out_idx_img[y*img_size.x + x] = clable;
		return;
	}

	int diff_count = 0;
	int diff_lable = -1;

	for (int j = -1; j <= 1; j++) for (int i = -1; i <= 1; i++)
	{
		int nlable = in_idx_img[(y + j)*img_size.x + (x + i)];
		if (nlable != clable)
		{
			diff_lable = nlable;
			diff_count++;
		}
	}

	if (diff_count >= 6)
		out_idx_img[pixel_idx] = diff_lable;
	else
		out_idx_img[pixel_idx] = clable;
}

_CPU_AND_GPU_CODE_ inline dim3 getGridSize(dim3 dataSz, dim3 blockSz)
{
	return dim3((dataSz.x + blockSz.x - 1)/blockSz.x, (dataSz.y + blockSz.y - 1)/blockSz.y, (dataSz.z + blockSz.z - 1)/blockSz.z);
}

_CPU_AND_GPU_CODE_ inline dim3 getGridSize(gSLIC::Vector2i dataSz, dim3 blockSz)
{
	return dim3((dataSz.x + blockSz.x - 1) / blockSz.x, (dataSz.y + blockSz.y - 1) / blockSz.y);
}

_CPU_AND_GPU_CODE_ inline float4 operator+= (float4 &a, float4 b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

_CPU_AND_GPU_CODE_ inline float2 operator+= (float2 &a, float2 b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}